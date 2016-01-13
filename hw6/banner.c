/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/gpio.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

/*  
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;
static int bytes_read = 0;
static int char_num = 79;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};



static unsigned int GPIO = 17;		//gpio port number im using
static unsigned int counter = 0;
static char msg_holder;
static unsigned int spacecounter = 1;
static unsigned int readcounter = 0;
/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
        Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	/* 
	 * Unregister the device 
	 */
  //	int ret = unregister_chrdev(Major, DEVICE_NAME);
	 printk(KERN_INFO "BANNER: closed module\n");
	unregister_chrdev(Major, DEVICE_NAME);

  //	if (ret < 0)
  //		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
}

void led_morsecode(char* morse)
{
	size_t length;
	int y = 0;
	length = strlen(morse);
	
	if(spacecounter==1)
		{
	while(y<length)
		{
		
		gpio_request(GPIO, "sysfs"); 		// set gpio to 17 for request
    	gpio_direction_output(GPIO, 1);   // Set the gpio to be in output mode and on	       
		gpio_export(GPIO, true);
			if(morse[y] == '.')
			{
			  mdelay(500);
			} 
		else if(morse[y] == '-')
		{
			mdelay(1000);
		} 
		else 
		{
			mdelay(1000);
		}
		
		

  	
		
		gpio_set_value(GPIO,0);
		mdelay(1000);
		y++;
	}
		}
		else 
		{						
			mdelay(2500);
			spacecounter = 1;
		}
}
/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	// static int counter = 0;
	 if (!gpio_is_valid(GPIO)){
      printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
      return -ENODEV;
   }
	

   //mdelay(1000);

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	//sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
	bytes_read = 0;

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{


	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int read= bytes_read;
	int x = 0;
	printk(KERN_INFO "BANNER: read from file\n");

	if (msg[0]!= '*')
	{
			printk(KERN_INFO "LED_DRIVER: Blinking the led\n");

	for( x= 0; x< char_num; x++){
			switch (msg[x]){
				case 'a':
				case 'A':
					led_morsecode(".-");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'b':
				case 'B':
					led_morsecode("-...");
					spacecounter--;
					led_morsecode(" ");

				break;
				case 'c':
				case 'C':
					led_morsecode("-.-.");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'd':
				case 'D':
					led_morsecode("-..");
					spacecounter--;
					led_morsecode(" ");
					break;
				case 'e':
				case 'E':
					led_morsecode(".");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'f':
				case 'F':
					led_morsecode("..-.");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'g':
				case 'G':
					led_morsecode("--.");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'h':
				case 'H':
					led_morsecode("....");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'i':
				case 'I':
					led_morsecode("..");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'j':
				case 'J':
					led_morsecode(".---");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'k':
				case 'K':
					led_morsecode("-.-");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'l':
				case 'L':
					led_morsecode(".-..");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'm':
				case 'M':
					led_morsecode("--");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'n':
				case 'N':
					led_morsecode("-.");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'o':
				case 'O':
					led_morsecode("---");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'p':
				case 'P':
					led_morsecode(".--.");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'q':
				case 'Q':
					led_morsecode("--.-");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'r':
				case 'R':
					led_morsecode(".-.");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 's':
				case 'S':
					led_morsecode("...");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 't':
				case 'T':
					led_morsecode("-");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'u':
				case 'U':
					led_morsecode("..-");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'v':
				case 'V':
					led_morsecode("...-");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'w':
				case 'W':
					led_morsecode(".--");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'x':
				case 'X':
					led_morsecode(".--");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'y':
				case 'Y':
					led_morsecode("-.--");
					spacecounter--;
					led_morsecode(" ");
				break;
				case 'z':
				case 'Z':
					led_morsecode("--..");
					spacecounter--;
					led_morsecode(" ");
				break;
				case ' ':
					led_morsecode(" ");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '1':
					led_morsecode(".----");
					spacecounter--;
					led_morsecode(" ");
					break;
				case '2':
					led_morsecode("..---");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '3':
					led_morsecode("...--");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '4':
					led_morsecode("....-");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '5':
					led_morsecode(".....");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '6':
					led_morsecode("-....");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '7':
					led_morsecode("--...");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '8':
					led_morsecode("---..");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '9':
					led_morsecode("----.");
					spacecounter--;
					led_morsecode(" ");
				break;
				case '0':
					led_morsecode("-----");
					spacecounter--;
					led_morsecode(" ");
				break;
			}
		}
	}

		counter++;
		mdelay(1000);
		if(counter == 1)
		{
			msg_holder = msg[0];
			msg[0]='*';
		}
		else
		{
			msg[0]=msg_holder;
			counter = 0;
		}
		
	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (bytes_read >=char_num )
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && (bytes_read < char_num)) {
	
		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(msg_Ptr[bytes_read], buffer+bytes_read);

		length--;
		bytes_read++;
	}
	

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read-read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char_num = 0;

	if(char_num >= BUF_LEN)
		return 0;
	printk(KERN_INFO "BANNER: wrote to file\n");

	while (len && (char_num< BUF_LEN))
	{
		get_user(msg_Ptr[char_num],buff+char_num);
	len--;
	char_num++;
	}

	return char_num;
}

/* The macros below register the init and exit functions with the kernel */
// module_init(init_module);
// module_exit(cleanup_module);
