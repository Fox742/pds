#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/proc_fs.h>   // for proc interconnection
#include <linux/string.h>    // for string functions using
#include <linux/slab.h>	     // kzalloc and co
#include <linux/uaccess.h>   // copy_from_user
#include <linux/seq_file.h>

static struct proc_dir_entry *proc_entry;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lakshmanan");
MODULE_DESCRIPTION("Process delaying Shedule");

char * procEntryName="pds";

//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
ssize_t pid_write(struct file *filp, const char *buf,size_t count, loff_t  *offp)
{
	//printk(KERN_INFO "\nWriting some data...\n");

	const unsigned long maxlen = 4096;
	char* data; //считываемые данные
	size_t len;
	len=min(maxlen, count);

	data = kzalloc (len, GFP_KERNEL);
	if (copy_from_user (data, buf, len))
	{
		printk(KERN_INFO "Error on 'copy_from_user' procedure\n");
		return EFAULT;
	}

	char command = 0;
	int processIdToOperate=-1;
	int timeNotToShedule=-1;

	char command2 = 0;
	int processIdToOperate2=-1;
	//sscanf (data, "%c ", &command);
	//sscanf (data, "%d", &processIdToOperate);		
	
	//sscanf (data, "%c %d %d", &command, &processIdToOperate, &timeNotToShedule); //считываем данные
	sscanf (data, "%c %d", &command, &processIdToOperate); //считываем данные
	
	
	switch (command)
	{

		case 'p':
			printk(KERN_INFO "Arrived pause command			---------->\n");
			printk(KERN_INFO "Process: %d				---------->\n",processIdToOperate);
			// Чтобы прочесть колличество времени нам нужно перепрочитать сначала строку data
			sscanf (data, "%c %d %d", &command, &processIdToOperate, &timeNotToShedule); //считываем данные

			printk(KERN_INFO "Number of milliseconds to stop: %d	---------->\n",timeNotToShedule);
			

		break;

		case 'r':
			printk(KERN_INFO "Arrived resume command		---------->\n");
			printk(KERN_INFO "Process: %d				---------->\n",processIdToOperate);
		break;


	


	}
	


	return len;
}


struct file_operations hello_proc_fops = {
  owner: THIS_MODULE,
  write: pid_write
};

static int __init hello_init(void)
{
	int ret;
	//proc_entry = create_proc_entry(procEntryName, 0644, NULL );
	proc_entry = proc_create(procEntryName, 0, NULL, &hello_proc_fops);
	if (proc_entry == NULL) 
	{
		ret = -ENOMEM;
		printk(KERN_INFO "%s: Couldn't create proc entry\n",procEntryName);
        } 
	else 
	{

		//proc_entry->write_proc = pid_write;
		printk(KERN_INFO "%s: proc_entry created\n",procEntryName);
	
	}	  
	

    	printk(KERN_INFO "Hello world!\n");
    	return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit hello_cleanup(void)
{
	if (proc_entry)
	{
		remove_proc_entry(procEntryName, NULL);
	}
	printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
