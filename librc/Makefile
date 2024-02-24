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
	drawcpu.$O\
	y.tab.$O\

default: $(LIB)
$(LIB): $(OFILES)
	$(AR) r $(LIB) $(OFILES)
	$(RANLIB) $(LIB)

%.$O: %.c
	$(CC) $(CFLAGS) $*.c
	