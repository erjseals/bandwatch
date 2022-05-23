#include <asm/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/timer.h>

// The GPL license is required for the module to load. 
// Without it, insmod reports unknown symbols in the module.
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Seals <ericseals@ku.edu>");

#define TIMER 0

/**************************************************************************
 * Throttle Addresses 
 **************************************************************************/

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
 * Global Variables
 **************************************************************************/

static struct dentry *bandwatch_dir = 0;

static u32 mc_all_avg = 0;
static u32 mc_cpu_avg = 0;
static u32 mc_all_count = 0;
static u32 mc_cpu_count = 0;

static u32 throttle_amount = 0;
static u32 throttle_limit  = 0;

// Timer Values
// 1ms period
int g_time_interval = 1;
struct timer_list g_timer;

// Memory Locations
void __iomem *io_throttle;
void __iomem *io_limit;
void __iomem *io_arbitration;
void __iomem *io_mc_all_avg_count;
void __iomem *io_mc_cpu_avg_count;

/**************************************************************************
 * Functions 
 **************************************************************************/

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

static int set_limit_op(void *data, u64 value)
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

int set_throttle(u32 value)
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

void _TimerHandler(unsigned long data)
{
  u32 mc_gpu_avg = 0;

  /* 1. Check Current MC Utilization */
  /* 2.a. If above threshold, increase throttle */
  /* 2.b. If below threshold, decrease throttle */
  
  mc_all_avg = ioread32(io_mc_all_avg_count);
  mc_cpu_avg = ioread32(io_mc_cpu_avg_count);
  mc_gpu_avg = mc_all_avg - mc_cpu_avg;

  // Sampling Period mismatch, ignore result
  // This only occurs with only CPU memory traffic
  // so this isn't a significant issue
  if (mc_all_avg > mc_cpu_avg) {
    trace_printk("mc_all_avg: %u, mc_cpu_avg: %u, mc_gpu_avg: %u\n", mc_all_avg, mc_cpu_avg, mc_gpu_avg);
    if (mc_all_avg > MAX_UTILIZATION) {
      if (throttle_amount < THROTTLE_MAX) {
        set_throttle(throttle_amount + 1);
        trace_printk("mc_all_avg above: %u, throttle_amount: %u\n", mc_all_avg, throttle_amount);
      }
    }

    if (mc_all_avg <= MAX_UTILIZATION) {
      if (throttle_amount > THROTTLE_MIN) {
        set_throttle(throttle_amount - 1);
        trace_printk("mc_all_avg below: %u, throttle_amount: %u\n", mc_all_avg, throttle_amount);
      }
    }
  }

#if TIMER
  /* Rewind the Timer */
  mod_timer(&g_timer, jiffies + msecs_to_jiffies(g_time_interval));
#endif
}

static int set_actmon(void)
{
  // Initialize Global Period
  // Writing 1/0 to bit 8 sets period time base in usec/msec
  // Writing n to bits 7:0 creates a n+1 usec/msec sampling period

  // Setting for 10 usec
  // (1 << 8) | (0x9)
  u32 bitWise = 0x109;
  void __iomem *io = ioremap(ACTMON_ADDRESS + ACTMON_GLB_PERIOD_CTRL_0, 32);
  iowrite32( bitWise , io);

  // mc_all 
  // Enable MC Activity Monitor
  // Write 1 to bit 31
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_CTRL_0, 32);
  bitWise = (1 << 31);
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
  // Write 1 to bit 31
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_CTRL_0, 32);
  bitWise = (1 << 31);
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

  // Setting for 10 usec
  // (1 << 8) | (0x9)
  u32 bitWise = 0x19;
  void __iomem *io = ioremap(ACTMON_ADDRESS + ACTMON_GLB_PERIOD_CTRL_0, 32);
  iowrite32( bitWise , io);

  // mc_cpu
  // Enable MC Activity Monitor
  // Write 1 to bit 31
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_CTRL_0, 32);
  bitWise = (0 << 31);
  iowrite32( (ioread32(io) | bitWise) , io);

  return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(throttle_fops, NULL, set_throttle_op, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(limit_fops, NULL, set_limit_op, "%llu\n");

static int bandwatch_init_debugfs(void)
{
  struct dentry *junk;

  bandwatch_dir = debugfs_create_dir("bandwatch", NULL);
  if (!bandwatch_dir) {
    // Abort module load
    trace_printk("debugfs: failed to create /sys/kernel/debug/bandwatch\n");
    return -1;
  }

  junk = debugfs_create_file(
          "throttle",
          0444,
          bandwatch_dir,
          NULL,
          &throttle_fops);
  if (!junk) {
    trace_printk("debugfs: failed to create /sys/kernel/debug/bandwatch/throttle\n");
  }

  junk = debugfs_create_file(
          "limit",
          0444,
          bandwatch_dir,
          NULL,
          &limit_fops);
  if (!junk) {
    trace_printk("debugfs: failed to create /sys/kernel/debug/bandwatch/limit\n");
  }

  debugfs_create_u32("mc_all_avg", 0444, bandwatch_dir, &mc_all_avg);
  debugfs_create_u32("mc_all_count", 0444, bandwatch_dir, &mc_all_count);
  debugfs_create_u32("mc_cpu_avg", 0444, bandwatch_dir, &mc_cpu_avg);
  debugfs_create_u32("mc_cpu_count", 0444, bandwatch_dir, &mc_cpu_count);
  debugfs_create_u32("throttle_limit", 0444, bandwatch_dir, &throttle_limit);
  debugfs_create_u32("throttle_amount", 0444, bandwatch_dir, &throttle_amount);

  // Initialize Memory Locations
  io_throttle = ioremap(MC_EMEM_ARB_RING1_THROTTLE_0, 32);
  io_limit = ioremap(MC_EMEM_ARB_OUTSTANDING_REQ_0, 32);
  io_arbitration = ioremap(MC_EMEM_ARB_RING0_THROTTLE_MASK_0, 32);
  io_mc_all_avg_count = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_AVG_COUNT_0, 32);
  io_mc_cpu_avg_count = ioremap(ACTMON_ADDRESS + ACTMON_MCCPU_AVG_COUNT_0, 32);

  return 0;
}


static int __init bandwatch_init(void) {
  bandwatch_init_debugfs();

#if TIMER
  /* Start the periodic timer */
  // 
  // timer_setup(&g_timer, _TimerHandler, 0);
  setup_timer(&g_timer, _TimerHandler, 0);
  mod_timer(&g_timer, jiffies + msecs_to_jiffies(g_time_interval));
#endif

  set_actmon();

  trace_printk("bandwatch module has been loaded\n");
  return 0;
}

static void __exit bandwatch_exit(void) {
  /* remove debugfs entries */
  debugfs_remove_recursive(bandwatch_dir);

#if TIMER
  /* Delete the timer */
  del_timer(&g_timer);
#endif

  reset_actmon();

  trace_printk("bandwatch module has been unloaded\n");
}

module_init(bandwatch_init);
module_exit(bandwatch_exit);
