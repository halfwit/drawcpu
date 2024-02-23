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
    //n = read(fd, buf, sizeof buf);
    //if (n > 0) {
    //return rcmain(buf, fd);
    //}
    close(fd);
    // write(dbg, buf, n);
    // sed out any line that says 'service=cpu' and put 'service=unix'
    return -1;
}
