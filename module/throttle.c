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
#define MC_EMEM_ARB_OUTSTANDING_REQ_0     0x94
#define MC_EMEM_ARB_RING1_THROTTLE_0      0xe0
// Disable bit 31 for ring1 arbitration
#define MC_EMEM_ARB_RING0_THROTTLE_MASK_0 0x6bc


/**************************************************************************
 * Global Variables
 **************************************************************************/

static struct dentry *throttle_dir = 0;

static struct dentry *debugfs_file = 0;

static u32 throttleAmount = 0;

static volatile u32 *i;

/**************************************************************************
 * Functions 
 **************************************************************************/

static int set_throttle_op(void *data, u64 value)
{
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

static int show(struct seq_file *m, void *v)
{
  seq_printf(m,
    "*i 0x%llx\n"
    "i %p\n"
    "virt_to_phys 0x%llx\n",
    (unsigned long long)*i,
    i,
    (unsigned long long)virt_to_phys((void *)i)
  );
  return 0;
}

static int open(struct inode *inode, struct file *file)
{
  return single_open(file, show, NULL);
}

static const struct file_operations fops = {
  .llseek = seq_lseek,
  .open = open,
  .owner = THIS_MODULE,
  .read = seq_read,
  .release = single_release,
};

static int __init throttle_init(void) {
  printk(KERN_INFO "Throttle module has been loaded\n");

  throttle_init_debugfs();

  i = kmalloc(sizeof(i), GFP_KERNEL);
  *i = 0x12345678;
  debugfs_file = debugfs_create_file(
      "lkmc_virt_to_phys", S_IRUSR, NULL, NULL, &fops);

  return 0;
}

static void __exit throttle_exit(void) {
  printk(KERN_INFO "Throttle module has been unloaded\n");

  /* remove debugfs entries */
  debugfs_remove_recursive(throttle_dir);
  kfree((void*)i);
}

module_init(throttle_init);
module_exit(throttle_exit);
