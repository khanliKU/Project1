#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/path.h>
#include <linux/namei.h>

struct fileinfo304 {
	size_t size;
	char filename[15];
    char datecreated[15];
	int owner_id;
    int file_id;
	struct list_head list;
};

/**
 * The following defines and initializes a list_head object named files_list
 */

static LIST_HEAD(files_list);
int ownerID = 0;
struct fileinfo304 *ptr;
struct fileinfo304 *next;

module_param(ownerID,int,0);

int fileinfo304_init(void)
{
	printk(KERN_INFO "Loading Module\n");
	
	// declare fileinfo304
	struct fileinfo304 *file0;
	struct fileinfo304 *file1;
	struct fileinfo304 *file2;
	struct fileinfo304 *file3;
	struct fileinfo304 *file4;
	struct fileinfo304 *file5;
	int removedNo = 0;
	int listSize = 0;
	// file0
	file0 = (struct fileinfo304 *) kmalloc( sizeof( struct fileinfo304), GFP_KERNEL);
	file0->size = 25;
	sprintf(file0->filename, "file0.txt");
	sprintf(file0->datecreated, "25 Feb 14:29");
	file0->owner_id = 2;
	file0->file_id = 806068;
	INIT_LIST_HEAD(&file0->list);
	list_add_tail(&file0->list, &files_list);
	// file1
	file1 = (struct fileinfo304 *) kmalloc( sizeof( struct fileinfo304), GFP_KERNEL);
	file1->size = 25;
	sprintf(file1->filename,"file1.txt");
	sprintf(file1->datecreated,"05 Feb 23:29");
	file1->owner_id = 1;
	file1->file_id = 806094;
	INIT_LIST_HEAD(&file1->list);
	list_add_tail(&file1->list, &files_list);
	// file2
	file2 = (struct fileinfo304 *) kmalloc( sizeof( struct fileinfo304), GFP_KERNEL);
	file2->size = 25;
	sprintf(file2->filename,"file2.txt");
	sprintf(file2->datecreated,"15 Mar 14:29");
	file2->owner_id = 1;
	file2->file_id = 806066;
	INIT_LIST_HEAD(&file2->list);
	list_add_tail(&file2->list,&files_list);
	// file3
	file3 = (struct fileinfo304 *) kmalloc( sizeof( struct fileinfo304), GFP_KERNEL);
	file3->size = 25;
	sprintf(file3->filename,"file3.txt");
	sprintf(file3->datecreated,"22 Feb 14:00");
	file3->owner_id = 3;
	file3->file_id = 806092;
	INIT_LIST_HEAD(&file3->list);
	list_add_tail(&file3->list,&files_list);
	// file4
	file4 = (struct fileinfo304 *) kmalloc( sizeof( struct fileinfo304), GFP_KERNEL);
	file4->size = 25;
	sprintf(file4->filename,"file4.txt");
	sprintf(file4->datecreated,"01 Sep 10:00");
	file4->owner_id = 2;
	file4->file_id = 806100;
	INIT_LIST_HEAD(&file4->list);
	list_add_tail(&file4->list,&files_list);

	// file5
	file5 = (struct fileinfo304 *) kmalloc( sizeof( struct fileinfo304), GFP_KERNEL);
	file5->size = 25;
	sprintf(file5->filename,"file5.txt");
	sprintf(file5->datecreated,"25 Feb 14:29");
	file5->owner_id = 3;
	file5->file_id = 789815;
	INIT_LIST_HEAD(&file5->list);
	list_add_tail(&file5->list,&files_list);

	// iteration over list

	list_for_each_entry(ptr, &files_list, list)
	{
		printk("Adding: Name: %s Size: %d Creating Date: %s OwnerID: %d FileID: %d\n",
			ptr->filename,ptr->size,ptr->datecreated,
			ptr->owner_id,ptr->file_id);
		listSize++;
	}

	// Part III

	list_for_each_entry_safe(ptr, next, &files_list, list)
	{
		if (ownerID != 0 && ptr != NULL && ptr->owner_id == ownerID)
		{
			list_del(&ptr->list);
			kfree(ptr);
			removedNo++;
		}
	}
	printk("Removed Files: # removed: %d # Remaining: %d\n",removedNo,listSize - removedNo);
    return 0;
}

void fileinfo304_exit(void)
{
	printk(KERN_INFO "Removing Module\n");
	
	list_for_each_entry_safe(ptr, next, &files_list, list)
	{
		// on each iteration ptr points 
		// to the next birthday struct
		printk("Removing: Name: %s Size: %d Creating Date: %s OwnerID: %d FileID: %d\n",
			ptr->filename,ptr->size,ptr->datecreated,
			ptr->owner_id,ptr->file_id);
		list_del(&ptr->list);
		kfree(ptr);
	}
}



module_init( fileinfo304_init);
module_exit( fileinfo304_exit);

MODULE_LICENSE( "GPL");
MODULE_DESCRIPTION( "Exercise for COMP304");
MODULE_AUTHOR("Kutlay Hanli");

/*
 * sudo insmod ...
 * sudo rmmod ...
 * dmesg
 */