/**
 * Memory bandwidth controller for multi-core systems
 *
 * Copyright (C) 2013  Heechul Yun <heechul@illinois.edu>
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define DEBUG(x)
#define DEBUG_RECLAIM(x)
#define DEBUG_USER(x)
#define DEBUG_PROFILE(x) x

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/smp.h> /* IPI calls */
#include <linux/irq_work.h>
#include <linux/hardirq.h>
#include <linux/perf_event.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/interrupt.h>

/* Patch Includes */

#include <asm/io.h>
#include <linux/init.h>
#include <linux/timer.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
#  include <uapi/linux/sched/types.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 13, 0)
#  include <linux/sched/types.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 8, 0)
#  include <linux/sched/rt.h>
#endif
#include <linux/sched.h>


/**************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
 * Throttle Addresses 
 **************************************************************************/

#define THROTTLE 1

// Base Address
#define MC_ADDRESS                          0x70019000
#define MC_EMEM_ARB_OUTSTANDING_REQ_0       0x70019094
#define MC_EMEM_ARB_RING1_THROTTLE_0        0x700190e0
#define MC_EMEM_ARB_RING3_THROTTLE_0        0x700190e4
// Disable bit 31 for ring1 arbitration
#define MC_EMEM_ARB_RING0_THROTTLE_MASK_0   0x700196bc
#define MC_EMEM_ARB_OUTSTANDING_REQ_RING3_0 0x7001966c

/**************************************************************************
 * Activity Monitor Addresses 
 **************************************************************************/

// Base Address
#define ACTMON_ADDRESS                      0x6000c800

// Offsets

// Global Enable
#define ACTMON_GLB_STATUS_0		              0x0
#define ACTMON_GLB_PERIOD_CTRL_0            0x4

// All Memory Controller Traffic
#define ACTMON_MCALL_CTRL_0		              0x1c0
#define ACTMON_MCALL_INIT_AVG_0             0x1cc
#define ACTMON_MCALL_COUNT_WEIGHT_0         0x128
#define ACTMON_MCALL_COUNT_0                0x1dc
#define ACTMON_MCALL_AVG_COUNT_0	          0x1e0

// Only CPU Memory Controller Traffic
#define ACTMON_MCCPU_CTRL_0		              0x200
#define ACTMON_MCCPU_INIT_AVG_0		          0x20c
#define ACTMON_MCCPU_COUNT_WEIGHT_0         0x218
#define ACTMON_MCCPU_COUNT_0		            0x21c
#define ACTMON_MCCPU_AVG_COUNT_0	          0x220

/**************************************************************************
 * Constants 
 **************************************************************************/

#define THROTTLE_MAX  16
#define THROTTLE_MIN  0
#define MAX_UTILIZATION 10000

/**************************************************************************
**************************************************************************
**************************************************************************


**************************************************************************
 * Public Definitions
 **************************************************************************/
#define CACHE_LINE_SIZE 64
#define BUF_SIZE 256
#define PREDICTOR 1  /* 0 - used, 1 - ewma(a=1/2), 2 - ewma(a=1/4) */

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0) // somewhere between 4.4-4.10
#  define TM_NS(x) (x)
#else
#  define TM_NS(x) (x).tv64
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
struct memstat{
	u64 used_budget;         /* used budget*/
	u64 assigned_budget;
	u64 throttled_time_ns;   
	int throttled;           /* throttled period count */
	u64 throttled_error;     /* throttled & error */
	int throttled_error_dist[10]; /* pct distribution */
	int exclusive;           /* exclusive period count */
	u64 exclusive_ns;        /* exclusive mode real-time duration */
	u64 exclusive_bw;        /* exclusive mode used bandwidth */
};

/* percpu info */
struct core_info {
	/* user configurations */
	int budget;              /* assigned budget */
	int limit;               /* limit mode (exclusive to weight)*/

	/* for control logic */
	int cur_budget;          /* currently available budget */

	struct task_struct * throttled_task;
	
	ktime_t throttled_time;  /* absolute time when throttled */

	u64 old_val;             /* hold previous counter value */
	int prev_throttle_error; /* check whether there was throttle error in 
				    the previous period */

	int exclusive_mode;      /* 1 - if in exclusive mode */
	ktime_t exclusive_time;  /* time when exclusive mode begins */

	struct irq_work	pending; /* delayed work for NMIs */
	struct perf_event *event;/* PMC: LLC misses */

	struct task_struct *throttle_thread;  /* forced throttle idle thread */
	wait_queue_head_t throttle_evt; /* throttle wait queue */

	/* statistics */
	struct memstat overall;  /* stat for overall periods. reset by user */
	int used[3];             /* EWMA memory load */
	int64_t period_cnt;         /* active periods count */
	struct hrtimer hr_timer;
};

/* global info */
struct memguard_info {
	ktime_t period_in_ktime;
	cpumask_var_t throttle_mask;
	cpumask_var_t active_mask;
	atomic_t wsum;
};


