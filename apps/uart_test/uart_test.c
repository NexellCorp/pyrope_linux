
#include <stdio.h>
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for open/close .. */
#include <fcntl.h> /* for O_RDONLY */
#include <sys/ioctl.h> /* for ioctl */
#include <sys/mman.h>
#include <termios.h>


#define FBNODE 	"/dev/fb0"


int main(int argc, char **argv)
{
    struct termios ios;
	int fd, ret, opt;
    unsigned char uart_rx_buf[16];


    fd = open("/dev/ttyGS0", O_RDWR|O_NOCTTY);
    if(fd < 0) {
        printf("uart_init", "open fail - errno = %d\r\n");
        return ;
    }

    memset(&ios, 0, sizeof(ios));

    ios.c_cflag = B4000000;// | CS8 | CREAD | CLOCAL | CRTSCTS;
    ios.c_ospeed = 4000000;
    ios.c_ispeed = 4000000;
    ios.c_iflag = IGNPAR;
    ios.c_oflag = 0;
//  ios.c_cc[VTIME] = 5;
//  ios.c_cc[VMIN] = 32;
    ios.c_cc[VMIN] = 1;

    tcsetattr(fd, TCSAFLUSH, &ios);

	while(1) {
	    ret = read(fd, uart_rx_buf, 1);
		printf("## %d, %c \n", ios.c_ospeed, uart_rx_buf[0]);

		if( 'q' == uart_rx_buf[0] ) {
			break;
		}

        ret = write(fd, uart_rx_buf, 1);
	}

    close(fd);


	return 0;
}
