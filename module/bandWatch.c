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
#define ACTMON_MCALL_COUNT_0                0x1dc
#define ACTMON_MCALL_AVG_COUNT_0	          0x1e0

// Only CPU Memory Controller Traffic
#define ACTMON_MCCPU_CTRL_0		              0x200
#define ACTMON_MCCPU_INIT_AVG_0		          0x20c
#define ACTMON_MCCPU_COUNT_0		            0x21c
#define ACTMON_MCCPU_AVG_COUNT_0	          0x220

/**************************************************************************
 * Constants 
 **************************************************************************/

#define THROTTLE_MAX  10
#define THROTTLE_MIN  0
#define MAX_BANDWIDTH 10000


/**************************************************************************
 * Global Variables
 **************************************************************************/

static struct dentry *bandWatch_dir = 0;

static u32 enableActmon = 0;
static u32 MCALL_AVG = 0;

static u32 throttleAmount = 0;
static u32 throttleLimit  = 0;

// Timer Values
// 1ms period
int g_time_interval = 1000;
struct timer_list g_timer;

// Memory Locations
void __iomem *io_throttle;
void __iomem *io_limit;
void __iomem *io_arbitration;
void __iomem *io_avgcount;

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

  throttleAmount = value;
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

  throttleLimit = value;
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

  throttleAmount = value;
  return 0;
}

//void _TimerHandler(struct timer_list *timer)
void _TimerHandler(unsigned long data)
{
  /* 1. Check Current MC Bandwidth */
  /* 2.a. If above threshold, increase throttle */
  /* 2.b. If below threshold, decrease throttle */

  MCALL_AVG = ioread32(io_avgcount);

  if (MCALL_AVG > MAX_BANDWIDTH) {
    if (throttleAmount < THROTTLE_MAX) {
      set_throttle(throttleAmount + 1);
      printk(KERN_INFO "MCALL above: %u, throttleAmount: %u\n", MCALL_AVG, throttleAmount);
    }
  }

  if (MCALL_AVG <= MAX_BANDWIDTH) {
    if (throttleAmount > THROTTLE_MIN) {
      set_throttle(throttleAmount - 1);
      printk(KERN_INFO "MCALL below: %u, throttleAmount: %u\n", MCALL_AVG, throttleAmount);
    }
  }

  /* Rewind the Timer */
  mod_timer(&g_timer, jiffies + msecs_to_jiffies(g_time_interval));
}

static int set_actmon_op(void *data, u64 value)
{
  // Initialize Global Enable
  // MCALL bit 9
  // MCCPU bit 8 
  u32 bitWise = (1 << 9);

  void __iomem *io = ioremap(ACTMON_ADDRESS + ACTMON_GLB_STATUS_0, 32);
  iowrite32( (ioread32(io) | bitWise) , io);

  // Initialize Global Period
  // Writing 1/0 to bit 8 sets period time base in usec/msec
  // Writing n to bits 7:0 creates a n+1 usec/msec sampling period

  // Setting for 10 usec
  bitWise = (1 << 8) | (0x9);

  io = ioremap(ACTMON_ADDRESS + ACTMON_GLB_PERIOD_CTRL_0, 32);
  iowrite32( bitWise , io);

  // MCALL
  //
  // Initialize the AVG Count to 0
  bitWise = 0;
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_INIT_AVG_0, 32);
  iowrite32( bitWise , io);

  // Enable MC Activity Monitor
  // Write 1 to bit 31
  io = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_CTRL_0, 32);

  bitWise = (1 << 31);
  iowrite32( (ioread32(io) | bitWise) , io);

  enableActmon = bitWise;
  return 0;
}

// Read the number of avg cycles 
// This will be placed in file mcall_count
static int set_mcall_count_op(void *data, u64 value)
{
  MCALL_AVG = ioread32(io_avgcount);
  return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(actmon_fops, NULL, set_actmon_op, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(mcall_count_fops, NULL, set_mcall_count_op, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(throttle_fops, NULL, set_throttle_op, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(limit_fops, NULL, set_limit_op, "%llu\n");

static int bandWatch_init_debugfs(void)
{
  struct dentry *junk;

  bandWatch_dir = debugfs_create_dir("bandWatch", NULL);
  if (!bandWatch_dir) {
    // Abort module load
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/bandWatch\n");
    return -1;
  }

  junk = debugfs_create_file(
          "throttle",
          0444,
          bandWatch_dir,
          NULL,
          &throttle_fops);
  if (!junk) {
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/bandWatch/throttle\n");
  }

  junk = debugfs_create_file(
          "limit",
          0444,
          bandWatch_dir,
          NULL,
          &limit_fops);
  if (!junk) {
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/bandWatch/limit\n");
  }

  junk = debugfs_create_file(
          "actmon",
          0444,
          bandWatch_dir,
          NULL,
          &actmon_fops);
  if (!junk) {
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/bandWatch/actmon\n");
  }
  
  junk = debugfs_create_file(
          "mcall_count",
          0444,
          bandWatch_dir,
          NULL,
          &mcall_count_fops);
  if (!junk) {
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/bandWatch/mcall_count\n");
  }

  debugfs_create_u32("enableActmon", 0444, bandWatch_dir, &enableActmon);
  debugfs_create_u32("MCALL_AVG", 0444, bandWatch_dir, &MCALL_AVG);
  debugfs_create_u32("throttleLimit", 0444, bandWatch_dir, &throttleLimit);
  debugfs_create_u32("throttleAmount", 0444, bandWatch_dir, &throttleAmount);

  // Initialize Memory Locations
  io_throttle = ioremap(MC_EMEM_ARB_RING1_THROTTLE_0, 32);
  io_limit = ioremap(MC_EMEM_ARB_OUTSTANDING_REQ_0, 32);
  io_arbitration = ioremap(MC_EMEM_ARB_RING0_THROTTLE_MASK_0, 32);
  io_avgcount = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_AVG_COUNT_0, 32);

  return 0;
}


static int __init bandWatch_init(void) {
  bandWatch_init_debugfs();

  /* Start the periodic timer */
  // timer_setup(&g_timer, _TimerHandler, 0);
  setup_timer(&g_timer, _TimerHandler, 0);
  mod_timer(&g_timer, jiffies + msecs_to_jiffies(g_time_interval));

  printk(KERN_INFO "bandWatch module has been loaded\n");
  return 0;
}

static void __exit bandWatch_exit(void) {
  /* remove debugfs entries */
  debugfs_remove_recursive(bandWatch_dir);

  /* Delete the timer */
  del_timer(&g_timer);

  printk(KERN_INFO "bandWatch module has been unloaded\n");
}

module_init(bandWatch_init);
module_exit(bandWatch_exit);