/**************************************************************************
 * Global Variables
 **************************************************************************/
static struct memguard_info memguard_info;
static struct core_info __percpu *core_info;

static int g_period_us = 1000;
static int g_use_bwlock = 0;
static int g_use_exclusive = 0;
static int *g_budget_mb;

static int g_hw_counter_id = -1;

static struct dentry *memguard_dir;

static u32 mc_all_avg = 0;
static u32 mc_cpu_avg = 0;
static u32 mc_all_count = 0;
static u32 mc_cpu_count = 0;
#if THROTTLE
static u32 throttle_amount = 0;
static u32 throttle_limit  = 0;
#endif

// Memory Locations
#if THROTTLE
void __iomem *io_throttle;
void __iomem *io_limit;
void __iomem *io_arbitration;
#endif

void __iomem *io_mc_all_avg_count;
void __iomem *io_mc_cpu_avg_count;

/**************************************************************************
 * External Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Function Prototypes
 **************************************************************************/
static void period_timer_callback_slave(struct core_info *cinfo);
enum hrtimer_restart period_timer_callback_master(struct hrtimer *timer);
static void memguard_process_overflow(struct irq_work *entry);
static int throttle_thread(void *arg);
static void memguard_on_each_cpu_mask(const struct cpumask *mask,
				      smp_call_func_t func,
				      void *info, bool wait);

/**************************************************************************
 * Module parameters
 **************************************************************************/

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0)
module_param(g_hw_counter_id, hexint,  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#else
module_param(g_hw_counter_id, int,  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#endif
MODULE_PARM_DESC(g_hw_counter_id, "Raw hardware counter number");

MODULE_PARM_DESC(g_hw_type, "raw hardware counter number");

module_param(g_use_bwlock, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(g_use_bwlock, "enable/disable reclaim");

module_param(g_period_us, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(g_period_us, "throttling period in usec");

/**************************************************************************
 * Module main code
 **************************************************************************/

#if THROTTLE
static int set_throttle_op(void *data, u64 value)
{
  u32 bitWise = 0;

  // Define Throttling amount
  // Bits 20:16 set throttling amount when limit is exceeded
  // Bits 4:0 set throttling for when limit not exceeded

  bitWise = (value << 16) | value;
  iowrite32( bitWise , io_throttle);

  throttle_amount = value;
  return 0;
}

static int set_throttle(u32 value)
{
  u32 bitWise = 0;

  // Define Throttling amount
  // Bits 20:16 set throttling amount when limit is exceeded
  // Bits 4:0 set throttling for when limit not exceeded

  bitWise = (value << 16) | value;
  iowrite32( bitWise , io_throttle);

  throttle_amount = value;
  return 0;
}

static int set_limit(u32 value)
{
  // Set Request Limit (If outstanding requests exceed, throttling initiates) 
  // 31st bit LOW - don't limit requests, we want them to exceed the arbitration limit 
  //
  // Bits 8:0 sets the request limit
  u32 bitWise = ~(1 << 31);

  // Clear the low 8 bits
  // to be or-ed with limit
  bitWise = bitWise & 0x11111E00;
  iowrite32( ( (ioread32(io_limit) & bitWise) | value ) , io_limit);

  // Enable Ring1 Arbitration
  bitWise = 0x80008000;
  iowrite32( bitWise , io_arbitration);

  throttle_limit = value;
  return 0;
}
#endif

static int set_actmon(void)
{
  // Initialize Global Period
  // Writing 1/0 to bit 8 sets period time base in usec/msec
  // Writing n to bits 7:0 creates a n+1 usec/msec sampling period

  // Setting for 10 usec
  // (1 << 8) | (0x9) == 0x109
  // Setting for 1 msec
  // (0 << 8) | (0x0) == 0x0
  u32 bitWise = 0x0;
  void __iomem *io = ioremap(ACTMON_ADDRESS + ACTMON_GLB_PERIOD_CTRL_0, 32);
  iowrite32( bitWise , io);

  // mc_all 
  // Enable MC Activity Monitor
  // Write 1 to bit 31
  // Write 1 to bit 18 to enable periodic sampling
  // Write 6 to bits 10:12
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_CTRL_0, 32);
  // (1 << 31) | (1 << 18) | (1 << 11) | (1 << 12);
  bitWise = 0x80041800;
  iowrite32( (ioread32(io) | bitWise) , io);

  // Initialize the Weight to 0x400 
  bitWise = 0x400;
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_COUNT_WEIGHT_0, 32);
  iowrite32( bitWise , io);

  // Initialize the AVG Count to 0
  bitWise = 0;
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_INIT_AVG_0, 32);
  iowrite32( bitWise , io);


  // mc_cpu
  // Enable MC Activity Monitor
  // Same bits need to be set as mc_all 
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_CTRL_0, 32);
  bitWise = 0x80041800;
  iowrite32( (ioread32(io) | bitWise) , io);

  // Initialize the Weight to 0x400 
  bitWise = 0x400;
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_COUNT_WEIGHT_0, 32);
  iowrite32( bitWise , io);

  // Initialize the AVG Count to 0
  bitWise = 0;
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_INIT_AVG_0, 32);
  iowrite32( bitWise , io);

  return 0;
}

