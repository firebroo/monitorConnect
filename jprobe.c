/*
 * Here's a sample kernel module showing the use of jprobes to dump
 * the arguments of do_fork().
 *
 * For more information on theory of operation of jprobes, see
 * Documentation/kprobes.txt
 *
 * Build and insert the kernel module as done in the kprobe example.
 * You will see the trace data in /var/log/messages and on the
 * console whenever do_fork() is invoked to create a new process.
 * (Some messages may be suppressed if syslogd is configured to
 * eliminate duplicate messages.)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <net/sock.h>
#include <linux/socket.h>

static void 
ip_to_str(int ip, char * buf)
{
    sprintf(buf, "%d.%d.%d.%d", ip >> 24, (ip & 0x00FF0000) >> 16, (ip & 0x0000FF00) >> 8, (ip & 0x000000FF));
}

/* Proxy routine having the same arguments as actual do_fork() routine */
static long 
do_sys_connect(int fd, struct sockaddr __user * uservaddr, int addrlen)
{
    struct sockaddr_in   *in;
    char                  buf[1024];

    in = (struct sockaddr_in *)uservaddr;
    ip_to_str(htonl((in->sin_addr).s_addr), buf);

	printk(KERN_INFO "jprobe: fd = %d, address = %s, port = %d, addrlen = %d\n", \
            fd, buf, htons(in->sin_port), addrlen);

	jprobe_return();
	return 0;
}

static struct jprobe my_jprobe = {
	.entry			= do_sys_connect,
	.kp = {
		.symbol_name	= "sys_connect",
	},
};

static int __init jprobe_init(void)
{
	int ret;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);
	return 0;
}

static void __exit jprobe_exit(void)
{
	unregister_jprobe(&my_jprobe);
	printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
