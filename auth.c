#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>
#include <authsrv.h>
#include <libsec.h>
#include "drawcpu.h"

char *authserver; // Likely unused

AuthInfo*
establish(Ticket *t, uchar *rand, int dp9ik)
{
	AuthInfo *ai;
	ai = mallocz(sizeof(AuthInfo), 1);
	ai->suid = t->suid;
	ai->cuid = t->cuid;
	if(dp9ik){
		static char info[] = "Plan 9 session secret";

		ai->nsecret = 256;
		ai->secret = mallocz(ai->nsecret, 1);
		hkdf_x(rand, 2*NONCELEN,
			(uchar*)info, sizeof(info)-1,
			(uchar*)t->key, NONCELEN,
			ai->secret, ai->nsecret,
			hmac_sha2_256, SHA2_256dlen
		);
	} else {
		ai->nsecret = 8;
		ai->secret = mallocz(ai->nsecret, 1);
		des56to64((uchar*)t->key, ai->secret);
	}

	return ai;
}


AuthInfo*
auth_host(int fd, char *authdom, char *pass)
{
    char *user;
	char cpd[ANAMELEN+DOMLEN+1], spd[2*DOMLEN+18], *proto, *dom;
	char trbuf[TICKREQLEN+PAKYLEN], abuf[MAXTICKETLEN+MAXAUTHENTLEN];
	uchar srand[2*NONCELEN], cchal[CHALLEN], y[PAKYLEN];
    Authkey key;
    Authenticator auth;
	AuthInfo *ai;
	PAKpriv p;
	Ticketreq tr;
	Ticket t;
	int afd;

	if((afd = open("/mnt/factotum/ctl", ORDWR)) >= 0)
		return auth_proxy(0, nil, "proto=p9any role=server");
	
    int n, m, dp9ik = 0;
    /* Start p9any */
#if P9ANY_VERSION==2
	n = sprintf(spd, "v.2 p9sk1@%s dp9ik@%s ", authdom, authdom);
#else
	n = sprintf(spd, "p9sk1@%s dp9ik@%s ", authdom, authdom);
#endif
	if(write(fd, spd, n+1) != n+1){
        fprint(dbg, "short write on p9any\n");
        return nil;
    }
    
	if(read(fd, cpd, ANAMELEN+DOMLEN+1) <= 0){
        fprint(dbg, "short read on client proto\n");
        return nil;
    }

	proto = strtok(cpd, " ");
	dom = strtok(NULL, " ");
	if(proto == NULL || dom == NULL){
        fprint(dbg, "unable to read requested proto/dom pair\n");
        return nil;
    }

	if(strcmp(proto, "dp9ik") == 0)
		dp9ik = 1;

#if P9ANY_VERSION==2
	if(write(fd, "OK\0", 3) != 3){
        fprint(dbg, "short write on proto challenge OK");
        return nil;
    }
#endif

	/* p9any success, start selected protocol */
    user = getenv("USER");

	memset(&tr, 0, sizeof(tr));
	tr.type = AuthTreq;
	strcpy(tr.authid, user);
	strcpy(tr.authdom, authdom);
	genrandom((uchar*)tr.chal, CHALLEN);

	if((n = readn(fd, cchal, CHALLEN)) != CHALLEN){
        fprint(dbg, "Short read on p9sk1 challenge\n");
        return nil;
    }

	m = TICKREQLEN;
    passtokey(&key, pass);
	if(dp9ik){
		authpak_hash(&key, user);
		tr.type = AuthPAK;
		m += PAKYLEN;
	}
	n = convTR2M(&tr, trbuf, m);
	if(dp9ik)
		authpak_new(&p, &key, (uchar *)trbuf+n, 1);

    //print(trbuf);
	if(write(fd, trbuf, m) < m){
        fprint(dbg, "short read sending ticket request\n");
        return nil;
    }
	if(dp9ik){
		if(readn(fd, y, PAKYLEN) < PAKYLEN){
            fprint(dbg, "short read on client pk");
            return nil;
        }
		if(authpak_finish(&p, &key, y)){
            fprint(dbg, "unable to decrypt message in auth_host\n");
            return nil;
        }
	}
	if((n = readn(fd, abuf, MAXTICKETLEN+MAXAUTHENTLEN)) != MAXTICKETLEN+MAXAUTHENTLEN){
        fprint(dbg, "short read receiving ticket\n");
        return nil;
    }
	m = convM2T(abuf, n, &t, &key);
	if(m <= 0 || convM2A(abuf+m, n-m, &auth, &t) <= 0){
        fprint(dbg, "short read on ticket\n");
        return nil;
    }
	if(dp9ik && t.form == 0){
        fprint(dbg, "auth_host: auth protocol botch");
        return nil;
    }

	if(t.num != AuthTs || tsmemcmp(t.chal, tr.chal, CHALLEN) != 0){
        fprint(dbg, "auth protocol botch\n");
        return nil;
    }

	if(auth.num != AuthAc || tsmemcmp(auth.chal, tr.chal, CHALLEN) != 0){
        fprint(dbg, "auth portocol botch");
        return nil;
    }
	memmove(srand, auth.rand, NONCELEN);
	genrandom(srand + NONCELEN, NONCELEN);
	auth.num = AuthAs;
	memmove(auth.chal, cchal, CHALLEN);
	memmove(auth.rand, srand + NONCELEN, NONCELEN);
	if((n = convA2M(&auth, abuf, sizeof(abuf), &t)) < 0){
        fprint(dbg, "unable to convert authenticator to message\n");
        return nil;
    }
	if(write(fd, abuf, n) != n){
        fprint(dbg, "short write sending authenticator\n");
        return nil;
    }
	ai = establish(&t, srand, dp9ik);
	memset(&key, 0, sizeof(key));
	memset(&t, 0, sizeof(t));
	memset(&auth, 0, sizeof(auth));
	return ai;
}

int
p9authsrv(char *authdom, char *pass)
{
    AuthInfo *ai;
    TLSconn *conn;

    int fd = open("/dev/cons", O_RDWR);
    ai = auth_host(fd, authdom, pass);
    if(ai == nil){
        fprint(dbg, "can't authenticate: %r\n");
        return -1;
    }

    conn = mallocz(sizeof(TLSconn), 1);
	conn->pskID = "p9secret";
	conn->psk = ai->secret;
	conn->psklen = ai->nsecret;

	fd = tlsServer(fd, conn);
	if(fd < 0){
        fprint(dbg, "tlsServer: %r\n");
		return -1;
    }
    
	auth_freeAI(ai);
	free(conn->sessionID);
	free(conn);

    return fd;
}
