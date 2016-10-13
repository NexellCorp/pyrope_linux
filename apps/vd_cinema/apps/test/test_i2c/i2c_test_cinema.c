#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h> 			/* error */

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#ifndef u32
typedef unsigned int u32;
#endif	/* u32 */
#ifndef u16
typedef unsigned short u16;
#endif	/* u16 */
#ifndef u8
typedef unsigned char u8;
#endif	/* u8  */

#define ARRAY_SIZE(a) (int)(sizeof(a) / sizeof((a)[0]))

static int is_valid_fd(int fd)
{
    int err = fcntl(fd, F_GETFL) != -1 || errno != EBADF;

    if (!err)
		printf("Invalid fd.%d ...\n", fd);

    return err;
}

static int i2c_open(const char *device)
{
	int fd;

  	fd = open(device, O_RDWR, 644);	/* O_RDWR, O_RDONLY */
  	if (0 > fd) {
    	printf("%s: fail, %s open, %s\n",
    		__func__, device, strerror(errno));
    	return -1;
  	}

	return fd;
}

static int i2c_close(int fd)
{
	if (is_valid_fd(fd))
		close(fd);
	return 0;
}

/*
 * I2C WRITE
 * slave /A buf[0] /A .. buf[n] /NA [S]
 */
#if 0
#define	dump_i2c_data(b, s)	do {	\
	int i = 0;	\
	for (i; s > i; i++)	\
		printf("[%3d]:0x%02x\n", i, b[i]);	\
	} while (0)
#else
#define	dump_i2c_data(b, s)
#endif

static int i2c_raw_write(int fd, int slave, u8 *buf, int size)
{
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[1];
	int err;

	dump_i2c_data(buf, size);

	err = is_valid_fd(fd);
	if (!err)
		return err;

	rdwr.msgs = msgs;
 	rdwr.nmsgs = 1;

 	rdwr.msgs[0].addr  = (slave>>1);
 	rdwr.msgs[0].flags = !I2C_M_RD;
 	rdwr.msgs[0].len   = (size);
 	rdwr.msgs[0].buf   = (__u8 *)buf;

	err = ioctl(fd, I2C_RDWR, &rdwr);
	if (0 > err)
		return err;

	return 0;
}

/*
 * I2C WRITE
 * slave /A buf[0] /A .. buf[n] /NA [S]
 */
static int i2c_raw_read(int fd, int slave, u8 *buf, int size)
{
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[1];
	int err;

	err = is_valid_fd(fd);
	if (!err)
		return err;

	rdwr.msgs = msgs;
 	rdwr.nmsgs = 1;

 	rdwr.msgs[0].addr  = (slave>>1);
 	rdwr.msgs[0].flags = I2C_M_RD;
 	rdwr.msgs[0].len   = size;
 	rdwr.msgs[0].buf   = (__u8 *)buf;

	err = ioctl(fd, I2C_RDWR, &rdwr);
	if (0 > err)
		return err;

	return 0;
}

/*
 * for nostop mode
 */
static int i2c_raw_write_read(int fd, int slave,
			u8 *write_buf, int write_size,
			u8 *read_buf, int read_size)
{
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[2];
	int err;

	err = is_valid_fd(fd);
	if (!err)
		return err;

	rdwr.msgs = msgs;
 	rdwr.nmsgs = 2;

 	rdwr.msgs[0].addr  = (slave>>1);
 	rdwr.msgs[0].flags = !I2C_M_RD;
 	rdwr.msgs[0].len   = write_size;
 	rdwr.msgs[0].buf   = (__u8 *)write_buf;

 	rdwr.msgs[1].addr  = (slave>>1);
 	rdwr.msgs[1].flags = I2C_M_RD;
 	rdwr.msgs[1].len   = read_size;
 	rdwr.msgs[1].buf   = (__u8 *)read_buf;

	err = ioctl(fd, I2C_RDWR, &rdwr);
	if (0 > err)
		return err;

	return 0;
}

static int i2c_check_valid(int fd, u8 from, u8 to)
{
	int i = 0, found = 0;
	int err;

	err = is_valid_fd(fd);
	if (!err)
		return err;

	printf("Valid chip addresses [0x%x~0x%x]:", from, to);

	for (i = from; to > i; i += 2) {
		int err = i2c_raw_write(fd, i, NULL, 0);

		if (0 == err) {
			printf(" 0x%02X", i);
			found++;
		}
	}
	printf ("\nFind %d EA\n", found);
	return found;
}