static int reset_actmon(void)
{
  // Initialize Global Period
  // Writing 1/0 to bit 8 sets period time base in usec/msec
  // Writing n to bits 7:0 creates a n+1 usec/msec sampling period

  // Setting for 25 2sec
  // (0 << 8) | (0x19) == 0x19
  u32 bitWise = 0x19;
  void __iomem *io = ioremap(ACTMON_ADDRESS + ACTMON_GLB_PERIOD_CTRL_0, 32);
  iowrite32( bitWise , io);

  // mc_cpu
  // Enable MC Activity Monitor
  // Write 1 to bit 31
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_CTRL_0, 32);
  bitWise = ~(1 << 31);
  iowrite32( (ioread32(io) & bitWise) , io);

  return 0;
}

/* similar to on_each_cpu_mask(), but this must be called with IRQ disabled */
static void memguard_on_each_cpu_mask(const struct cpumask *mask, 
				      smp_call_func_t func,
				      void *info, bool wait)
{
	int cpu = smp_processor_id();
	smp_call_function_many(mask, func, info, wait);
	if (cpumask_test_cpu(cpu, mask)) {
		func(info);
	}
}

/** convert MB/s to #of events (i.e., LLC miss counts) per 1ms */
static inline u64 convert_mb_to_events(int mb)
{
	return div64_u64((u64)mb*1024*1024,
			 CACHE_LINE_SIZE* (1000000/g_period_us));
}
static inline int convert_events_to_mb(u64 events)
{
	int divisor = g_period_us*1024*1024;
	int mb = div64_u64(events*CACHE_LINE_SIZE*1000000 + (divisor-1), divisor);
	return mb;
}

static inline void print_current_context(void)
{
	trace_printk("in_interrupt(%ld)(hard(%ld),softirq(%d)"
		     ",in_nmi(%d)),irqs_disabled(%d)\n",
		     in_interrupt(), in_irq(), (int)in_softirq(),
		     (int)in_nmi(), (int)irqs_disabled());
}

/** read current counter value. */
static inline u64 perf_event_count(struct perf_event *event)
{
	return local64_read(&event->count) + 
		atomic64_read(&event->child_count);
}

/** return used event in the current period */
static inline u64 memguard_event_used(struct core_info *cinfo)
{
	return perf_event_count(cinfo->event) - cinfo->old_val;
}

static void print_core_info(int cpu, struct core_info *cinfo)
{
	pr_info("CPU%d: budget: %d, cur_budget: %d, period: %ld\n", 
		cpu, cinfo->budget, cinfo->cur_budget, (long)cinfo->period_cnt);
}

/**
 * update per-core usage statistics
 */
void update_statistics(struct core_info *cinfo)
{
	/* counter must be stopped by now. */
	s64 new;
	int used;

	new = perf_event_count(cinfo->event);
	used = (int)(new - cinfo->old_val); 

	cinfo->old_val = new;
	cinfo->overall.used_budget += used;
	cinfo->overall.assigned_budget += cinfo->budget;

	/* EWMA filtered per-core usage statistics */
	cinfo->used[0] = used;
	cinfo->used[1] = (cinfo->used[1] * (2-1) + used) >> 1; 
	/* used[1]_k = 1/2 used[1]_k-1 + 1/2 used */
	cinfo->used[2] = (cinfo->used[2] * (4-1) + used) >> 2; 
	/* used[2]_k = 3/4 used[2]_k-1 + 1/4 used */

	/* core is currently throttled. */
	if (cinfo->throttled_task) {
		cinfo->overall.throttled_time_ns +=
			(TM_NS(ktime_get()) - TM_NS(cinfo->throttled_time));
		cinfo->overall.throttled++;
	}

	/* throttling error condition:
	   I was too aggressive in giving up "unsed" budget */
	if (cinfo->prev_throttle_error && used < cinfo->budget) {
		int diff = cinfo->budget - used;
		int idx;
		cinfo->overall.throttled_error ++; // += diff;
		BUG_ON(cinfo->budget == 0);
		idx = (int)(diff * 10 / cinfo->budget);
		cinfo->overall.throttled_error_dist[idx]++;
		trace_printk("ERR: throttled_error: %d < %d\n", used, cinfo->budget);
		/* compensation for error to catch-up*/
		cinfo->used[PREDICTOR] = cinfo->budget + diff;
	}
	cinfo->prev_throttle_error = 0;

	/* I was the lucky guy who used the DRAM exclusively */
	if (cinfo->exclusive_mode) {
		u64 exclusive_ns, exclusive_bw;

		/* used time */
		exclusive_ns = (TM_NS(ktime_get()) -
				TM_NS(cinfo->exclusive_time));
		
		/* used bw */
		exclusive_bw = (cinfo->used[0] - cinfo->budget);

		cinfo->overall.exclusive_ns += exclusive_ns;
		cinfo->overall.exclusive_bw += exclusive_bw;
		cinfo->exclusive_mode = 0;
		cinfo->overall.exclusive++;
	}

  if (smp_processor_id() == 0) {
    mc_all_avg = ioread32(io_mc_all_avg_count);
    mc_cpu_avg = ioread32(io_mc_cpu_avg_count);

	  DEBUG_PROFILE(trace_printk("%d %d %lld %d\n",
				   mc_all_avg, mc_cpu_avg,
           new, used));
    
  }
}


