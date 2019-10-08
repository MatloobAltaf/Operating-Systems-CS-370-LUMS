#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/list.h>
//to use kmalloc() for memory allocation
#include <linux/slab.h>

static LIST_HEAD(birthday_list);

struct birthday{
	int day;
	int month;
	int year;
	struct list_head list;
};


/* This function is called when the module is loaded. */
int simple_init(void)
{

	struct birthday *person;
	struct birthday *ptr;
	int i;
	for(i = 0; i < 5; i++){  
person = kmalloc(sizeof(*person),GFP_KERNEL);
	person->day = 1;
	person->month = 1;
	person->year = 2000+i;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list,&birthday_list);
}


	list_for_each_entry(ptr,&birthday_list,list){
	printk(KERN_INFO "BIRTHDAY: %u-%u-%u",ptr->day,ptr->month,ptr->year);
} 
	printk(KERN_INFO "Module Loaded\n");
       return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {

	struct birthday *ptr; 
	struct birthday *next;
	list_for_each_entry_safe(ptr,next,&birthday_list,list){
	list_del(&ptr->list);
	kfree(ptr);
	}
	printk(KERN_INFO "Module Removed\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");

