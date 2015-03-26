#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/media.h>
#include <linux/v4l2-subdev.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <ion.h>
#include <linux/ion.h>
#include <linux/nxp_ion.h>
#include <linux/videodev2_nxp_media.h>

#include <nxp-v4l2.h>

#include "common.cpp"

#define COLOR_RED       0xffff0000
#define COLOR_GREEN     0xff00ff00
#define COLOR_BLUE      0xff0000ff
#define COLOR_WHITE     0xffffffff
static void fill_buffer(unsigned int *buffer, unsigned int color, int width, int height)
{
    int size = width * height;
    unsigned int *pbuffer = buffer;
    for (int i = 0; i < size; i++) {
        *pbuffer = color;
        pbuffer++;
    }
}

int main(int argc, char *argv[])
{

    int ion_fd = ion_open();
    int width = 720;
    int height = 480;
    int format = V4L2_PIX_FMT_RGB32;

    if (ion_fd < 0) {
        fprintf(stderr, "can't open ion!!!\n");
        return -EINVAL;
    }

    struct V4l2UsageScheme s;
    memset(&s, 0, sizeof(s));

    s.useMlc1Rgb = true;
    s.useTvout = true;

    CHECK_COMMAND(v4l2_init(&s));
    CHECK_COMMAND(v4l2_link(nxp_v4l2_mlc1, nxp_v4l2_tvout));

    CHECK_COMMAND(v4l2_set_format(nxp_v4l2_mlc1_rgb, width, height, V4L2_PIX_FMT_RGB32));
    CHECK_COMMAND(v4l2_set_crop(nxp_v4l2_mlc1_rgb, 0, 0, width, height));
    CHECK_COMMAND(v4l2_reqbuf(nxp_v4l2_mlc1_rgb, MAX_BUFFER_COUNT));

    struct nxp_vid_buffer rgb_bufs[MAX_BUFFER_COUNT];
    CHECK_COMMAND(alloc_buffers(ion_fd, MAX_BUFFER_COUNT, rgb_bufs, width, height, V4L2_PIX_FMT_RGB32));

    fill_buffer((uint32_t *)rgb_bufs[0].virt[0], COLOR_RED, width, height);
    fill_buffer((uint32_t *)rgb_bufs[1].virt[0], COLOR_GREEN, width, height);
    fill_buffer((uint32_t *)rgb_bufs[2].virt[0], COLOR_BLUE, width, height);
    fill_buffer((uint32_t *)rgb_bufs[3].virt[0], COLOR_WHITE, width, height);

    int out_index = 0;
    int out_dq_index = 0;
    int out_q_count = 0;
    bool started_out = false;
    int j;
    unsigned short *prgb_data;
    struct nxp_vid_buffer *rgb_buf;
    int count = 100;
    while (count--) {
        rgb_buf = &rgb_bufs[out_index];
        CHECK_COMMAND(v4l2_qbuf(nxp_v4l2_mlc1_rgb, 1, out_index, rgb_buf, -1, NULL));

        out_q_count++;
        out_index++;
        out_index %= MAX_BUFFER_COUNT;

        if (!started_out) {
            CHECK_COMMAND(v4l2_streamon(nxp_v4l2_mlc1_rgb));
            started_out = true;
        }

        printf("display %d\n", out_index);
        usleep(1000000);

        if (out_q_count >= 2) {
            CHECK_COMMAND(v4l2_dqbuf(nxp_v4l2_mlc1_rgb, 1, &out_dq_index, NULL));
            out_q_count--;
        }
    }

    CHECK_COMMAND(v4l2_streamoff(nxp_v4l2_mlc1_rgb));

    v4l2_exit();
    close(ion_fd);

    return 0;
}
