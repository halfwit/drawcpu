/*
 * Maybe `simple' is a misnomer.
 */
#include "rc.h"
#include "getflags.h"
#include "exec.h"
#include "io.h"
#include "fns.h"

/*
 * Search through the following code to see if we're just going to exit.
 */
int
exitnext(void){
	int i=ifnot;
	thread *p=runq;
	code *c;
loop:
	c=&p->code[p->pc];
	while(1){
		if(c->f==Xpopredir || c->f==Xunlocal)
			c++;
		else if(c->f==Xsrcline)
			c += 2;
		else if(c->f==Xwastrue){
			c++;
			i=0;
		}
		else if(c->f==Xifnot){
			if(i)
				c += 2;
			else
				c = &p->code[c[1].i];
		}
		else if(c->f==Xreturn){
			p = p->ret;
			if(p==0)
				return 1;
			goto loop;
		}else
			break;
	}
	return c->f==Xexit;
}

void (*builtinfunc(char *name))(void)
{
	extern builtin Builtin[];
	builtin *bp;

	for(bp = Builtin;bp->name;bp++)
		if(strcmp(name, bp->name)==0)
			return bp->fnc;
	return 0;
}

void
Xsimple(void)
{
	void (*f)(void);
	word *a;
	var *v;
	int pid;

	a = runq->argv->words;
	if(a==0){
		Xerror1("empty argument list");
		return;
	}
	if(flag['x'])
		pfmt(err, "%v\n", a); /* wrong, should do redirs */
	v = gvlook(a->word);
	if(v->fn)
		execfunc(v);
	else{
		if(strcmp(a->word, "builtin")==0){
			a = a->next;
			if(a==0){
				Xerror1("builtin: empty argument list");
				return;
			}
			popword();	/* "builtin" */
		}
		f = builtinfunc(a->word);
		if(f){
			(*f)();
			return;
		}
		if(exitnext()){
			/* fork and wait is redundant */
			pushword("exec");
			execexec();
			/* does not return */
		}
		else{
			if((pid = execforkexec()) < 0){
				Xerror2("try again", Errstr());
				return;
			}
			poplist();

			/* interrupts don't get us out */
			while(Waitfor(pid) < 0)
				;
		}
	}
}

static void
doredir(redir *rp)
{
	if(rp){
		doredir(rp->next);
		switch(rp->type){
		case ROPEN:
			if(rp->from!=rp->to){
				Dup(rp->from, rp->to);
				Close(rp->from);
			}
			break;
		case RDUP:
			Dup(rp->from, rp->to);
			break;
		case RCLOSE:
			Close(rp->from);
			break;
		}
	}
}

char*
makercpath(char *dir, char *file)
{
	char *path;
	int m, n = strlen(dir);
	if(n==0) return estrdup(file);
	while (n > 0 && dir[n-1]=='/') n--;
	while (file[0]=='/') file++;
	m = strlen(file);
	path = emalloc(n + m + 2);
	if(n>0) memmove(path, dir, n);
	path[n++]='/';
	memmove(path+n, file, m+1);
	return path;
}

word*
searchpath(char *w, char *v)
{
	static struct word nullpath = { "", 0 };
	word *path;

	if(w[0] && w[0] != '/' && w[0] != '#' &&
	  (w[0] != '.' || (w[1] && w[1] != '/' && (w[1] != '.' || (w[2] && w[2] != '/'))))){
		path = vlook(v)->val;
		if(path)
			return path;
	}
	return &nullpath;
}

static char**
mkargv(word *a)
{
	char **argv = (char **)emalloc((count(a)+2)*sizeof(char *));
	char **argp = argv+1;
	for(;a;a = a->next) *argp++=a->word;
	*argp = 0;
	return argv;
}

int
chars(int fd, int multi, vlong count, char *file)
{
	char buf[8*1024];
	vlong m;
	int n;

	for(m = 0; m < count; m += n){
		n = sizeof(buf);
		if(n > (count - m))
			n = count - m;
		if((n = read(fd, buf, n)) < 0){
			return -1;
		}
		if(n == 0){
			if(m == 0)
				setstatus("eof");
			break;
		}
		write(1, buf, n);
	}
	return 0;
}

int
line(int fd, char *file)
{
	char c;
	int m, n, nalloc;
	char *buf;

	nalloc = 0;
	buf = nil;
	for(m=0; ; ){
		n = read(fd, &c, 1);
		if(n < 0){
			return -1;
		}
		if(n == 0){
			if(m == 0)
				setstatus("eof");
			break;
		}
		if(m == nalloc){
			nalloc += 1024;
			buf = realloc(buf, nalloc);
			if(buf == nil){
				return -1;
			}
		}
		buf[m++] = c;
		if(c == '\n')
			break;
	}
	if(m > 0)
		write(1, buf, m);
	free(buf);
	return m;
}

