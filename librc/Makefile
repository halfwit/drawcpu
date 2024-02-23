ROOT=..
include ../Make.config

LIB=librc.a
OFILES=\
	code.$O\
	exec.$O\
	getflags.$O\
	glob.$O\
	here.$O\
	io.$O\
	lex.$O\
	pcmd.$O\
	pfnc.$O\
	simple.$O\
	subr.$O\
	trap.$O\
	tree.$O\
	var.$O\
	havefork.$O\
	unix.$O\
	y.tab.c

default: $(LIB)
$(LIB): $(OFILES) $(YFILES)
	$(AR) r $(LIB) $(OFILES) $(YFILES)
	$(RANLIB) $(LIB)

%.$O: %.c
	$(CC) $(CFLAGS) $*.c
