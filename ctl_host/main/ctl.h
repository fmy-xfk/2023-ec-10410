#ifndef __CTL_H__
#define __CTL_H__

#include "string.h"

#define MAX_CTL_STREAM_BUF_SZ 128

typedef int (*read_func)(unsigned int id, unsigned char *data, unsigned int dataLen);
typedef int (*proc_func)(unsigned char op,unsigned int data);

typedef struct{
    unsigned char buf[MAX_CTL_STREAM_BUF_SZ];
    unsigned int hnd;int rem;
    read_func fr;proc_func fp;
}ctl_stream;

void ctl_stream_init(ctl_stream *s,read_func f0,proc_func f1,unsigned int handle);
unsigned int ctl_stream_act(ctl_stream *s);

#endif