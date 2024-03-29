#include <u.h>
#include <libc.h>
#include <fcall.h>

static
long
dirpackage(uchar *buf, long ts, Dir **d)
{
	char *s;
	long i, n, nn, m;

	/*
	 * first find number of all stats, check they look like stats, & size all associated strings
	 */
	n = 0;
	for(i = 0; i < ts; i += m){
		if(i+BIT16SZ >= ts)
			return -1;
		m = BIT16SZ + GBIT16(&buf[i]);
		if(i+m > ts || statcheck(&buf[i], m) < 0)
			return -1;
		n++;
	}

	*d = malloc(n * sizeof(Dir) + ts);
	if(*d == nil)
		return -1;

	/*
	 * then convert all buffers
	 */
	s = (char*)*d + n * sizeof(Dir);
	nn = 0;
	for(i = 0; i < ts; i += m){
		m = BIT16SZ + GBIT16(&buf[i]);
		if(i+m > ts || nn >= n || convM2D(&buf[i], m, *d + nn, s) != m){
			free(*d);
			*d = nil;
			return -1;
		}
		nn++;
		s += m;
	}

	return nn;
}

long
dirread(int fd, Dir **d)
{
	uchar *buf;
	long ts;

	*d = nil;
	buf = malloc(DIRMAX);
	if(buf == nil)
		return -1;
	ts = read(fd, buf, DIRMAX);
	if(ts > 0)
		ts = dirpackage(buf, ts, d);
	free(buf);
	return ts;
}

long
dirreadall(int fd, Dir **d)
{
	uchar *buf, *nbuf;
	long n, ts;

	*d = nil;
	buf = nil;
	ts = 0;
	for(;;){
		nbuf = realloc(buf, ts+DIRMAX);
		if(nbuf == nil){
			free(buf);
			return -1;
		}
		buf = nbuf;
		n = read(fd, buf+ts, DIRMAX);
		if(n <= 0)
			break;
		ts += n;
	}
	if(ts > 0)
		ts = dirpackage(buf, ts, d);
	free(buf);
	if(ts == 0 && n < 0)
		return -1;
	return ts;
}