/**
 * budget is used up. PMU generate an interrupt
 * this run in hardirq, nmi context with irq disabled
 */
static void event_overflow_callback(struct perf_event *event,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
				    int nmi,
#endif
				    struct perf_sample_data *data,
				    struct pt_regs *regs)
{
	struct core_info *cinfo = this_cpu_ptr(core_info);
	BUG_ON(!cinfo);
	irq_work_queue(&cinfo->pending);
}


/**
 * called by process_overflow
 */
static void __unthrottle_core(void *info)
{
	struct core_info *cinfo = this_cpu_ptr(core_info);
	if (cinfo->throttled_task) {
		cinfo->exclusive_mode = 1;
		cinfo->exclusive_time = ktime_get();

		cinfo->throttled_task = NULL;
		DEBUG_RECLAIM(trace_printk("exclusive mode begin\n"));
	}
}

static void __newperiod(void *info)
{
	ktime_t start = ktime_get();
	struct memguard_info *global = &memguard_info;
	struct core_info *cinfo = this_cpu_ptr(core_info);
	
	ktime_t new_expire = ktime_add(start, global->period_in_ktime);

	/* arrived before timer interrupt is called */
	hrtimer_start_range_ns(&cinfo->hr_timer, new_expire,
			       0, HRTIMER_MODE_ABS_PINNED);

	DEBUG(trace_printk("begin new period\n"));
	period_timer_callback_slave(cinfo);
}

/**
 * memory overflow handler.
 * must not be executed in NMI context. but in hard irq context
 */
static void memguard_process_overflow(struct irq_work *entry)
{
	struct core_info *cinfo = this_cpu_ptr(core_info);
	struct memguard_info *global = &memguard_info;
	ktime_t start;
	s64 budget_used;

	start = ktime_get();

	BUG_ON(in_nmi() || !in_irq());

	budget_used = memguard_event_used(cinfo);

	/* erroneous overflow, that could have happend before period timer
	   stop the pmu */
	if (budget_used < cinfo->cur_budget) {
		trace_printk("ERR: overflow in timer. used %lld < cur_budget %d. ignore\n",
			     budget_used, cinfo->cur_budget);
		return;
	}

	/* no more overflow interrupt */
	local64_set(&cinfo->event->hw.period_left, 0xfffffff);

	/* check if we donated too much */
	if (budget_used < cinfo->budget) {
		trace_printk("ERR: throttling error\n");
		cinfo->prev_throttle_error = 1;
	}

	if (!cpumask_test_cpu(smp_processor_id(), global->active_mask)) {
		trace_printk("ERR: not active\n");
		cinfo->throttled_task = NULL;
		return;
	}
	/* we are going to be throttled */
	cpumask_set_cpu(smp_processor_id(), global->throttle_mask);
	smp_mb(); // w -> r ordering of the local cpu.
	if (cpumask_equal(global->throttle_mask, global->active_mask)) {
		/* all other cores are alreay throttled */
		if (g_use_exclusive == 1) {
			/* algorithm 1: last one get all remaining time */
			cinfo->exclusive_mode = 1;
			cinfo->exclusive_time = ktime_get();
			DEBUG_RECLAIM(trace_printk("exclusive mode begin\n"));
			return;
		} else if (g_use_exclusive == 2) {
			/* algorithm 2: wakeup all (i.e., non regulation) */
			memguard_on_each_cpu_mask(global->throttle_mask, __unthrottle_core, NULL, 0);
			return;
		} else if (g_use_exclusive == 5) {
			/* algorithm 5: begin a new period */
			memguard_on_each_cpu_mask(global->active_mask, __newperiod, NULL, 0);
			return;
		} else if (g_use_exclusive > 5) {
			trace_printk("ERR: Unsupported exclusive mode %d\n", 
				     g_use_exclusive);
			return;
		} else if (g_use_exclusive != 0 &&
			   cpumask_weight(global->active_mask) == 1) {
			trace_printk("ERR: don't throttle one active core\n");
			return;
		}
	}

	if (cinfo->prev_throttle_error)
		return;
	/*
	 * fail to reclaim. now throttle this core
	 */
	DEBUG_RECLAIM(trace_printk("fail to reclaim after %lld nsec.\n",
				   TM_NS(ktime_get()) - TM_NS(start)));

	/* wake-up throttle task */
	cinfo->throttled_task = current;
	cinfo->throttled_time = start;

	// WARN_ON_ONCE(!strncmp(current->comm, "swapper", 7));
	wake_up_interruptible(&cinfo->throttle_evt);
}


