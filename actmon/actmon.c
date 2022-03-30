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


// Base Address
#define ACTMON_ADDRESS                0x6000c800

// Offsets

// Global Enable
#define ACTMON_GLB_STATUS_0		        0x0
#define ACTMON_GLB_PERIOD_CTRL_0      0x4

// All Memory Controller Traffic
#define ACTMON_MCALL_CTRL_0		        0x1c0
#define ACTMON_MCALL_INIT_AVG_0       0x1cc
#define ACTMON_MCALL_COUNT_0          0x1dc
#define ACTMON_MCALL_AVG_COUNT_0	    0x1e0

// Only CPU Memory Controller Traffic
#define ACTMON_MCCPU_CTRL_0		        0x200
#define ACTMON_MCCPU_INIT_AVG_0		    0x20c
#define ACTMON_MCCPU_COUNT_0		      0x21c
#define ACTMON_MCCPU_AVG_COUNT_0	    0x220


/**************************************************************************
 * Global Variables
 **************************************************************************/

static struct dentry *actmon_dir = 0;

static u32 EnableACTMON = 0;
static u32 MCALL_AVG = 0;

// Timer Values
// 1ms period
int g_time_interval = 1000;
struct timer_list g_timer;

/**************************************************************************
 * Functions 
 **************************************************************************/

void _TimerHandler(unsigned long data)
{
  /* 1. Check Current MC Bandwidth */


  /* 2.a. If above threshold, increase throttle */

  /* 2.b. If below threshold, decrease throttle */







  /* Rewind the Timer */
  mod_timer(&g_timer, jiffies + msecs_to_jiffies(g_time_interval));

  printk(KERN_INFO "Timer reset by Handler\n");
}

// value is unused, clean this up later
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
  //bitWise = 0;

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

  EnableACTMON = bitWise;
  return 0;
}

static int set_mcall_count_op(void *data, u64 value)
{
  // Read the number of avg cycles 
  // This will be placed in file mcall_count
  void __iomem *io = ioremap(ACTMON_ADDRESS + ACTMON_MCALL_AVG_COUNT_0, 32);

  MCALL_AVG = ioread32(io);
  return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(actmon_fops, NULL, set_actmon_op, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(mcall_count_fops, NULL, set_mcall_count_op, "%llu\n");

static int actmon_init_debugfs(void)
{
  struct dentry *junk;

  actmon_dir = debugfs_create_dir("actmon", NULL);
  if (!actmon_dir) {
    // Abort module load
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/actmon\n");
    return -1;
  }

  junk = debugfs_create_file(
          "actmon",
          0444,
          actmon_dir,
          NULL,
          &actmon_fops);
  if (!junk) {
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/actmon/actmon\n");
  }
  
  junk = debugfs_create_file(
          "mcall_count",
          0444,
          actmon_dir,
          NULL,
          &mcall_count_fops);
  if (!junk) {
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/actmon/mcall_count\n");
  }

  debugfs_create_u32("EnableACTMON", 0444, actmon_dir, &EnableACTMON);
  debugfs_create_u32("MCALL_AVG", 0444, actmon_dir, &MCALL_AVG);

  return 0;
}


static int __init actmon_init(void) {
  actmon_init_debugfs();

  /* Start the periodic timer */
  setup_timer(&g_timer, _TimerHandler, 0);
  mod_timer(&g_timer, jiffies + msecs_to_jiffies(g_time_interval));

  printk(KERN_INFO "ACTMON module has been loaded\n");
  return 0;
}

static void __exit actmon_exit(void) {
  /* remove debugfs entries */
  debugfs_remove_recursive(actmon_dir);

  /* Delete the timer */
  del_timer(&g_timer);

  printk(KERN_INFO "ACTMON module has been unloaded\n");
}

module_init(actmon_init);
module_exit(actmon_exit);
