#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/proc_fs.h>   // for proc interconnection
#include <linux/string.h>    // for string functions using
#include <linux/slab.h>	     // kzalloc and co
#include <linux/uaccess.h>   // copy_from_user
#include <linux/seq_file.h>
#include <linux/rwsem.h>

static struct proc_dir_entry *proc_entry;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lakshmanan");
MODULE_DESCRIPTION("Process delaying Shedule");

char * procEntryName="pds";
static struct semaphore r;
static struct semaphore w;

int reader_number=0;
int writer_number=0;
static struct semaphore rmutex, wmutex, readTry, resource; //(initial value = 1)


void add_delete_process(int processID,int millisecondNumbers)
{
	down(&wmutex);//reserve entry section for writers - avoids race conditions
	writer_number++;//report yourself as a writer entering
	if (writer_number == 1)//checks if you're first writer
	{
		down(&readTry);//if you're first, then you must lock the readers out. Prevent them from trying to enter CS
	}
	up(&wmutex);//release entry section
	down(&resource);//reserve the resource for yourself - prevents other writers from simultaneously editing the shared resource
	// writers' actions
	// ...
	up(&resource);//release file
	down(&wmutex);//reserve exit section
	writer_number--;//indicate you're leaving
	if (writer_number == 0)//checks if you're the last writer
	{
		up(&readTry);//if you're last writer, you must unlock the readers. Allows them to try enter CS for reading
	}
	up(&wmutex);//release exit section
}

int processInList(int processID)
{
	down(&readTry);//Indicate a reader is trying to enter
	down(&rmutex);//lock entry section to avoid race condition with other readers
	reader_number++;//report yourself as a reader
	if (reader_number == 1)//checks if you are first reader
	{
		down(&resource);//if you are first reader, lock  the resource
	}
	up(&rmutex);//release entry section for other readers
	up(&readTry);//indicate you are done trying to access the resource
	// Readers' actions
	// ...
	down(&rmutex);//reserve exit section - avoids race condition with readers
	reader_number--;//indicate you're leaving
	if (reader_number == 0)//checks if you are last reader leaving
	{
		up(&resource);//if last, you must release the locked resource
	}	
	up(&rmutex);//release exit section for other readers

}

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
	sscanf (data, "%c %d", &command, &processIdToOperate); //считываем данные
	
	
	switch (command)
	{

		case 'p':
			printk(KERN_INFO "Arrived pause command			---------->\n");
			printk(KERN_INFO "Process: %d				---------->\n",processIdToOperate);
			
			/* Перепрочитываем целиком все аргументы. Вообще, за такие вещи в драйвере автору надо отрывать руки и скармливать 
				их каким-то очень страшным животным, но пока что я не придумал более оптимальный с одной стороны и наглядный 					способ получения третьего аргумента.
			*/
			sscanf (data, "%c %d %d", &command, &processIdToOperate, &timeNotToShedule); //считываем данные

			printk(KERN_INFO "Number of milliseconds to stop: %d	---------->\n",timeNotToShedule);
			add_delete_process(processIdToOperate,timeNotToShedule);
		break;

		case 'r':
			printk(KERN_INFO "Arrived resume command		---------->\n");
			printk(KERN_INFO "Process: %d				---------->\n",processIdToOperate);
			add_delete_process(processIdToOperate,-1);
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

	sema_init(&rmutex,1);
	sema_init(&wmutex,1);	
	sema_init(&readTry,1);
	sema_init(&resource,1);	

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
