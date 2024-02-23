#define	ROUND(s, sz)	(((s)+((sz)-1))&~((sz)-1))

Block*		adjustblock(Block*, int);
Block*		allocb(int);
int		blocklen(Block*);
char*		chanpath(Chan*);
int		cangetc(void*);
int		canlock(Lock*);
int		canputc(void*);
int		canqlock(QLock*);
int		canrlock(RWlock*);
void		chandevinit(void);
void		chandevreset(void);
void		chandevshutdown(void);
void		chanfree(Chan*);
void		chanrec(Mnt*);
void		checkb(Block*, char*);
Chan*		cclone(Chan*);
void		cclose(Chan*);
char*	clipread(void);
int		clipwrite(char*);
void		closeegrp(Egrp*);
void		closefgrp(Fgrp*);
void		closemount(Mount*);
void		closepgrp(Pgrp*);
void		closergrp(Rgrp*);
void		cmderror(Cmdbuf*, char*);
int		cmount(Chan*, Chan*, int, char*);
Block*		concatblock(Block*);
Block*		copyblock(Block*, int);
void		cunmount(Chan*, Chan*);
int		decref(Ref*);
Chan*		devattach(int, char*);
Block*		devbread(Chan*, long, ulong);
long		devbwrite(Chan*, Block*, ulong);
Chan*		devclone(Chan*);
int		devconfig(int, char *, DevConf *);
Chan*		devcreate(Chan*, char*, int, ulong);
void		devdir(Chan*, Qid, char*, vlong, char*, long, Dir*);
long		devdirread(Chan*, char*, long, Dirtab*, int, Devgen*);
Devgen		devgen;
void		devinit(void);
int		devno(int, int);
Chan*		devopen(Chan*, int, Dirtab*, int, Devgen*);
void		devpermcheck(char*, ulong, int);
void		devpower(int);
void		devremove(Chan*);
void		devreset(void);
void		devshutdown(void);
int		devstat(Chan*, uchar*, int, Dirtab*, int, Devgen*);
Walkqid*	devwalk(Chan*, Chan*, char**, int, Dirtab*, int, Devgen*);
int		devwstat(Chan*, uchar*, int);
Dir*		dirchanstat(Chan*);
void		drawcmap(void);
Fgrp*		dupfgrp(Fgrp*);
int		emptystr(char*);
void		envcpy(Egrp*, Egrp*);
int		eqchan(Chan*, Chan*, int);
int		eqqid(Qid, Qid);
void		error(char*);
void		exhausted(char*);
void		exit(int);
Chan*		fdtochan(int, int, int, int);
void		free(void*);
void		freeb(Block*);
void		freeblist(Block*);
uintptr		getmalloctag(void*);
uintptr		getrealloctag(void*);
void		gotolabel(Label*);
char*		getconfenv(void);
long		hostdomainwrite(char*, int);
long		hostownerwrite(char*, int);
Block*		iallocb(int);
void		ilock(Lock*);
void		iunlock(Lock*);
int		incref(Ref*);
int		iprint(char*, ...);
void		isdir(Chan*);
int		iseve(void);
#define	islo()	(0)
int		kbdputc(Queue*, int);
void		kbdkey(Rune, int);
int		kproc(char*, void(*)(void*), void*);
void		ksetenv(char*, char*, int);
void		kstrcpy(char*, char*, int);
void		kstrdup(char**, char*);
long		latin1(Rune*, int);
Chan*		lfdchan(void *);
void		lock(Lock*);
void		lockinit(void);
void		logopen(Log*);
void		logclose(Log*);
char*		logctl(Log*, int, char**, Logflag*);
void		logn(Log*, int, void*, int);
long		logread(Log*, void*, ulong, long);
void		log(Log*, int, char*, ...);
Cmdtab*		lookupcmd(Cmdbuf*, Cmdtab*, int);
void*		mallocz(ulong, int);
#define		malloc kmalloc
void*		malloc(ulong);
void		mkqid(Qid*, vlong, ulong, int);
Chan*		mntauth(Chan*, char*);
void		mntdump(void);
long		mntversion(Chan*, char*, int, int);
Chan*		mntattach(Chan*, Chan*, char*, int);
void		mountfree(Mount*);
void		muxclose(Mnt*);
Chan*		namec(char*, int, int, ulong);
Chan*		newchan(void);
int		newfd(Chan*);
Mhead*		newmhead(Chan*);
Mount*		newmount(Chan*, int, char*);
Path*		newpath(char*);
Pgrp*		newpgrp(void);
Rgrp*		newrgrp(void);
Proc*		newproc(void);
char*		nextelem(char*, char*);
void		nexterror(void);
int		openmode(ulong);
void*		oscmd(char**, int, char*, Chan**);
int		oscmdwait(void*, char*, int);
int		oscmdkill(void*);
void		oscmdfree(void*);
void		oserrstr(void);
void		oserror(void);
void		osexit(void);
Block*		packblock(Block*);
Block*		padblock(Block*, int);
void		panic(char*, ...);
Cmdbuf*		parsecmd(char *a, int n);
void		pathclose(Path*);
void		pexit(char*, int);
void		printinit(void);
int		procindex(ulong);
void		pgrpcpy(Pgrp*, Pgrp*);
void		pgrpnote(ulong, char*, long, int);
Pgrp*		pgrptab(int);
#define		poperror()		up->nerrlab--
int		postnote(Proc*, int, char*, int);
int		pprint(char*, ...);
int		procfdprint(Chan*, int, int, char*, int);
void		procinit0(void);
Proc*		proctab(int);
void		procwired(Proc*, int);
int		pullblock(Block**, int);
Block*		pullupblock(Block*, int);
Block*		pullupqueue(Queue*, int);
void		putmhead(Mhead*);
void		putstr(char*);
void		putstrn(char*, int);
Label*	pwaserror(void);
long		readblist(Block*, uchar*, long, ulong);
int		qaddlist(Queue*, Block*);
Block*		qbread(Queue*, int);
long		qbwrite(Queue*, Block*);
Queue*		qbypass(void (*)(void*, Block*), void*);
int		qcanread(Queue*);
void		qclose(Queue*);
int		qconsume(Queue*, void*, int);
Block*		qcopy(Queue*, int, ulong);
int		qdiscard(Queue*, int);
void		qflush(Queue*);
void		qfree(Queue*);
int		qfull(Queue*);
Block*		qget(Queue*);
void		qhangup(Queue*, char*);
int		qisclosed(Queue*);
void		qinit(void);
int		qiwrite(Queue*, void*, int);
int		qlen(Queue*);
void		qlock(QLock*);
Queue*		qopen(int, int, void (*)(void*), void*);
int		qpass(Queue*, Block*);
int		qpassnolim(Queue*, Block*);
int		qproduce(Queue*, void*, int);
void		qputback(Queue*, Block*);
long		qread(Queue*, void*, int);
Block*		qremove(Queue*);
void		qreopen(Queue*);
void		qsetlimit(Queue*, int);
void		qunlock(QLock*);
int		qwindow(Queue*);
int		qwrite(Queue*, void*, int);
void		qnoblock(Queue*, int);
void		randominit(void);
ulong		randomread(void*, ulong);
int		readnum(ulong, char*, ulong, ulong, int);
int		readstr(ulong, char*, ulong, char*);
int		return0(void*);
void		rlock(RWlock*);
void		runlock(RWlock*);
extern void		(*screenputs)(char*, int);
void*		secalloc(ulong);
void		secfree(void*);
long		seconds(void);
int		setlabel(Label*);
void		setmalloctag(void*, uintptr);
void		setrealloctag(void*, uintptr);
long		showfilewrite(char*, int);
char*		skipslash(char*);
void		sleep(Rendez*, int(*)(void*), void*);
void*		smalloc(ulong);
int		splhi(void);
int		spllo(void);
void		splx(int);
Block*		trimblock(Block*, int, int);
long		unionread(Chan*, void*, long);
void		unlock(Lock*);
#define	validaddr(a, b, c)
void		validname(char*, int);
char*		validnamedup(char*, int);
void		validstat(uchar*, int);
void*		vmemchr(void*, int, int);
Proc*		wakeup(Rendez*);
int		walk(Chan**, char**, int, int, int*);
#define	waserror()	(setjmp(pwaserror()->buf))
void		wlock(RWlock*);
void		wunlock(RWlock*);
void		osyield(void);
void		osmsleep(int);
ulong	ticks(void);
void	osproc(Proc*);
void	osnewproc(Proc*);
void	procsleep(void);
void	procwakeup(Proc*);
void	osinit(void);
void	screeninit(void);
extern	void	terminit(void);
extern	void	setterm(int);