static int i2c_cinema_burst_mode(int fd, u8 slave, u16 addr,
			int burst_bit, bool burst)
{
	u8  buf[4];
	u16 data;
	int err;

	printf("Burst %s  slave:0x%02x [0x%x] bit:%d\n",
			burst ? "ON " : "OFF", slave, addr, burst_bit);

	data = (burst ? 1 : 0) << burst_bit;

	buf[0] = (addr >> 8) & 0xFF;
	buf[1] = (addr >> 0) & 0xFF;
	buf[2] = (data >> 8) & 0xFF;
	buf[3] = (data >> 0) & 0xFF;

	err = i2c_raw_write(fd, slave, buf, ARRAY_SIZE(buf));
	if (0 > err)
		printf("%s: fail, write %d size err:%s\n",
			__func__, ARRAY_SIZE(buf), strerror(errno));

	return err;
}

void print_usage(void)
{
    printf( "usage: options\n"
            "-p	port i2c-n, default i2c-0 	\n"
            "-s	slave id (8bit hex)			\n"
            "-a	address (hex)				\n"
            "-w	write (8bit hex) : 0xN,0xN,...\n"
            "-r	read size: size \n"
            "-m multile write/read buffer : <w>,<size>,<8bit hex> or  <r>,<size>\n"
            "-c	loop count			 		\n"
            "-d	detect i2c.n's devices (0x0 ~ 0xff)	\n"
            "-b	burst bit i2c transfer for vd cinema\n"
    		"    burst: \n"
    		"    Burst on  = W: id/0x00/0x01\n"
    		"          Burst W: id/reg[MSB]/reg[LSB]/data/data/data/...\n"
    		"          Burst R: id/reg[MSB]/reg[LSB]/ [s] id/data/data/...\n"
    		"    Burst off = W: id/0x00/0x00\n"
            );
}

#define	MAX_I2C_BUF_SIZE	1024

#define	opt_str_cnv(p, v, t)	{	\
	if (p) {	\
		v = strtol(p, NULL, t), p = strchr(p, ',');	\
		if (!p)	\
			break;	\
		p++;	\
	} }

