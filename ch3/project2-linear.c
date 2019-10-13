#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>

/* This function is called when the module is loaded. */
int simple_init(void)
{
	struct task_struct *task;

       printk(KERN_INFO "Listing Modules Linearly\n");

	for_each_process(task)
	{
		printk(KERN_INFO "Name: %s | State: %ld | Process ID: %d \n",task->comm,task->state,task->pid);
	}

       return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
	printk(KERN_INFO "Removing Linear Listing\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Linear Listing Task");
MODULE_AUTHOR("SGG");

