#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/syscall.h>
#include <sys/reboot.h>

#define REBOOT_CMD	"update"

int main(void)
{
	printf("Reboot for updating...\n");
	int ret;
	sync();
	sleep(3);
	ret = syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, REBOOT_CMD);
	return ret;
}
