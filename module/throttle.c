#include <asm/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

// The GPL license is required for the module to load. 
// Without it, insmod reports unknown symbols in the module.
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Seals <ericseals@ku.edu>");

#define MC_ADDRESS 0x70019000
#define MC_EMEM_ARB_OUTSTANDING_REQ_0     0x70019094
#define MC_EMEM_ARB_RING1_THROTTLE_0      0x700190e0
// Disable bit 31 for ring1 arbitration
#define MC_EMEM_ARB_RING0_THROTTLE_MASK_0 0x700196bc


/**************************************************************************
 * Global Variables
 **************************************************************************/

static struct dentry *throttle_dir = 0;

static u32 throttleAmount = 0;

/**************************************************************************
 * Functions 
 **************************************************************************/

static int set_throttle_op(void *data, u64 value)
{
  // Set Request Limit (If outstanding requests exceed, throttling initiates) 
  // 31st bit LOW - don't limit requests, we want them to exceed the arbitration limit 
  //
  // Bits 8:0 sets the request limit
  u32 bitWise = ~(1 << 31);
  u32 limit = 0x000001FF;
 
  void __iomem *io = ioremap(MC_EMEM_ARB_OUTSTANDING_REQ_0, 32);
  iowrite32( ( (ioread32(io) & bitWise) | limit ) , io);

  // Define Throttling amount
  // Bits 20:16 set throttling amount when limit is exceeded
  bitWise = 0x001f0000;
  io = ioremap(MC_EMEM_ARB_RING1_THROTTLE_0, 32);
  iowrite32( (ioread32(io) | bitWise) , io);

  // Enable Ring1 Arbitration
  bitWise = 0x80008041;
  io = ioremap(MC_EMEM_ARB_RING0_THROTTLE_MASK_0, 32);
  iowrite32( (ioread32(io) | bitWise) , io);

  throttleAmount = value;
  return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(add_fops, NULL, set_throttle_op, "%llu\n");

static int throttle_init_debugfs(void)
{
  struct dentry *junk;

  throttle_dir = debugfs_create_dir("throttle", NULL);
  if (!throttle_dir) {
    // Abort module load
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/throttle\n");
    return -1;
  }

  junk = debugfs_create_file(
          "set",
          0444,
          throttle_dir,
          NULL,
          &add_fops);
  if (!junk) {
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/throttle/set\n");
  }

  debugfs_create_u32("limit", 0444, throttle_dir, &throttleAmount);

  return 0;
}


static int __init throttle_init(void) {
  printk(KERN_INFO "Throttle module has been loaded\n");

  throttle_init_debugfs();

  return 0;
}

static void __exit throttle_exit(void) {
  printk(KERN_INFO "Throttle module has been unloaded\n");

  /* remove debugfs entries */
  debugfs_remove_recursive(throttle_dir);
}

module_init(throttle_init);
module_exit(throttle_exit);
