
int	Executable(char*);
char*	Freeword(word*);
char*	Getstatus(void);
int	Isatty(int);
word*	Newword(char*,word*);
void	Noerror(void);
word*	Poplist(void);
char*	Popword(void);
word*	Pushword(char*);
void	Setstatus(char*);
void	Updenv(void);
void	Vinit(void);
int	Waitfor(int);
void	addwaitpid(int);
void	clearwaitpids(void);
void	codefree(code*);
int	compile(tree*);
int	count(word*);
char*	deglob(char*);
void	delwaitpid(int);
void	dotrap(void);
void	freenodes(void);
void    notifyf(void*, char*);
void	freewords(word*);
void	globword(word*);
int	havewaitpid(int);
int	idchr(int);
void	inttoascii(char*, int);
void	kinit(void);
int	mapfd(int);
int	match(char*, char*, int);
char*	rcmakepath(char*, char*);
void	pfln(io*, char*, int);
void	poplist(void);
void	popword(void);
void	pprompt(void);
void	Prompt(char*);
void	psubst(io*, unsigned char*);
void	pushlist(void);
void	pushredir(int, int, int);
word*	pushword(char*);
void	readhere(io*);
void	heredoc(tree*);
void	setstatus(char*);
void	skipnl(void);
void	start(code*, int, var*, redir*);
int	truestatus(void);
void	usage(char*);
int	wordchr(int);
void	yyerror(char*);
int	yylex(void);
int	yyparse(void);
#define notify(x)