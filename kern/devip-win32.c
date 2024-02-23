#define  _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "u.h"
#include "lib.h"
#include "dat.h"
#include "fns.h"
#include "error.h"
#include "ip.h"

#include "devip.h"

#ifdef MSVC
#pragma comment(lib, "wsock32.lib")
#endif

#undef listen
#undef accept
#undef bind

static int
family(unsigned char *addr)
{
	if(isv4(addr))
		return AF_INET;
	return AF_INET6;
}

static int
addrlen(struct sockaddr_storage *ss)
{
	switch(ss->ss_family){
	case AF_INET:
		return sizeof(struct sockaddr_in);
	case AF_INET6:
		return sizeof(struct sockaddr_in6);
	}
	return 0;
}

void
osipinit(void)
{
	WSADATA wasdat;
	char buf[1024];

	if(WSAStartup(MAKEWORD(1, 1), &wasdat) != 0)
		panic("no winsock.dll");

	gethostname(buf, sizeof(buf));
	kstrdup(&sysname, buf);
}

int
so_socket(int type, unsigned char *addr)
{
	int fd, one;

	switch(type) {
	default:
		error("bad protocol type");
	case S_TCP:
		type = SOCK_STREAM;
		break;
	case S_UDP:
		type = SOCK_DGRAM;
		break;
	}

	fd = socket(family(addr), type, 0);
	if(fd < 0)
		oserror();

	one = 1;
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one)) > 0){
		oserrstr();
		print("setsockopt: %s\n", up->errstr);
	}

	return fd;
}


void
so_connect(int fd, unsigned char *raddr, unsigned short rport)
{
	struct sockaddr_storage ss;

	memset(&ss, 0, sizeof(ss));

	ss.ss_family = family(raddr);

	switch(ss.ss_family){
	case AF_INET:
		hnputs(&((struct sockaddr_in*)&ss)->sin_port, rport);
		v6tov4((unsigned char*)&((struct sockaddr_in*)&ss)->sin_addr.s_addr, raddr);
		break;
	case AF_INET6:
		hnputs(&((struct sockaddr_in6*)&ss)->sin6_port, rport);
		memcpy(&((struct sockaddr_in6*)&ss)->sin6_addr.s6_addr, raddr, sizeof(struct in6_addr));
		break;
	}

	if(connect(fd, (struct sockaddr*)&ss, addrlen(&ss)) < 0)
		oserror();
}

void
so_getsockname(int fd, unsigned char *laddr, unsigned short *lport)
{
	int len;
	struct sockaddr_storage ss;

	len = sizeof(ss);
	if(getsockname(fd, (struct sockaddr*)&ss, &len) < 0)
		oserror();

	switch(ss.ss_family){
	case AF_INET:
		v4tov6(laddr, (unsigned char*)&((struct sockaddr_in*)&ss)->sin_addr.s_addr);
		*lport = nhgets(&((struct sockaddr_in*)&ss)->sin_port);
		break;
	case AF_INET6:
		memcpy(laddr, &((struct sockaddr_in6*)&ss)->sin6_addr.s6_addr, sizeof(struct in6_addr));
		*lport = nhgets(&((struct sockaddr_in6*)&ss)->sin6_port);
		break;
	default:
		error("not AF_INET or AF_INET6");
	}
}

void
so_listen(int fd)
{
	if(listen(fd, 5) < 0)
		oserror();
}

int
so_accept(int fd, unsigned char *raddr, unsigned short *rport)
{
	int nfd;
	int len;
	struct sockaddr_storage ss;

	len = sizeof(ss);
	nfd = accept(fd, (struct sockaddr*)&ss, &len);
	if(nfd < 0)
		oserror();

	switch(ss.ss_family){
	case AF_INET:
		v4tov6(raddr, (unsigned char*)&((struct sockaddr_in*)&ss)->sin_addr.s_addr);
		*rport = nhgets(&((struct sockaddr_in*)&ss)->sin_port);
		break;
	case AF_INET6:
		memcpy(raddr, &((struct sockaddr_in6*)&ss)->sin6_addr.s6_addr, sizeof(struct in6_addr));
		*rport = nhgets(&((struct sockaddr_in6*)&ss)->sin6_port);
		break;
	default:
		error("not AF_INET or AF_INET6");
	}
	return nfd;
}

void
so_bind(int fd, int su, unsigned short port, unsigned char *addr)
{
	int i, one;
	struct sockaddr_storage ss;

	one = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one)) < 0){
		oserrstr();
		print("setsockopt: %r");
	}

	if(su) {
		for(i = 600; i < 1024; i++) {
			memset(&ss, 0, sizeof(ss));
			ss.ss_family = family(addr);

			switch(ss.ss_family){
			case AF_INET:
				((struct sockaddr_in*)&ss)->sin_port = i;
				break;
			case AF_INET6:
				((struct sockaddr_in6*)&ss)->sin6_port = i;
				break;
			}

			if(bind(fd, (struct sockaddr*)&ss, addrlen(&ss)) >= 0)	
				return;
		}
		oserror();
	}

	memset(&ss, 0, sizeof(ss));
	ss.ss_family = family(addr);

	switch(ss.ss_family){
	case AF_INET:
		hnputs(&((struct sockaddr_in*)&ss)->sin_port, port);
		break;
	case AF_INET6:
		hnputs(&((struct sockaddr_in6*)&ss)->sin6_port, port);
		break;
	}

	if(bind(fd, (struct sockaddr*)&ss, addrlen(&ss)) < 0)
		oserror();
}

int
so_gethostbyname(char *host, char **hostv, int n)
{
	char buf[INET6_ADDRSTRLEN];
	struct addrinfo *r, *p;
	DWORD l;
	int i;

	r = NULL;
	if(getaddrinfo(host, NULL, NULL, &r))
		return 0;
	for(i = 0, p = r; i < n && p != NULL; p = p->ai_next){
		switch (p->ai_family) {
		default:
			continue;
		case AF_INET:
		case AF_INET6:
			l = sizeof(buf);
			WSAAddressToStringA(p->ai_addr, p->ai_addrlen, NULL, buf, &l);
			break;
		}
		hostv[i++] = strdup(buf);
	}
	freeaddrinfo(r);
	return i;
}

int
so_getservbyname(char *service, char *net, char *port)
{
	struct servent *s;

	s = getservbyname(service, net);
	if(s == 0)
		return -1;

	sprint(port, "%d", nhgets(&s->s_port));
	return 0;
}

int
so_send(int fd, void *d, int n, int f)
{
	return send(fd, d, n, f);
}

int
so_recv(int fd, void *d, int n, int f)
{
	return recv(fd, d, n, f);
}