int main(int argc, char **argv)
{
 	struct i2c_rdwr_ioctl_data rw_arg;
 	struct i2c_msg ctl_msgs[2];

  	char device[20];
  	int fd = 0, opt;
	int err;

  	int op_port  = 0, op_slave = 0;
	u32 op_addr = 0;
	bool op_write = false, op_read = true;
	bool op_detect = false;
	bool op_burst = false;
	int op_burst_bit = 0;
	long long op_loop = 1;

  	u8 buf[MAX_I2C_BUF_SIZE] = { 0, };
  	int buf_index = 0;
	u8 *ptr = buf;

	int offs_address = 2;
  	int wr_size = 0, rd_size = 1;
  	int i = 0, n = 0;
	u16 burst_addr = 0;

    while (-1 != (opt = getopt(argc, argv, "hp:a:s:r:w:m:c:db:"))) {
		switch(opt) {
        case 'p':
        	op_port = strtol(optarg, NULL, 10);
        	break;

        case 's':
        	sscanf(optarg, "%x", &op_slave);
        	break;

        case 'a':
        	sscanf(optarg, "%x", &op_addr);
        	break;

        case 'r':
        	{
	       		rd_size = strtol(optarg, NULL, 10);
        		op_read = true;

        		if (rd_size > MAX_I2C_BUF_SIZE) {
					printf("%s: fail, read size %d over max %d\n",
						__func__, rd_size, MAX_I2C_BUF_SIZE);
					exit(0);
				}
				break;
			}

        case 'w':
        	{
       			char *c = optarg;

       			for (i = 0; ; i++)
       				opt_str_cnv(c, buf[i], 16);

       			wr_size = i + 1;
      			op_write = true;

				if (wr_size > MAX_I2C_BUF_SIZE) {
					printf("%s: fail, write size %d over max %d\n",
						__func__, wr_size, MAX_I2C_BUF_SIZE);
					exit(0);
				}
       			break;
			}

        case 'm':
        	{
       			int type, size = 0;
				char *c = optarg;
				bool w = false;
        		u8 data = 0x0;

				do {
					char *h = c;

					opt_str_cnv(c, type, 10);	/* dummy for next */
					if (*h == 'w') {
						w = true;
    	   				opt_str_cnv(c, size, 10);
						opt_str_cnv(c, data, 16);
					} else if (*h == 'r') {
		   				opt_str_cnv(c, size, 10);
					} else {
						printf("unknown type %c <w/r>\n", *h);
						exit(0);
					}
				} while (0);

				if (w) {
					wr_size = size;
					op_write = true;
				} else {
					rd_size = size;
					op_read = true;
				}

				ptr = malloc(size);
				if (!ptr) {
					printf("%s: fail, write allocate buffer %d\n",
						__func__, size);
					exit(0);
				}

				memset(ptr, data, size);
       			break;
			}

		case 'c':
			op_loop = strtoll(optarg, NULL, 10);
			break;

		case 'd':
			op_detect = true;
			break;

		case 'b':
			op_burst = true;
			op_burst_bit = strtol(optarg, NULL, 10);
			burst_addr = 0x001;
			break;

        case 'h':
        default:
        	print_usage();  exit(0);
        	break;
		}
	}

  	sprintf(device, "/dev/i2c-%d", op_port);

	if (!is_valid_fd(fd = i2c_open(device)))
		exit(0);

	if (op_detect) {
		u32 from = 0, to = 255;

		if (argc > optind)
			sscanf(argv[optind++], "%x", &from);

		if (argc > optind)
			sscanf(argv[optind++], "%x", &to);

		return i2c_check_valid(fd, (u8)from, (u8)to);
	}


	if (op_burst) {
		err = i2c_cinema_burst_mode(fd, op_slave,
					burst_addr, op_burst_bit, true);
		if (err)
			goto __i2c_end_trans;
	}

	if (op_write) {

		int size = wr_size;
		int offs = offs_address;
		long long cnt = 0;

		memmove(ptr + offs, ptr, size);

		/* set write address */
		for (n = 0; offs > n; n++)
			ptr[n] = (op_addr >> (8 * (offs - (n+1)))) & 0xff;

		printf("[wd] i2c.%d slave:0x%02x [0x%x] size %d\n",
			op_port, op_slave, op_addr, size);

		for (cnt = 0; op_loop > cnt; cnt++) {
			err = i2c_raw_write(fd, op_slave, ptr, size + offs);
			if (err) {
				printf("%s: fail, [wd] i2c.%d slave:0x%02x [0x%x] size %d:%s\n",
					__func__, op_port, op_slave, op_addr, size, strerror(errno));
				goto __i2c_end_trans;
			}

			if (op_loop > 1) {
				printf("\r[count:%lld]", cnt);
				fflush(stdout);
			}
		}

	} else {
		int size = rd_size;
		int offs = offs_address;
		long long cnt = 0;

		/* set write address */
		for (n = 0; offs > n; n++)
			ptr[n] = (op_addr >> (8 * (offs - (n+1)))) & 0xff;

		printf("[rd] i2c.%d slave:0x%02x [0x%x] size %d\n",
			op_port, op_slave, op_addr, size);

		for (cnt = 0; op_loop > cnt; cnt++) {
			/* trans address */
			err = i2c_raw_write(fd, op_slave, ptr, offs);
			if (err) {
				printf("%s: fail, [wr] i2c.%d slave:0x%02x [0x%x] size %d:%s\n",
					__func__, op_port, op_slave, op_addr, offs, strerror(errno));
				goto __i2c_end_trans;
			}

			/* clear address */
			memset(ptr, 0, offs);

			/* read data */
			err = i2c_raw_read(fd, op_slave, ptr, size);
			if (err) {
				printf("%s: fail, [rd] i2c.%d slave:0x%02x [0x%x]  size %d:%s\n",
					__func__, op_port, op_slave, op_addr, size, strerror(errno));
				goto __i2c_end_trans;
			}

			if (op_loop > 1) {
				printf("\r[count:%lld]", cnt);
				fflush(stdout);
			}

			if (cnt == op_loop - 1) {
				printf("[rd] data:\n");
				for (n = 0; rd_size > n; n++) {
					printf("[0x%02x]", ptr[n]);
					if (!((n+1)%8))
						printf("\n");
				}
				printf("\n");
			}
		}
 	}

	if (op_burst)
		i2c_cinema_burst_mode(fd, op_slave, burst_addr, op_burst_bit, false);

__i2c_end_trans:
	i2c_close(fd);

  	return 0;
}