int
lines(int fd, int multi, vlong count, char *file)
{
	int n;
	do{
		if((n = line(fd, file)) <= 0)
			break;
	}while(multi || --count > 0);
	return n;
}

void
execread(void)
{
//	print("Execread\n");
	int (*proc)(int, int, vlong, char*);
	word *a;
	int fd, multi = 0;
	vlong num = 0;
	popword(); /* "read" */
	proc = lines;
	while(runq->argv->words && runq->argv->words->word[0]=='-'){
		char *f = runq->argv->words->word+1;
		if(*f == '-'){
			popword();
			break;
		}
		proc = lines;
		for(; *f; f++){
			switch(*f){
			case 'c':
				num = atoll(runq->argv->words->next->word);
				proc = chars;
				break;
			case 'n':
				num = atoi(runq->argv->words->next->word);
				break;
			case 'm':
				multi = 1;
				break;
			default:
				goto Usage;
			}
			popword();
		}
	}
	if(count(runq->argv->words)==0){
		(*proc)(0, multi, num, "<stdin>");
	} else {
		a = runq->argv->words->next;
		for(;a;a = a->next){
			fd = open(a->word, OREAD);
			if(fd < 0){
				goto Error;
			}
			if((*proc)(fd, multi, num, a->word) < 0)
				goto Error;
			close(fd);
		}
	}
	return;
Error:
	pfmt(err, "read: %s\n", strerror(errno));
	setstatus("read error");
	poplist();
	return;
Usage:
	pfmt(err, "Usage: read [ -m | -n nlines | -c nbytes | -r nrunes ] [ file ... ]\n");
	setstatus("read usage");
	poplist();
}

void
execrm(void)
{
	int i, recurse, force;
	char *fd;
	Dir *db;
	word *a;

	force = recurse = 0;
	popword(); /* rm */
	while(runq->argv->words && runq->argv->words->word[0]=='-'){
		char *f = runq->argv->words->word+1;
		if(*f == '-'){
			popword();
			break;
		}
		for(; *f; f++){
			switch(*f){
			case 'r':
				recurse = 1;
				break;
			case 'f':
				force = 1;
				break;
			default:
				goto Usage;
			}
			popword();
		}
	}
	a = runq->argv->words;
	for(;a;a = a->next){
		if(remove(a->word) != -1)
			continue;
		db = nil;
		if(recurse && (db=dirstat(a->word))!=nil && (db->qid.type&QTDIR))
			rmdir(a->word);
		else
			goto Error;
		free(db);
	}
	return;
Error:
	pfmt(err, "rm: %r\n", strerror(errno));
	poplist();
	return;
Usage:
	pfmt(err, "usage: rm [-fr] file ...\n");
	setstatus("rm usage");
	poplist();
	return;
}

void
execns(void)
{
//print("Execns\n");
}

void
execbind(void)
{
//print("Execbind\n");
	ulong flag = 0;
	int qflag = 0;
	popword(); /* "bind" */
	while(runq->argv->words && runq->argv->words->word[0]=='-'){
		char *f = runq->argv->words->word+1;
		if(*f == '-'){
			popword();
			break;
		}
		for(; *f; f++){
			switch(*f){
			case 'a':
				flag |= MAFTER;
				break;
			case 'b':
				flag |= MBEFORE;
				break;
			case 'c':
				flag |= MCREATE;
				break;
			case 'q':
				qflag = 1;
				break;
			default:
				goto Usage;
			}
			popword();
		}
	}
	if(count(runq->argv->words)!=2 || ((flag & MAFTER) && (flag & MBEFORE)))
		goto Usage;
	if(bind(runq->argv->words->word, runq->argv->words->next->word, flag) == -1)
		goto Error;
	return;
Error:
	poplist();
	if(qflag)
		return;
	pfmt(err, "bind: %s\n", strerror(errno));
	return;
Usage:
	pfmt(err, "usage: bind [-b|-a|-c|-bc|-ac] new old\n");
	setstatus("bind usage");
	poplist();
	return;
}

