#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

static unsigned long long mwatch_addr = 0x0; // default addr
static struct perf_event * __percpu *hw_bp_event = NULL;
bool hw_bp_registred = false;

static int setup_watchpoint(void);

// function for notifying write syscalls on sysfs module's entry
static int notify_param_set(const char *val, const struct kernel_param *kp)
{
  int res = param_set_ulong(val, kp); // Use helper for write variable
  if (res == 0) {
    pr_info("%s: write call back function called...\n", THIS_MODULE->name);
    pr_info("%s: new value of mwatch_addr = 0x%llx\n", THIS_MODULE->name,
              mwatch_addr);

    if (setup_watchpoint() != 0) {
      pr_err("%s: failed to update watchpoint to new 0x%llx address\n",
             THIS_MODULE->name, mwatch_addr);
    }

    return 0;
  }
  return -1;
}

// function for notifying read syscalls on sysfs module's entry
static int notify_param_get(char *buffer, const struct kernel_param *kp)
{
  pr_info("%s: read callback function called...\n", THIS_MODULE->name);
  pr_info("%s: value of mwatch_addr = 0x%llx\n", THIS_MODULE->name, mwatch_addr);
  return 0;
}

const struct kernel_param_ops my_param_ops = 
{
  .set = &notify_param_set,   // use our function as setter 
  .get = &notify_param_get,   // use our function as getter
};

// creating our module's entry at /sys/modules/mwatch/parameters/mwatch_addr and 
// linking our Read/Write callbacks and 
// setting RW only for user
module_param_cb(mwatch_addr, &my_param_ops, &mwatch_addr, S_IRUSR | S_IWUSR);

typedef void (*access_callback_t)(void);

static void hw_bp_handler(struct perf_event *bp, struct perf_sample_data *data, 
                          struct pt_regs *regs) 
{ 
    pr_info("%s: watchpoint triggered at address: 0x%llx\n", THIS_MODULE->name, 
            mwatch_addr);

    pr_info("%s: dumping stack:\n", THIS_MODULE->name);
    // this function is responsible for printing call stack(backtrace)
    dump_stack();
    pr_info("%s: end of stack dump\n", THIS_MODULE->name);
}

static int setup_watchpoint(void) 
{
  if (hw_bp_registred) {
    unregister_wide_hw_breakpoint(hw_bp_event);
    hw_bp_registred = false;
    pr_info("%s: previous hardware breakpoint unregistered\n", THIS_MODULE->name);
  }

  struct perf_event_attr attr;
  
  hw_breakpoint_init(&attr);
  attr.bp_addr = mwatch_addr;
  attr.bp_len = HW_BREAKPOINT_LEN_4;
  attr.bp_type = HW_BREAKPOINT_W | HW_BREAKPOINT_R;  // Watch for read and write

  hw_bp_event = register_wide_hw_breakpoint(&attr, hw_bp_handler, NULL);

  if (IS_ERR(hw_bp_event)) {
    pr_err("%s: failed to register hardware breakpoint\n", THIS_MODULE->name);
    return PTR_ERR(hw_bp_event);
  }

  hw_bp_registred = true;
  pr_info("%s: hardware breakpoint registered at address: 0x%llx\n", 
          THIS_MODULE->name, mwatch_addr);
  return 0;
}

static int __init mwatch_init(void) 
{
  pr_info("%s: loading with default mwatch_addr(0x%llx)...\n", THIS_MODULE->name,
          mwatch_addr);
  
  if (setup_watchpoint() != 0) {
    pr_err("%s: failed to load\n", THIS_MODULE->name);
    return -1;
  }

  pr_info("%s: module loaded!\n", THIS_MODULE->name);
  return 0;
}

static void __exit mwatch_exit(void) 
{
  if (hw_bp_registred) {
      unregister_wide_hw_breakpoint(hw_bp_event);
      hw_bp_registred = false;
  }
  pr_info("%s: module unloaded!\n", THIS_MODULE->name);
}

module_init(mwatch_init);
module_exit(mwatch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Myrrrca, myrrrrrca@gmail.com");
MODULE_DESCRIPTION("Watchpoint kernel module (for R/W)");
