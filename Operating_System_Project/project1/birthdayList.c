#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/*Define the "birthday" struct*/
struct birthday{
	int day;
	int month;
	int year;
	struct list_head list;	
};

/*Define five "birthday" structs*/
struct birthday *person1;
struct birthday *person2;
struct birthday *person3;
struct birthday *person4;
struct birthday *person5;

/*Declare a list_head object*/
static LIST_HEAD(birthday_list);


/*This function is called when this module is loaded.*/
int birthdayList_init(void)
{	
	/*Once enter into the init function, initialize the instances 
	of five struct birthday respectively, and add them into 
	the list by using the "list_add_tail() function"*/
	printk("Init Module:\n");

	person1 = kmalloc(sizeof(*person1), GFP_KERNEL);
	person1->day = 2;
	person1->month = 8;
	person1->year = 1995;
	INIT_LIST_HEAD(&person1->list);
	list_add_tail(&(person1->list), &(birthday_list));

	person2 = kmalloc(sizeof(*person2), GFP_KERNEL);
	person2->day = 1;
	person2->month = 4;
	person2->year = 1992;
	INIT_LIST_HEAD(&person2->list);
	list_add_tail(&(person2->list), &(birthday_list));
	
	person3 = kmalloc(sizeof(*person3), GFP_KERNEL);
	person3->day = 10;
	person3->month = 2;
	person3->year = 1993;
	INIT_LIST_HEAD(&person3->list);
	list_add_tail(&(person3->list), &(birthday_list));

	person4 = kmalloc(sizeof(*person4), GFP_KERNEL);
	person4->day = 4;
	person4->month = 5;
	person4->year = 1991;
	INIT_LIST_HEAD(&person4->list);
	list_add_tail(&(person4->list), &(birthday_list));

	person5 = kmalloc(sizeof(*person5), GFP_KERNEL);
	person5->day = 18;
	person5->month = 12;
	person5->year = 1994;
	INIT_LIST_HEAD(&person5->list);
	list_add_tail(&(person5->list), &(birthday_list));

	/*After adding five structs into list, use the "list_for_each_entry()"
	function to traverse the linked list, and print their contents 
	to the kernel log buffer"*/
	struct birthday *ptr;
	list_for_each_entry(ptr, &birthday_list, list){
	printk(KERN_INFO "The person's birthday is %d/%d/%d(Day/Month/Year)\n", ptr->day,ptr->month,ptr->year);
	}

       return 0;
}

/*This function is called when this module is unloaded.*/
void birthdayList_exit(void) {

	/*Once enter into the exit function, delete the list by using
	the "list_for_each_entry_safe() function, free the kernel memory.*/
	printk("Exit Module:\n");
	struct birthday *ptr, *next;
	list_for_each_entry_safe(ptr,next,&birthday_list,list){	
		list_del(&ptr->list);
		kfree(ptr);
	}

	/*After free the memory from the list, verify the list by using
	the "list_empty" function, which is defined in list.h*/
	if (list_empty(&birthday_list)){
		printk("The list has been removed.");
	}
	else{
		printk("The list has not been removed.");
	}

}

module_init( birthdayList_init );
module_exit( birthdayList_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("birthdayList Module");
MODULE_AUTHOR("Shuaiqi Cao");
