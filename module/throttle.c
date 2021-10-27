#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>

// The GPL license is required for the module to load. 
// Without it, insmod reports unknown symbols in the module.
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Seals <ericseals@ku.edu>");

/**************************************************************************
 * Global Variables
 **************************************************************************/

static struct dentry *throttle_dir = 0;

static u32 hello = 0;

/**************************************************************************
 * Functions 
 **************************************************************************/

static int throttle_init_debugfs(void)
{

  throttle_dir = debugfs_create_dir("throttle", NULL);
  if (!throttle_dir) {
    // Abort module load
    printk(KERN_ALERT "debugfs: failed to create /sys/kernel/debug/control\n");
    return -1;
  }

  debugfs_create_u32("hello", 0666, throttle_dir, &hello);

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