/**
 * per-core period timer callback
 *
 *   called while cpu_base->lock is held by hrtimer_interrupt()
 *
 */
enum hrtimer_restart period_timer_callback_master(struct hrtimer *timer)
{
	struct core_info *cinfo = this_cpu_ptr(core_info);
	struct memguard_info *global = &memguard_info;
	int orun;

	/* must be irq disabled. hard irq */
	BUG_ON(!irqs_disabled());
	// WARN_ON_ONCE(!in_interrupt());

	/* stop counter */
	cinfo->event->pmu->stop(cinfo->event, PERF_EF_UPDATE);

	/* forward timer */
	orun = hrtimer_forward_now(timer, global->period_in_ktime);
	BUG_ON(orun == 0);
	if (orun > 1)
		trace_printk("ERR: timer overrun %d at period %ld\n",
			     orun, (long)cinfo->period_cnt);

	/* assign local period */
	cinfo->period_cnt += orun;

	period_timer_callback_slave(cinfo);

	return HRTIMER_RESTART;
}

/**
 * period_timer algorithm:
 *	excess = 0;
 *	if predict < budget:
 *	   excess = budget - predict;
 *	   global += excess
 *	set interrupt at (budget - excess)
 *
 */
static void period_timer_callback_slave(struct core_info *cinfo)
{
	struct memguard_info *global = &memguard_info;
	struct task_struct *target;
	int cpu = smp_processor_id();

	/* I'm actively participating */
	cpumask_clear_cpu(cpu, global->throttle_mask);
	smp_mb();
	cpumask_set_cpu(cpu, global->active_mask);

	/* unthrottle tasks (if any) */
	if (cinfo->throttled_task)
		target = (struct task_struct *)cinfo->throttled_task;
	else
		target = current;
	cinfo->throttled_task = NULL;

	DEBUG(trace_printk("%p|New period %ld. global->budget=%d\n",
			   cinfo->throttled_task,
			   (long)cinfo->period_cnt, global->budget));
	
	/* update statistics. */
	update_statistics(cinfo);

	/* new budget assignment from user */
	if (cinfo->limit > 0) {
		/* limit mode */
		cinfo->budget = cinfo->limit;
	} 

	/* budget can't be zero? */
	cinfo->budget = max(cinfo->budget, 1);

	if (cinfo->event->hw.sample_period != cinfo->budget) {
		/* new budget is assigned */
		DEBUG(trace_printk("MSG: new budget %d is assigned\n", 
				   cinfo->budget));
		cinfo->event->hw.sample_period = cinfo->budget;
	}

	/* per-task donation policy */
	cinfo->cur_budget = cinfo->budget;

	/* setup an interrupt */
	cinfo->cur_budget = max(1, cinfo->cur_budget);
	local64_set(&cinfo->event->hw.period_left, cinfo->cur_budget);

	/* enable performance counter */
	cinfo->event->pmu->start(cinfo->event, PERF_EF_RELOAD);
}

static struct perf_event *init_counter(int cpu, int budget)
{
	struct perf_event *event = NULL;
	struct perf_event_attr sched_perf_hw_attr = {
		/* use generalized hardware abstraction */
		.type           = PERF_TYPE_HARDWARE,
		.config         = PERF_COUNT_HW_CACHE_MISSES,
		.size		= sizeof(struct perf_event_attr),
		.pinned		= 1,
		.disabled	= 1,
		.exclude_kernel = 1,   /* TODO: 1 mean, no kernel mode counting */
	};

	if (g_hw_counter_id >= 0) {
		/*
		   known counters for architecture/platforms

		   intel  0x7024   ??
		          0x08b0   ??
			  0x412e   PERF_COUNT_HW_CACHE_MISSES

		   ARM    0x17     L2 data cache refill

		   AMD    ???      ???

		 */
		sched_perf_hw_attr.type           = PERF_TYPE_RAW;
		sched_perf_hw_attr.config         = g_hw_counter_id;
	}

	/* select based on requested event type */
	sched_perf_hw_attr.sample_period = budget;

