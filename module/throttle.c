#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init throttle_init(void) {
  printk(KERN_INFO "Throttle module has been loaded\n");
  return 0;
}

static void __exit throttle_exit(void) {
  printk(KERN_INFO "Throttle module has been unloaded\n");
}

module_init(throttle_init);
module_exit(throttle_exit);

MODULE_LICENSE("GPL");
MODULE_LICENSE("Eric Seals <ericseals@ku.edu>");
