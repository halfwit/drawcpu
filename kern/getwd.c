#include <u.h>
#include <libc.h>

#undef getwd

char*
sysgetwd(char *s, int ns)
{
	return getcwd(s, ns);
}
