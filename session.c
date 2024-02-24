#include <u.h>
#include <libc.h>
#include <rc.h>

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
    char buf[1024];
    read(fd, buf, sizeof buf);
    char *cmd[] = {
        "rc",
        "-c",
        buf
    };

    runscript(fd, 1, cmd);
    return -1;
}