	/* Try to register using hardware perf events */
	event = perf_event_create_kernel_counter(
		&sched_perf_hw_attr,
		cpu, NULL,
		event_overflow_callback
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0)
		,NULL
#endif
		);

	if (!event)
		return NULL;

	if (IS_ERR(event)) {
		/* vary the KERN level based on the returned errno */
		if (PTR_ERR(event) == -EOPNOTSUPP)
			pr_info("cpu%d. not supported\n", cpu);
		else if (PTR_ERR(event) == -ENOENT)
			pr_info("cpu%d. not h/w event\n", cpu);
		else
			pr_err("cpu%d. unable to create perf event: %ld\n",
			       cpu, PTR_ERR(event));
		return NULL;
	}

	/* success path */
	pr_info("cpu%d enabled counter.\n", cpu);

	return event;
}

static void __start_counter(void *info)
{
	struct memguard_info *global = &memguard_info;
	struct core_info *cinfo = this_cpu_ptr(core_info);
	BUG_ON(!cinfo->event);

	/* initialize hr timer */
        hrtimer_init(&cinfo->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
        cinfo->hr_timer.function = &period_timer_callback_master;

	/* start timer */
        hrtimer_start(&cinfo->hr_timer, global->period_in_ktime,
                      HRTIMER_MODE_REL_PINNED);

	/* initialize */
	cinfo->throttled_task = NULL;
	cinfo->period_cnt = 0;

	/* start performance counter */
	/* cinfo->event->pmu->start(cinfo->event, PERF_EF_RELOAD); */
}

static void __stop_counter(void *info)
{
	struct core_info *cinfo = this_cpu_ptr(core_info);
	BUG_ON(!cinfo->event);

	/* stop the kthrottle/i */
	cinfo->throttled_task = NULL;
	cinfo->period_cnt = -1; // done

	/* stop the counter */
	cinfo->event->pmu->stop(cinfo->event, PERF_EF_UPDATE);

	/* stop timer */
	hrtimer_cancel(&cinfo->hr_timer);
}


/**************************************************************************
 * Local Functions
 **************************************************************************/

static ssize_t memguard_control_write(struct file *filp,
				    const char __user *ubuf,
				    size_t cnt, loff_t *ppos)
{
	char buf[BUF_SIZE];
	char *p = buf;
	if (copy_from_user(&buf, ubuf, (cnt > BUF_SIZE) ? BUF_SIZE: cnt) != 0)
		return 0;

	if (!strncmp(p, "exclusive ", 10))
		sscanf(p+10, "%d", &g_use_exclusive);
	else
		pr_info("ERROR: %s\n", p);
	return cnt;
}

static int memguard_control_show(struct seq_file *m, void *v)
{
	struct memguard_info *global = &memguard_info;
	char buf[BUF_SIZE];
	seq_printf(m, "exclusive: %d\n", g_use_exclusive);
	cpumap_print_to_pagebuf(1, buf, global->active_mask);
	seq_printf(m, "active: %s\n", buf);
	cpumap_print_to_pagebuf(1, buf, global->throttle_mask);	
	seq_printf(m, "throttle: %s\n", buf);
	return 0;
}

static int memguard_control_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, memguard_control_show, NULL);
}

static const struct file_operations memguard_control_fops = {
	.open		= memguard_control_open,
	.write          = memguard_control_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};


static void __update_budget(void *info)
{
	struct core_info *cinfo = this_cpu_ptr(core_info);

	if ((unsigned long)info == 0) {
		pr_info("ERR: Requested budget is zero\n");
		return;
	}
	cinfo->limit = (unsigned long)info;
	DEBUG_USER(trace_printk("MSG: New budget of Core%d is %d\n",
				smp_processor_id(), cinfo->budget));

}

static ssize_t memguard_limit_write(struct file *filp,
				    const char __user *ubuf,
				    size_t cnt, loff_t *ppos)
{
	char buf[BUF_SIZE];
	char *p = buf;
	int i;
	int use_mb = 0;

	if (copy_from_user(&buf, ubuf, (cnt > BUF_SIZE) ? BUF_SIZE: cnt) != 0) 
		return 0;

	if (!strncmp(p, "mb ", 3)) {
		use_mb = 1;
		p+=3;
	}
	for_each_online_cpu(i) {
		int input;
		unsigned long events;
		sscanf(p, "%d", &input);
		if (input == 0) {
			pr_err("ERR: CPU%d: input is zero: %s.\n",i, p);
			continue;
		}
		if (use_mb)
			events = (unsigned long)convert_mb_to_events(input);
		else
			events = input;

		pr_info("CPU%d: New budget=%ld (%d %s)\n", i, 
			events, input, (use_mb)?"MB/s": "events");
		smp_call_function_single(i, __update_budget,
					 (void *)events, 0);
		
		p = strchr(p, ' ');
		if (!p) break;
		p++;
	}
	return cnt;
}

static int memguard_limit_show(struct seq_file *m, void *v)
{
	int i, cpu;
	cpu = get_cpu();

	seq_printf(m, "cpu  |budget (MB/s,pct,weight)\n");
	seq_printf(m, "-------------------------------\n");

	for_each_online_cpu(i) {
		struct core_info *cinfo = per_cpu_ptr(core_info, i);
		int budget = 0;
		if (cinfo->limit > 0)
			budget = cinfo->limit;

		WARN_ON_ONCE(budget == 0);
		seq_printf(m, "CPU%d: %d (%dMB/s)\n", 
				   i, budget,
				   convert_events_to_mb(budget));
	}

	put_cpu();
	return 0;
}