void
execmount(void)
{
//print("Execmount\n");
	char *spec = "";
	int flag = MREPL;
	int qflag, noauth, fd;
	qflag = noauth = 0;

	popword(); /* mount */
	while(runq->argv->words && runq->argv->words->word[0]=='-'){
		char *f = runq->argv->words->word+1;
		if(*f == '-'){
			popword();
			break;
		}
		for(; *f; f++){
			switch(*f){
			case 'a':
				flag |= MAFTER;
				break;
			case 'b':
				flag |= MBEFORE;
				break;
			case 'c':
				flag |= MCREATE;
				break;
			case 'C':
				flag |= MCACHE;
				break;
			case 'n':
				noauth = 1;
				break;
			case 'q':
				qflag = 1;
				break;
			default:
				goto Usage;
			}
			popword();
		}
	}
	if(count(runq->argv->words)==3)
		spec = runq->argv->words->next->next->word;
	else if(count(runq->argv->words)!=2)
		goto Usage;
	if((flag&MAFTER)&&(flag&MBEFORE))
		goto Usage;
	fd = Open(runq->argv->words->word, ORDWR);
	if(fd < 0)
		goto Error;
	if(sysmount(fd, -1, runq->argv->words->next->word, flag, spec) < 0)
		goto Error;
	poplist();
	return;
Error:
	setstatus("mount error");
	poplist();
	if(qflag)
		return;
	pfmt(err, "mount: %s\n", strerror(errno));
	return;
Usage:
	pfmt(err, "usage: mount [-a|-b] [-cCnNq] [-k keypattern] /srv/service dir [spec]\n");
	setstatus("mount usage");
	return;
}

void
execunmount(void)
{
	//unmount
}

int
cat(int f, char *s)
{
	char buf[IOUNIT];
	long n;
	while((n=Read(f, buf, sizeof buf))>0)
		if(Write(1, buf, n)!=n)
			pfmt(err, "write error copying %s: %s", s, strerror(errno));
	if(n < 0)
		pfmt(err, "error reading %s: %s", s, strerror(errno));
}

void
execcat(void)
{
	int f;
	popword(); /* cat */
	word *a;

	a = runq->argv->words;
	if(count(a) == 0){
		cat(f, "<stdin>");
		close(f);
	} else for(;a;a = a->next){
		f = Open(a->word, OREAD);
		if(f < 0){
			pfmt(err, "can't open %s: %s", a->word, strerror(errno));
			break;
		}
		cat(f, a->word);
		close(f);
		write(1, "\n", 1);
	}
	poplist();
}

int
hasmode(char *f, ulong m)
{
	int r;
	Dir *dir;
	dir = dirstat(f);
	if (dir == nil)
		return 0;
	r = (dir->mode & m) != 0;
	free(dir);
	return r;
}

void
exectest(void)
{
	/* TODO(halfwit): Only care about -d for my needs, but test has a ton of things */
	setstatus("no such file");
	if(strcmp(runq->argv->words->next->word, "-d")==0)
		if(hasmode(runq->argv->words->next->next->word, DMDIR))
			setstatus("");
	poplist();
}

void
execmkdir(void)
{

}

void
exececho(void)
{
	int nflag;
	int i, len;
	char *buf, *p;
	word *a, *c;

	nflag = 0;
	popword(); /* echo */
	a = runq->argv->words;
	if(count(a) > 0 && strcmp(a->word, "-n") == 0){
		a = a->next; // move up our counter as well
		nflag = 1;
	}
	len = 1;
	for(c = a; c; c = c->next)
		len += strlen(c->word)+1;
	buf = malloc(len);
	if(buf == 0)
		panic("no memory");
	p = buf;
	for(c = a; c; c = c->next){
		strcpy(p, c->word);
		p += strlen(p);
		if(c->next)
			*p++ = ' ';
	}
	if(!nflag)
		*p++ = '\n';
	write(1, buf, p-buf);
}

void
execls(void)
{
	Dir *db;
	int fd, n, i;
	char *path;

	/* Read in our dir and just output name in a row */
	popword(); /* "ls" */
	switch(count(runq->argv->words)){
	case 0:
		path = ".";
		break;
	case 1:
		path = runq->argv->words->word;
		break;
	default:
		pfmt(err, "ls: listing multiple files not supported\n");
		return;
	}
	db = dirstat(path);
	if(db == nil)
		goto Error;
	if((db->qid.type&QTDIR)) {
		free(db);
		fd = open(path, OREAD);
		if(fd == -1)
			goto Error;
		n = dirreadall(fd, &db);
		if(n < 0)
			goto Error;
		for(i = 0; i < n; i++){
			write(1, db->name, strlen(db->name));
			write(1, "\n", 1);
			db++;
		}
		close(fd);
	} else {
		write(1, db->name, strlen(db->name));
		write(1, "\n", 1);
	}
	return;
Error:
	pfmt(err, "ls: %sr\n", strerror(errno));
	setstatus("ls error");
	poplist();
	return;
}

