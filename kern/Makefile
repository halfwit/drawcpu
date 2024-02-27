ROOT=..
include ../Make.config
LIB=libkern.a

OFILES=\
	alloc.$O\
	allocb.$O\
	await.$O\
	chan.$O\
	data.$O\
	dev.$O\
	devaudio.$O\
	devaudio-$(AUDIO).$O\
	devcmd.$O\
	devcons.$O\
	devdraw.$O\
	devenv.$O\
	devfs-$(OS).$O\
	devip.$O\
	devip-$(OS).$O\
	devkbd.$O\
	devlfd-$(OS).$O\
	devmnt.$O\
	devmouse.$O\
	devpipe.$O\
	devroot.$O\
	devssl.$O\
	devtls.$O\
	devtab.$O\
	dirread.$O\
	error.$O\
	getwd.$O\
	parse.$O\
	pgrp.$O\
	postnote.$O\
	procinit.$O\
	notify.$O\
	rfork.$O\
	rwlock.$O\
	sleep.$O\
	stub.$O\
	sysfile.$O\
	time.$O\
	qio.$O\
	qlock.$O\
	term.$O\
	wait.$O\
	waserror.$O\
	$(OS).$O

default: $(LIB)
$(LIB): $(OFILES)
	$(AR) r $(LIB) $(OFILES)
	$(RANLIB) $(LIB)

%.$O: %.c
	$(CC) $(CFLAGS) $*.c

