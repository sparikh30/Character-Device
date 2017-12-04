#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

static struct miscdevice namedpipe;		// created miscdevice structure

int N;
static int char_count = 100;			// number of lines for buffer. Assuming max count is 100 

static int read_index = 0, write_index = 0;	// for read and write function
static int pipe_empty_slots;			// Empty lines in buffer			
static struct semaphore full;			// to keep track of buffer space empty and used
static struct semaphore empty;			
static struct semaphore read_op_mutex;		// to lock the buffer
static struct semaphore write_op_mutex;


module_param(N, int, 0000);			// Number of lines in buffer from command line


struct chardevice				// Allocate buffer like Two Dimension array. buffer[lines][characters]
{
char** pipe;
}char_dev;

static int open(struct inode*, struct file*);						// functions supported by device -declaration 
static ssize_t read(struct file*, char*, size_t, loff_t*);
static ssize_t write(struct file*, const char*, size_t, loff_t*);
static int release(struct inode*, struct file*);


struct file_operations fops = {				// structure of file operations
 .read =  &read,
 .write=  &write,
 .open=   &open,
 .release= &release
};



int char_dev_init (void) 
{
      int ret;
      
      
    namedpipe.name = "namedpipe";					// defining the device name
	namedpipe.minor = MISC_DYNAMIC_MINOR;				
	namedpipe.fops = &fops;				// passing functional operations
	
    printk (" Inside init module\n");
      
	if((ret = misc_register(&namedpipe)))			// register device with misc_register. if negaive means error
	{
		printk(KERN_ERR "Could not register the device\n");
		return ret;
	}
	printk(KERN_INFO "Device Registered! : %s\n",namedpipe.name);

      
		int count = 0;		// allocate memory for two Dimension buffer		

        char_dev.pipe = (char**)kmalloc(N*sizeof(char*), GFP_KERNEL);	// Number of rows

        while(count < N)
        {	
                char_dev.pipe[count] = (char*)kmalloc((char_count+1)*sizeof(char), GFP_KERNEL);		// characters in each rows
                char_dev.pipe[char_count] = '\0';
                ++count;
        }
        printk("New changes added\n");
	
		sema_init(&full, 0);		//initializing semaphores and mutexes
		sema_init(&empty, N);		
        sema_init(&read_op_mutex, 1);
        sema_init(&write_op_mutex, 1);

        //Initializing pipe
        pipe_empty_slots = N;

	    printk("Initialization Done\n");


	     return 0;
}



int open(struct inode *inode, struct file *filp)
 {
        printk(KERN_INFO "Inside open: %d\n",N);
		 return 0;
}

int release(struct inode *inode, struct file *filp)
 {
        printk (KERN_INFO "Inside close \n");
        return 0;
}

ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp) 
{
        down_interruptible(&read_op_mutex);		// lock buffer on read ; at a time only one read
        down_interruptible(&full);			// decrease full; consume the item
	read_index %= N;				// max line of buffer N
	int i;

        for(i = 0; i < count; i++){
                if(N <= pipe_empty_slots){
                        break;
                }
                copy_to_user(&buff[i], &char_dev.pipe[read_index][i], 1);
        }
        ++read_index;					// increas read index			
        ++pipe_empty_slots;				//imcreament empty slot

	//Perform up operation after completing the operation
        up(&empty);				// unlock read
        up(&read_op_mutex);
        return i;
}

ssize_t write(struct file *filp, const char *buff, size_t count, loff_t *offp) 
{
       
	printk("Inside Write\n");
	int i = 0;

        down_interruptible(&write_op_mutex);	// lock on write
		down_interruptible(&empty);	// decrease empty
	    write_index %= N;			// Max write line N	 	

	for(i = 0; i < count; i++)
	{
                if(pipe_empty_slots <= 0)
		{
                        break;
                }
                copy_from_user(&char_dev.pipe[write_index][i], &buff[i], 1);
    	}
        ++write_index;
        --pipe_empty_slots;

        up(&full);
        up(&write_op_mutex);			// unlock write

        return i;
}

void char_dev_cleanup(void) 			// free memory
{
     printk(KERN_INFO " Inside cleanup_module\n");
     
	int i;
        for(i = 0; i < N; i++)
	{
                kfree(char_dev.pipe[i]);
        }
	misc_deregister(&namedpipe);
	printk(KERN_INFO "Device Unregistered!\n");
}

MODULE_LICENSE("GPL");
module_init(char_dev_init);
module_exit(char_dev_cleanup);