void
execexec(void)
{
//	print("Execexec\n");
	char **argv;
	word *path;

	popword();	/* "exec" */
	if(runq->argv->words==0){
		Xerror1("exec: empty argument list");
		return;
	}
	argv = mkargv(runq->argv->words);
	Updenv();
	doredir(runq->redir);
	for(path = searchpath(argv[1], "path"); path; path = path->next){
		argv[0] = makercpath(path->word, argv[1]);
		Exec(argv);
	}
	setstatus(Errstr());
	pfln(err, srcfile(runq), runq->line);
	pfmt(err, ": %s: %s\n", argv[1], getstatus());
	Xexit();
}

void
execfunc(var *func)
{
//	print("Execfunc\n");
	popword();	/* name */
	startfunc(func, Poplist(), runq->local, runq->redir);
}

void
execcd(void)
{
	word *a = runq->argv->words;
	word *cdpath;
	char *dir;
//	print("Execcd\n");
	setstatus("can't cd");
	switch(count(a)){
	default:
		pfmt(err, "Usage: cd [directory]\n");
		break;
	case 2:
		a = a->next;
		for(cdpath = searchpath(a->word, "cdpath"); cdpath; cdpath = cdpath->next){
			dir = makercpath(cdpath->word, a->word);
			if(Chdir(dir)>=0){
				if(cdpath->word[0] != '\0' && strcmp(cdpath->word, ".") != 0)
					pfmt(err, "%s\n", dir);
				free(dir);
				setstatus("");
				break;
			}
			free(dir);
		}
		if(cdpath==0)
			pfmt(err, "Can't cd %s: %s\n", a->word, Errstr());
		break;
	case 1:
		a = vlook("home")->val;
		if(a){
			if(Chdir(a->word)>=0)
				setstatus("");
			else
				pfmt(err, "Can't cd %s: %s\n", a->word, Errstr());
		}
		else
			pfmt(err, "Can't cd -- $home empty\n");
		break;
	}
	poplist();
}

void
execexit(void)
{
//	print("Execexit\n");
	switch(count(runq->argv->words)){
	default:
		pfmt(err, "Usage: exit [status]\nExiting anyway\n");
	case 2:
		setstatus(runq->argv->words->next->word);
	case 1:	Xexit();
	}
}

void
execshift(void)
{
	int n;
	word *a;
	var *star;
//	print("execshift\n");
	switch(count(runq->argv->words)){
	default:
		pfmt(err, "Usage: shift [n]\n");
		setstatus("shift usage");
		poplist();
		return;
	case 2:
		n = atoi(runq->argv->words->next->word);
		break;
	case 1:
		n = 1;
		break;
	}
	star = vlook("*");
	for(;n>0 && star->val;--n){
		a = star->val->next;
		free(Freeword(star->val));
		star->val = a;
		star->changed = 1;
	}
	setstatus("");
	poplist();
}

int
mapfd(int fd)
{
	redir *rp;
	for(rp = runq->redir;rp;rp = rp->next){
		switch(rp->type){
		case RCLOSE:
			if(rp->from==fd)
				fd=-1;
			break;
		case RDUP:
		case ROPEN:
			if(rp->to==fd)
				fd = rp->from;
			break;
		}
	}
	return fd;
}

void
execcmds(io *input, char *file, var *local, redir *redir)
{
	static union code rdcmds[5];
//	print("Execcmds\n");
	if(rdcmds[0].i==0){
		rdcmds[0].i = 1;
		rdcmds[1].s="*rdcmds*";
		rdcmds[2].f = Xrdcmds;
		rdcmds[3].f = Xreturn;
		rdcmds[4].f = 0;
	}

	if(exitnext()) turfstack(local);

	start(rdcmds, 2, local, redir);
	runq->lex = newlexer(input, file);
}

void
execeval(void)
{
	char *cmds;
	int len;
	io *f;
//print("Execeval\n");
	popword();	/* "eval" */

	if(runq->argv->words==0){
		Xerror1("Usage: eval cmd ...");
		return;
	}
	Xqw();		/* make into single word */
	cmds = Popword();
	len = strlen(cmds);
	cmds[len++] = '\n';
	poplist();

	f = openiostr();
	pfln(f, srcfile(runq), runq->line);
	pstr(f, " *eval*");

	execcmds(openiocore(cmds, len), closeiostr(f), runq->local, runq->redir);
}

