#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>
#include <authsrv.h>
#include <libsec.h>
#include "drawcpu.h"

// TODO: aanserver
static void
sessionexit(void)
{
    char *s = getenv("rstatus");
    if(s != nil)
        exit(*s);
}

int
session(int fd)
{
    int n;
    char buf[1024];
    //while (1) {
        n = read(fd, buf, sizeof buf);
        
    //}
        //return rcmain(buf, fd);
    // sed out any line that says 'service=cpu' and put 'service=unix'
    return -1;
}
