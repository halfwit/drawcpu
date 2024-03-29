/* sys calls */
#define	bind	sysbind
#define	chdir	syschdir
#define	close	sysclose
#define create	syscreate
#define dup	    sysdup
#define export	sysexport
#define getwd   sysgetwd
#define fd2path sysfd2path
#define fstat	sysfstat
#define fwstat	sysfwstat
#define mount	sysmount
#define	open	sysopen
#define read	sysread
#define remove	sysremove
#define rfork   sysrfork
#define seek	sysseek
#define stat	sysstat
#define wait    syswait
#define	write	syswrite
#define wstat	syswstat
#define unmount	sysunmount
#define pipe	syspipe
#define rendezvous	sysrendezvous
#define getpid	sysgetpid
#define time systime
#define nsec sysnsec
#define pread syspread
#define pwrite syspwrite
#undef sleep
#define	sleep	osmsleep
#define iounit	sysiounit
#define getenv	sysgetenv

extern	int	bind(char*, char*, int);
extern	int	chdir(char*);
extern	int	close(int);
extern	int	create(char*, int, ulong);
extern	int	dup(int, int);
extern  int	export(int);
extern  int fd2path(int, char*, int);
extern	int	fstat(int, uchar*, int);
extern	int	fwstat(int, uchar*, int);
extern  char* getwd(char *, int);
extern	int	mount(int, int, char*, int, char*);
extern	int	unmount(char*, char*);
extern	int	open(char*, int);
extern	int	pipe(int*);
extern	long	read(int, void*, long);
extern	long	readn(int, void*, long);
extern	int	remove(char*);
extern  int rfork(int);
extern	vlong	seek(int, vlong, int);
extern	int	stat(char*, uchar*, int);
extern  long time(long*);
extern	long	write(int, void*, long);
extern	int	wstat(char*, uchar*, int);
extern	void	werrstr(char* ,...);
extern  Waitmsg* wait(void);
extern	Dir	*dirstat(char*);
extern	Dir	*dirfstat(int);
extern	int	dirwstat(char*, Dir*);
extern	int	dirfwstat(int, Dir*);
extern	long	dirread(int, Dir**);
extern  long    dirreadall(int, Dir**);
extern	ulong	iounit(int);

extern	int	lfdfd(int);

/*
 *  network dialing and authentication
 */
#define NETPATHLEN 40
extern	int	accept(int, char*);
extern	int	announce(char*, char*);
extern	int	dial(char*, char*, char*, int*);
extern	int	hangup(int);
extern	int	listen(char*, char*);
extern	char *netmkaddr(char*, char*, char*);
extern	int	reject(int, char*, char*);

extern 	char*	argv0;

extern	ulong	truerand(void);
extern	int	pushssl(int, char*, char*, char*, int*);
extern	long	pread(int, void*, long, vlong);
extern	long	pwrite(int, void*, long, vlong);
extern	void*	rendezvous(void*, void*);
extern	int	kproc(char*, void(*)(void*), void*);
extern	int	getpid(void);
extern	void	panic(char*, ...);
extern	void	sleep(int);
extern	void	osyield(void);
extern	void	setmalloctag(void*, uintptr);
extern	void	setrealloctag(void*, uintptr);
extern	int	errstr(char*, uint);
extern  char *estrdup(char*);
extern	int	rerrstr(char*, uint);
extern  int notify(void (*)(void*, char*));
extern  int noted(int);
extern	int	encrypt(void*, void*, int);
extern	int	decrypt(void*, void*, int);
extern	void	qlock(QLock*);
extern	void	qunlock(QLock*);
extern	long	time(long*);
extern	vlong	nsec(void);
extern	void	lock(Lock*);
extern	void	unlock(Lock*);
extern	int	iprint(char*, ...);
extern	int	atexit(void (*)(void));
extern	void	exits(char*);
extern	char*	getenv(char*);

extern  int await(char*, int);
extern  int awaitnohang(char*, int);
extern  int awaitfor(int, char*, int);

#define IOUNIT			32768	/* default buffer size for 9p io */
