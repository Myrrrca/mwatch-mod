#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>

static int value = 0;
static struct task_struct *test_mwatch_thread = NULL;

// incrementing global *value* every 15 seconds
static int value_inc(void *data) 
{
    while (!kthread_should_stop()) {
        ++value;
        msleep(15000); 
        pr_info("%s: value incremented. value = %d\n", THIS_MODULE->name, value);
        pr_info("%s: value address: 0x%px\n", THIS_MODULE->name, &value);
    }

    return 0;
}
      
static int __init test_mwatch_init(void) 
{
    pr_info("%s: loading module...\n", THIS_MODULE->name);
    pr_info("%s: Value address: 0x%px\n", THIS_MODULE->name, &value);

    test_mwatch_thread = kthread_run(value_inc, NULL, "test_mwatch_thread");

    if (IS_ERR(test_mwatch_thread)) { 
        pr_info("%s: unable to start kernel thread\n", THIS_MODULE->name);
        int err = PTR_ERR(test_mwatch_thread);
        test_mwatch_thread = NULL;
        return err;
    }

  pr_info("%s: module loaded!\n", THIS_MODULE->name);
	return 0;
}
 
static void __exit test_mwatch_exit(void) 
{
  kthread_stop(test_mwatch_thread);
	pr_info("%s: module unloaded!\n", THIS_MODULE->name);
}

module_init(test_mwatch_init);
module_exit(test_mwatch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Myrrrca, myrrrrrca@gmail.com");
MODULE_DESCRIPTION("Testing watchpoint kernel module (for mwatch module)");
