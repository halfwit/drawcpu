/*
 * Plan 9 versions of system-specific functions
 *	By convention, exported routines herein have names beginning with an
 *	upper case letter.
 */

#include "rc.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
#include "getflags.h"

#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>

static void execfinit(void);

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

// TODO: PREFIX "/lib/rcmain"
char Rcmain[]="./rc/rcmain.unix";
char Fdprefix[]="/dev/fd/";

char *Signame[NSIG];

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

void
Vinit(void)
{
	int fd;
	DIR *dir;
	struct dirent *ent;

	dir = opendir("/env");
	if(dir == nil){
		pfmt(err, "%s: can't open: %s\n", argv0, Errstr());
		return;
	}
	for(;;){
		ent = readdir(dir);
		if (ent == nil)
			break;
		if(ent->d_namlen<=0 || strncmp(ent->d_name, "fn#", 3)==0)
			continue;
		if((fd = Open(Env(ent->d_name, 0), 0))>=0){
			io *f = openiofd(fd);
			word *w = 0, **wp = &w;
			char *s;
			while((s = rstr(f, "")) != 0){
				*wp = Newword(s, (word*)0);
				wp = &(*wp)->next;
			}
			closeio(f);
			setvar(ent->d_name, w);
			vlook(ent->d_name)->changed = 0;
		}
		free(ent);
	}
	closedir(dir);
}

char*
Errstr(void)
{
	return strerror(errno);
}

/* Can we do this inside the kernel? */
int
Waitfor(int pid)
{
	thread *p;
	char num[12];
	int wpid, status;

	if(pid >= 0 && !havewaitpid(pid))
		return 0;
	while((wpid = wait(&status))!=-1){
		delwaitpid(wpid);
		inttoascii(num, WIFSIGNALED(status)?WTERMSIG(status)+1000:WEXITSTATUS(status));
		if(wpid==pid){
			setstatus(num);
			return 0;
		}
		for(p = runq->ret;p;p = p->ret)
			if(p->pid==wpid){
				p->pid=-1;
				p->status = estrdup(num);
				break;
			}
	}
	if(errno==EINTR) return -1;
	return 0;
}

static void
addenv(var *v)
{
	word *w;
	int fd;
	io *f;

	if(v->changed){
		v->changed = 0;
		if((fd = Creat(Env(v->name, 0)))<0)
			pfmt(err, "%s: can't open: %s\n", argv0, Errstr());
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
		if((fd = Creat(Env(v->name, 1)))<0)
			pfmt(err, "%s: can't open: %s\n", argv0, Errstr());
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

void
Exec(char **argv)
{
	// TODO: execve after loading env to string
	execv(argv[0], argv+1);
}

int
Fork(void)
{
	Updenv();
	return fork();
}

void*
Opendir(char *name)
{
	return opendir(name);
}

char*
Readdir(void *arg, int onlydirs)
{
	DIR *rd = arg;
	struct dirent *ent = readdir(rd);
	if(ent == NULL)
		return 0;
	return ent->d_name;
}

void
Closedir(void *arg)
{
	DIR *rd = arg;
	closedir(rd);
}

static void
sighandler(int sig)
{
	trap[sig]++;
	ntrap++;
}

// TODO: Use kernel sighandling 
void
Trapinit(void)
{
	int i;

	Signame[0] = "sigexit";

#ifdef SIGINT
	Signame[SIGINT] = "sigint";
#endif
#ifdef SIGTERM
	Signame[SIGTERM] = "sigterm";
#endif
#ifdef SIGHUP
	Signame[SIGHUP] = "sighup";
#endif
#ifdef SIGQUIT
	Signame[SIGQUIT] = "sigquit";
#endif
#ifdef SIGPIPE
	Signame[SIGPIPE] = "sigpipe";
#endif
#ifdef SIGUSR1
	Signame[SIGUSR1] = "sigusr1";
#endif
#ifdef SIGUSR2
	Signame[SIGUSR2] = "sigusr2";
#endif
#ifdef SIGBUS
	Signame[SIGBUS] = "sigbus";
#endif
#ifdef SIGWINCH
	Signame[SIGWINCH] = "sigwinch";
#endif

	for(i=1; i<NSIG; i++) if(Signame[i]){
#ifdef SA_RESTART
		struct sigaction a;

		sigaction(i, NULL, &a);
		a.sa_flags &= ~SA_RESTART;
		a.sa_handler = sighandler;
		sigaction(i, &a, NULL);
#else
		signal(i, sighandler);
#endif
	}
}

long
Write(int fd, void *buf, long cnt)
{
	return write(fd, buf, cnt);
}

long
Read(int fd, void *buf, long cnt)
{
	return read(fd, buf, cnt);
}

long
Seek(int fd, long cnt, long whence)
{
	return seek(fd, cnt, whence);
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

int
Open(char *file, int mode)
{
	static int tab[] = {OREAD,OWRITE,ORDWR,OREAD|ORCLOSE};
	return open(file, tab[mode&3]);
}

void
Close(int fd)
{
	close(fd);
}

int
Creat(char *file)
{
	return create(file, OWRITE, 0666L);
}

int
Dup(int a, int b)
{
	return dup(a, b);
}

int
Dup1(int a)
{
	return dup(a, -1);
}

void
Exit(void)
{
	Updenv();
	exits(truestatus()?"":getstatus());
}

void
Noerror(void)
{
	errno = 0;
}

int
Isatty(int fd)
{
	return isatty(fd);
}

void
Abort(void)
{
	abort();
}

int
Chdir(char *dir)
{
	return chdir(dir);
}

void
Prompt(char *s)
{
	pstr(err, s);
	flushio(err);
}
