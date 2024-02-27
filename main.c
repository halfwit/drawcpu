#include "u.h"
#include "lib.h"
#include "kern/dat.h"
#include "kern/fns.h"
#include "user.h"
#include "drawcpu.h"
#include "ip.h"
#include "authsrv.h"

char *argv0;
char *authserver = "";
char *dbgfile = "./debug.log";

void
sizebug(void)
{
	/*
	 * Needed by various parts of the code.
	 * This is a huge bug.
	 */
	assert(sizeof(char)==1);
	assert(sizeof(short)==2);
	assert(sizeof(ushort)==2);
	assert(sizeof(int)==4);
	assert(sizeof(uint)==4);
	assert(sizeof(long)==4);
	assert(sizeof(ulong)==4);
	assert(sizeof(vlong)==8);
	assert(sizeof(uvlong)==8);
}

// TODO: remove libgui, or at least revamp as cpubody goes away
void cpubody(void) {}

char*
estrdup(char *s)
{
	s = strdup(s);
	if(s == nil)
		sysfatal("out of memory");
    return s;
}

int
main(int argc, char **argv)
{
	extern ulong kerndate;
	kerndate = seconds();
	eve = getuser();
	if(eve == nil)
		eve = "drawcpu";

	sizebug();
	osinit();
	procinit0();
	printinit();

	chandevreset();
	chandevinit();
	quotefmtinstall();

	if(bind("#c", "/dev", MBEFORE) < 0)
		panic("bind #c: %r");
	if(bind("#e", "/env", MREPL|MCREATE) < 0)
		panic("bind #e: %r");
	if(bind("#U", "/root", MREPL) < 0)
		panic("bind #U: %r");
    if(bind("/root", "/", MAFTER) < 0)
		panic("bind /root: %r");

	// We get service=cpu, change to =unix
	char *cmd[] = {
		"drawcpu",
		"-c"
		". <{n=`{read} && ! ~ $#n 0 && read -c $n} >[2=1]"	
	};

	runcommand(3, cmd);
	_exit(0);
}