static int memguard_limit_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, memguard_limit_show, NULL);
}

static const struct file_operations memguard_limit_fops = {
	.open		= memguard_limit_open,
	.write          = memguard_limit_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};



/**
 * Display usage statistics
 *
 * TODO: use IPI
 */
static int memguard_usage_show(struct seq_file *m, void *v)
{
	int i, j;

	/* current utilization */
	for (j = 0; j < 3; j++) {
		for_each_online_cpu(i) {
			struct core_info *cinfo = per_cpu_ptr(core_info, i);
			u64 budget, used, util;

			budget = cinfo->budget;
			used = cinfo->used[j];
			util = div64_u64(used * 100, (budget) ? budget : 1);
			seq_printf(m, "%llu ", util);
		}
		seq_printf(m, "\n");
	}
	seq_printf(m, "<overall>----\n");

	/* overall utilization
	   WARN: assume budget did not changed */
	for_each_online_cpu(i) {
		struct core_info *cinfo = per_cpu_ptr(core_info, i);
		u64 total_budget, total_used, result;

		total_budget = cinfo->overall.assigned_budget;
		total_used   = cinfo->overall.used_budget;
		result       = div64_u64(total_used * 100, 
					 (total_budget) ? total_budget : 1 );
		seq_printf(m, "%lld ", result);
	}
	return 0;
}

static int memguard_usage_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, memguard_usage_show, NULL);
}

static const struct file_operations memguard_usage_fops = {
	.open		= memguard_usage_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#if THROTTLE
DEFINE_SIMPLE_ATTRIBUTE(throttle_fops, NULL, set_throttle_op, "%llu\n");
#endif

static int memguard_init_debugfs(void)
{

	memguard_dir = debugfs_create_dir("memguard", NULL);
	BUG_ON(!memguard_dir);
	debugfs_create_file("control", 0444, memguard_dir, NULL,
			    &memguard_control_fops);

	debugfs_create_file("limit", 0444, memguard_dir, NULL,
			    &memguard_limit_fops);

	debugfs_create_file("usage", 0666, memguard_dir, NULL,
			    &memguard_usage_fops);

#if THROTTLE
  debugfs_create_file("throttle", 0444, memguard_dir, NULL, &throttle_fops);
#endif
  
  debugfs_create_u32("mc_all_avg", 0444, memguard_dir, &mc_all_avg);
  debugfs_create_u32("mc_all_count", 0444, memguard_dir, &mc_all_count);
  debugfs_create_u32("mc_cpu_avg", 0444, memguard_dir, &mc_cpu_avg);
  debugfs_create_u32("mc_cpu_count", 0444, memguard_dir, &mc_cpu_count);
#if THROTTLE 
  debugfs_create_u32("throttle_limit", 0444, memguard_dir, &throttle_limit);
  debugfs_create_u32("throttle_amount", 0444, memguard_dir, &throttle_amount);
#endif

#if THROTTLE
  io_throttle = ioremap(MC_EMEM_ARB_RING1_THROTTLE_0, 32);
  io_limit = ioremap(MC_EMEM_ARB_OUTSTANDING_REQ_0, 32);
  io_arbitration = ioremap(MC_EMEM_ARB_RING0_THROTTLE_MASK_0, 32);
#endif
  io_mc_all_avg_count = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_AVG_COUNT_0, 32);
  io_mc_cpu_avg_count = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_AVG_COUNT_0, 32);

	return 0;
}

static int throttle_thread(void *arg)
{
	int cpunr = (unsigned long)arg;
	struct core_info *cinfo = per_cpu_ptr(core_info, cpunr);

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 9, 0)
	sched_set_fifo(current);
#else
	static const struct sched_param param = {
		.sched_priority = MAX_USER_RT_PRIO/2,
	};

	sched_setscheduler(current, SCHED_FIFO, &param);
#endif

	while (!kthread_should_stop() && cpu_online(cpunr)) {

		DEBUG(trace_printk("wait an event\n"));
		wait_event_interruptible(cinfo->throttle_evt,
					 cinfo->throttled_task ||
					 kthread_should_stop());

		DEBUG(trace_printk("got an event\n"));

		if (kthread_should_stop())
			break;

		while (cinfo->throttled_task && !kthread_should_stop())
		{
			smp_mb();
			cpu_relax();
			/* TODO: mwait */
		}
	}

	DEBUG(trace_printk("exit\n"));
	return 0;
}

