#include "ctl.h"

void ctl_stream_init(ctl_stream *s,read_func f0,proc_func f1,unsigned int handle){
    memset(s->buf,0,sizeof(s->buf));
    s->fr=f0;s->fp=f1;s->hnd=handle;s->rem=0;
}

unsigned int ctl_stream_act(ctl_stream *s){
    int len;
    if((len=(*(s->fr))(s->hnd, s->buf+s->rem, sizeof(s->buf)-s->rem))<=0){
        return 0;
    }
    len+=s->rem;
    int i=0;
    while(i<len){
        while(s->buf[i]!=0xff && i<len)++i;
        if(i>=len) {s->rem=0;break;}
        if(len-i<5) {s->rem=len-i;memcpy(s->buf+i,s->buf,s->rem);break;}
        ++i;
        unsigned char op=s->buf[i++];
        unsigned int dat=((s->buf[i]-'0')*100+(s->buf[i+1]-'0')*10+(s->buf[i+2]-'0'))*10;
        //printf("[%s]%c %d\n",s->name,op,dat);
        (*(s->fp))(op,dat);
    }
    return 1;
}