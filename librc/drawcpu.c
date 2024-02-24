/*
 * Unix versions of system-specific functions
 *	By convention, exported routines herein have names beginning with an
 *	upper case letter.
 */
#include <u.h>
#include <libc.h>
#include <rc.h>
#include <errno.h>
#include <string.h>
#include "exec.h"
#include "io.h"
#include "fns.h"
#include "getflags.h"

static void execfinit(void);

// TODO: add bind, mount, etc
builtin Builtin[] = {
	"cd",		execcd,
	"whatis",	execwhatis,
	"eval",		execeval,
	"exec",		execexec,	/* but with popword first */
	"exit",		execexit,
	"shift",	execshift,
	"wait",		execwait,
	".",		execdot,
	"flag",		execflag,
	"finit",	execfinit,
	0
};

char Rcmain[] = "./librc/rcmain.drawcpu";
char Fdprefix[] = "/dev/fd/";
char *Signame[] = {
	"sigexit",	"sighup",	"sigint",	"sigquit",
	"sigalrm",	"sigkill",	"sigfpe",	"sigterm",
	0
};

/*
 * finit could be removed but is kept for
 * backwards compatibility, see: rcmain.plan9
 */
static void
execfinit(void)
{
	char *cmds = estrdup("for(i in '/env/fn#'*){. -bq $i}\n");
	int line = runq->line;
	poplist();
	execcmds(openiocore(cmds, strlen(cmds)), estrdup(srcfile(runq)), runq->local, runq->redir);
	runq->lex->line = line;
	runq->lex->qflag = 1;
}

char*
Env(char *name, int fn)
{
	static char buf[128];

	strcpy(buf, "/env/");
	if(fn) strcat(buf, "fn#");
	return strncat(buf, name, sizeof(buf)-1);
}
/* TODO: Proper env read
void
Vinit(void)
{
	int dir, fd, i, n;
	Dir *ent;

	dir = open(Env("", 0), 0);
	if(dir<0){
		pfmt(err, "%s: can't open: %s\n", argv0, strerror(errno));
		return;
	}
	for(;;){
		ent = 0;
		n = devdirread(dir, ent, sizeof ent);
		if(n <= 0)
			break;
		for(i = 0; i<n; i++){
			if(ent[i].length<=0 || strncmp(ent[i].name, "fn#", 3)==0)
				continue;
			if((fd = open(Env(ent[i].name, 0), 0))>=0){
				io *f = openiofd(fd);
				word *w = 0, **wp = &w;
				char *s;
				while((s = rstr(f, "")) != 0){
					*wp = Newword(s, (word*)0);
					wp = &(*wp)->next;
				}
				closeio(f);
				setvar(ent[i].name, w);
				vlook(ent[i].name)->changed = 0;
			}
		}
		free(ent);
	}
	close(dir);
}
*/

int
Waitfor(int pid)
{
	thread *p;
	Waitmsg *w;

	if(pid >= 0 && !havewaitpid(pid))
		return 0;

	while((w = wait(pid)) != nil){
		delwaitpid(w->pid);
		if(w->pid==pid){
			setstatus(w->msg);
			free(w);
			return 0;
		}
		for(p = runq->ret;p;p = p->ret)
			if(p->pid==w->pid){
				p->pid=-1;
				p->status = estrdup(w->msg);
				break;
			}
		free(w);
	}

	if(strcmp(strerror(errno), "interrupted")==0) return -1;
	return 0;
}

// TODO: /dev/env integration
static void
addenv(var *v)
{
	word *w;
	int fd;
	io *f;

	if(v->changed){
		v->changed = 0;
		if((fd = create(Env(v->name, 0), ORDWR, 0644))<0)
			pfmt(err, "%s: can't open: %s\n", argv0, strerror(errno));
		else{
			f = openiofd(fd);
			for(w = v->val;w;w = w->next){
				pstr(f, w->word);
				pchr(f, '\0');
			}
			flushio(f);
			closeio(f);
		}
	}
	if(v->fnchanged){
		v->fnchanged = 0;
		if((fd = create(Env(v->name, 1), ORDWR, 0644))<0)
			pfmt(err, "%s: can't open: %s\n", argv0, strerror(errno));
		else{
			f = openiofd(fd);
			if(v->fn)
				pfmt(f, "fn %q %s\n", v->name, v->fn[v->pc-1].s);
			flushio(f);
			closeio(f);
		}
	}
}

static void
updenvlocal(var *v)
{
	if(v){
		updenvlocal(v->next);
		addenv(v);
	}
}

void
Updenv(void)
{
	var *v, **h;
	for(h = gvar;h!=&gvar[NVAR];h++)
		for(v=*h;v;v = v->next)
			addenv(v);
	if(runq)
		updenvlocal(runq->local);
	if(err)
		flushio(err);
}

typedef struct rdir rdir;
struct rdir {
	Dir	*dbuf;
	int	i, n;
	int	fd;
};

static int
trimdirs(Dir *d, int nd)
{
	int r, w;

	for(r=w=0; r<nd; r++)
		if(d[r].mode&DMDIR)
			d[w++] = d[r];
	return w;
}

static int interrupted = 0;
static char *syssigname[] = {
	"exit",		/* can't happen */
	"hangup",
	"interrupt",
	"quit",		/* can't happen */
	"alarm",
	"kill",
	"sys: fp: ",
	"term",
	0
};

void
notifyf(void* q, char *s)
{
	int i;
	USED(q);
	for(i = 0;syssigname[i];i++) if(strncmp(s, syssigname[i], strlen(syssigname[i]))==0){
		if(strncmp(s, "sys: ", 5)!=0) interrupted = 1;
		goto Out;
	}
	// TODO: Handle notes
	//noted(NDFLT);
	return;
Out:
	if(strcmp(s, "interrupt")!=0 || trap[i]==0){
		trap[i]++;
		ntrap++;
	}
	// TODO: Handle notes
	//noted(NCONT);
}

int
Executable(char *file)
{
	Dir *statbuf;
	int ret;

	statbuf = dirstat(file);
	if(statbuf == nil)
		return 0;
	ret = ((statbuf->mode&0111)!=0 && (statbuf->mode&DMDIR)==0);
	free(statbuf);
	return ret;
}

void
Noerror(void)
{
	interrupted = 0;
}

int
Isatty(int fd)
{
	isatty(fd);
}

void
Prompt(char *s)
{
	pstr(err, s);
	flushio(err);
}
	