int init_module( void )
{
	int i;

	struct memguard_info *global = &memguard_info;

	/* initialized memguard_info structure */
	memset(global, 0, sizeof(struct memguard_info));
	zalloc_cpumask_var(&global->throttle_mask, GFP_NOWAIT);
	zalloc_cpumask_var(&global->active_mask, GFP_NOWAIT);
	g_budget_mb = (int *)kmalloc(num_online_cpus()*sizeof(int), GFP_KERNEL);

	if (g_period_us < 0 || g_period_us > 1000000) {
		printk(KERN_INFO "Must be 0 < period < 1 sec\n");
		return -ENODEV;
	}

	global->period_in_ktime = ktime_set(0, g_period_us * 1000);

	/* initialize all online cpus to be active */
	cpumask_copy(global->active_mask, cpu_online_mask);

	pr_info("NR_CPUS: %d, online: %d\n", NR_CPUS, num_online_cpus());
		
	#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0)
		pr_info("Above\n");
	#else
		pr_info("Below\n");
	#endif
	
	if (g_hw_counter_id >= 0) pr_info("RAW HW COUNTER ID: 0x%x\n", g_hw_counter_id);
	pr_info("HZ=%d, g_period_us=%d\n", HZ, g_period_us);

	pr_info("Initilizing perf counter\n");
	core_info = alloc_percpu(struct core_info);

	get_online_cpus();
	for_each_online_cpu(i) {
		struct core_info *cinfo;
		int budget;

		cinfo = per_cpu_ptr(core_info, i);

		/* initialize counter h/w & event structure */
                g_budget_mb[i] = 100000; // no limit

		budget = convert_mb_to_events(g_budget_mb[i]);
		pr_info("budget[%d] = %d (%d MB)\n", i, budget, g_budget_mb[i]);

		/* initialize per-core data structure */
		memset(cinfo, 0, sizeof(struct core_info));

		/* create performance counter */
		cinfo->event = init_counter(i, budget);
		if (!cinfo->event)
			break;

		/* initialize budget */
		cinfo->budget = cinfo->limit = cinfo->event->hw.sample_period;

		/* throttled task pointer */
		cinfo->throttled_task = NULL;

		init_waitqueue_head(&cinfo->throttle_evt);

		/* initialize statistics */
		/* update local period information */
		cinfo->period_cnt = 0;
		cinfo->used[0] = cinfo->used[1] = cinfo->used[2] =
			cinfo->budget; /* initial condition */
		cinfo->cur_budget = cinfo->budget;
		cinfo->overall.used_budget = 0;
		cinfo->overall.assigned_budget = 0;
		cinfo->overall.throttled_time_ns = 0;
		cinfo->overall.throttled = 0;
		cinfo->overall.throttled_error = 0;

		memset(cinfo->overall.throttled_error_dist, 0, sizeof(int)*10);
		cinfo->throttled_time = ktime_set(0,0);

		print_core_info(smp_processor_id(), cinfo);

		/* initialize nmi irq_work_queue */
		init_irq_work(&cinfo->pending, memguard_process_overflow);

		/* create and wake-up throttle threads */
		cinfo->throttle_thread =
			kthread_create_on_node(throttle_thread,
					       (void *)((unsigned long)i),
					       cpu_to_node(i),
					       "kthrottle/%d", i);

		perf_event_enable(cinfo->event);

		BUG_ON(IS_ERR(cinfo->throttle_thread));
		kthread_bind(cinfo->throttle_thread, i);
		wake_up_process(cinfo->throttle_thread);
	}

	memguard_init_debugfs();

	/* start timer and perf counters */
	pr_info("Start period timer (period=%lld us)\n",
		div64_u64(TM_NS(global->period_in_ktime), 1000));
	on_each_cpu(__start_counter, NULL, 0);

	set_actmon();

#if THROTTLE
  // Initialize Throttle Mechanism
  set_throttle(0);
  set_limit(511);
#endif

  put_online_cpus();

	return 0;
}

void cleanup_module( void )
{
	int i;

	struct memguard_info *global = &memguard_info;

	get_online_cpus();

	/* stop perf_event counters and timers */
	on_each_cpu(__stop_counter, NULL, 0);
	pr_info("LLC bandwidth throttling disabled\n");

	/* destroy perf objects */
	for_each_online_cpu(i) {
		struct core_info *cinfo = per_cpu_ptr(core_info, i);
		pr_info("Stopping kthrottle/%d\n", i);
		kthread_stop(cinfo->throttle_thread);
		perf_event_disable(cinfo->event);
		perf_event_release_kernel(cinfo->event); 
		cinfo->event = NULL; 
	}

	/* remove debugfs entries */
	debugfs_remove_recursive(memguard_dir);

	/* free allocated data structure */
	free_cpumask_var(global->throttle_mask);
	free_cpumask_var(global->active_mask);
	free_percpu(core_info);
	kfree(g_budget_mb);

	put_online_cpus();

  reset_actmon();

	pr_info("module uninstalled successfully\n");
	return;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Heechul Yun <heechul@illinois.edu>");