void
execdot(void)
{
	int fd, bflag, iflag, qflag;
	word *path, *argv;
	char *file;
//print("Execdot\n");
	popword();	/* "." */

	bflag = iflag = qflag = 0;
	while(runq->argv->words && runq->argv->words->word[0]=='-'){
		char *f = runq->argv->words->word+1;
		if(*f == '-'){
			popword();
			break;
		}
		for(; *f; f++){
			switch(*f){
			case 'b':
				bflag = 1;
				continue;
			case 'i':
				iflag = 1;
				continue;
			case 'q':
				qflag = 1;
				continue;
			}
			goto Usage;
		}
		popword();
	}

	/* get input file */
	if(runq->argv->words==0){
Usage:
		Xerror1("Usage: . [-biq] file [arg ...]");
		return;
	}
	argv = Poplist();
		
	file = 0;
	fd = -1;
	for(path = searchpath(argv->word, "path"); path; path = path->next){
		file = makercpath(path->word, argv->word);
		fd = Open(file, 0);
		if(fd >= 0)
			break;
		free(file);
	}
	if(fd<0){
		if(!qflag)
			Xerror3(". can't open", argv->word, Errstr());
		freewords(argv);
		return;
	}
	execcmds(openiofd(fd), file, (var*)0, runq->redir);
	pushredir(RCLOSE, fd, 0);
	runq->lex->qflag = qflag;
	runq->iflag = iflag;
	if(iflag || (!bflag && flag['b']==0)){
		runq->lex->peekc=EOF;
		runq->lex->epilog="";
	}

	runq->local = newvar("*", runq->local);
	runq->local->val = argv->next;
	argv->next=0;
	runq->local->changed = 1;

	runq->local = newvar("0", runq->local);
	runq->local->val = argv;
	runq->local->changed = 1;
}

void
execflag(void)
{
//	print("Execflag\n");
	char *letter, *val;
	switch(count(runq->argv->words)){
	case 2:
		setstatus(flag[(unsigned char)runq->argv->words->next->word[0]]?"":"flag not set");
		break;
	case 3:
		letter = runq->argv->words->next->word;
		val = runq->argv->words->next->next->word;
		if(strlen(letter)==1){
			if(strcmp(val, "+")==0){
				flag[(unsigned char)letter[0]] = flagset;
				setstatus("");
				break;
			}
			if(strcmp(val, "-")==0){
				flag[(unsigned char)letter[0]] = 0;
				setstatus("");
				break;
			}
		}
	default:
		Xerror1("Usage: flag [letter] [+-]");
		return;
	}
	poplist();
}

void
execwhatis(void){	/* mildly wrong -- should fork before writing */
	word *a, *b, *path;
	var *v;
	char *file;
	io *out;
	int found, sep;
	a = runq->argv->words->next;
//	print("Execwhatis\n");
	if(a==0){
		Xerror1("Usage: whatis name ...");
		return;
	}
	setstatus("");
	out = openiofd(mapfd(1));
	for(;a;a = a->next){
		v = vlook(a->word);
		if(v->val){
			pfmt(out, "%s=", a->word);
			if(v->val->next==0)
				pfmt(out, "%q\n", v->val->word);
			else{
				sep='(';
				for(b = v->val;b && b->word;b = b->next){
					pfmt(out, "%c%q", sep, b->word);
					sep=' ';
				}
				pstr(out, ")\n");
			}
			found = 1;
		}
		else
			found = 0;
		v = gvlook(a->word);
		if(v->fn)
			pfmt(out, "fn %q %s\n", v->name, v->fn[v->pc-1].s);
		else{
			if(builtinfunc(a->word))
				pfmt(out, "builtin %s\n", a->word);
			else {
				for(path = searchpath(a->word, "path"); path; path = path->next){
					file = makercpath(path->word, a->word);
					if(Executable(file)){
						pfmt(out, "%s\n", file);
						free(file);
						break;
					}
					free(file);
				}
				if(!path && !found){
					pfmt(err, "%s: not found\n", a->word);
					setstatus("not found");
				}
			}
		}
		flushio(out);
	}
	poplist();
	free(closeiostr(out));	/* don't close fd */
}

void
execwait(void)
{
//	print("Execwait\n");
	switch(count(runq->argv->words)){
	default:
		Xerror1("Usage: wait [pid]");
		return;
	case 2:
		Waitfor(atoi(runq->argv->words->next->word));
		break;
	case 1:
		Waitfor(-1);
		break;
	}
	poplist();
}
