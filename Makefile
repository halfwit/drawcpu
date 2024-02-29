ROOT=.

include Make.config

OFILES=\
	main.$O\
	aan.$O\
	latin1.$O\
	getuser.$O\
	$(XOFILES)\

LIBS1=\
	kern/libkern.a\
	rc/librc.a\
	libmemdraw/libmemdraw.a\
	libmemlayer/libmemlayer.a\
	libdraw/libdraw.a\
	gui-$(GUI)/libgui.a\
	libc/libc.a\

# stupid gcc
LIBS=$(LIBS1) libmachdep.a

default: $(TARG)
$(TARG): $(OFILES) $(LIBS)
	$(CC) $(LDFLAGS) -o $(TARG) $(OFILES) $(LIBS) $(LDADD)

%.$O: %.c
	$(CC) $(CFLAGS) $*.c

clean:
	rm -f *.o */*.o */*.a *.a drawcpu drawcpu.exe

kern/libkern.a:
	(cd kern; $(MAKE))


libmemdraw/libmemdraw.a:
	(cd libmemdraw; $(MAKE))

libmemlayer/libmemlayer.a:
	(cd libmemlayer; $(MAKE))

libdraw/libdraw.a:
	(cd libdraw; $(MAKE))

libc/libc.a:
	(cd libc; $(MAKE))

rc/librc.a:
	(cd rc; $(MAKE))

gui-$(GUI)/libgui.a:
	(cd gui-$(GUI); $(MAKE))
