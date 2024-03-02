#include "u.h"
#include "lib.h"
#include "dat.h"
#include "fns.h"
#include "error.h"

extern Dev consdevtab;
extern Dev rootdevtab;
extern Dev pipedevtab;
extern Dev fsdevtab;
extern Dev lfddevtab;
extern Dev ipdevtab;
extern Dev mntdevtab;
extern Dev lfddevtab;
extern Dev cmddevtab;
extern Dev envdevtab;

Dev *devtab[] = {
	&rootdevtab,
	&consdevtab,
	&pipedevtab,
	&fsdevtab,
	&lfddevtab,
	&ipdevtab,
	&mntdevtab,
	&lfddevtab,
	&cmddevtab,
	&envdevtab,
	0
};

