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
	devcmd.$O\
	devcons.$O\
	devenv.$O\
	devfs-$(OS).$O\
	devlfd-$(OS).$O\
	devip-$(OS).$O\
	devip.$O\
	devmnt.$O\
	devpipe.$O\
	devroot.$O\
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
	wait.$O\
	waserror.$O\
	$(OS).$O

default: $(LIB)
$(LIB): $(OFILES)
	$(AR) r $(LIB) $(OFILES)
	$(RANLIB) $(LIB)

%.$O: %.c
	$(CC) $(CFLAGS) $*.c

