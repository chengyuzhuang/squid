
/*
 * $Id: send-announce.c,v 1.18 1996/09/16 21:11:14 wessels Exp $
 *
 * DEBUG: section 27    Cache Announcer
 * AUTHOR: Duane Wessels
 *
 * SQUID Internet Object Cache  http://www.nlanr.net/Squid/
 * --------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from the
 *  Internet community.  Development is led by Duane Wessels of the
 *  National Laboratory for Applied Network Research and funded by
 *  the National Science Foundation.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *  
 */

#include "squid.h"

void
send_announce()
{
    LOCAL_ARRAY(char, tbuf, 256);
    LOCAL_ARRAY(char, sndbuf, BUFSIZ);
    icpUdpData *qdata = NULL;
    struct hostent *hp = NULL;
    char *host = NULL;
    char *file = NULL;
    u_short port;
    int fd;
    int l;
    int n;

    host = Config.Announce.host;
    port = Config.Announce.port;

    if ((hp = ipcache_gethostbyname(host, IP_BLOCKING_LOOKUP)) == NULL) {
	debug(27, 1, "send_announce: Unknown host '%s'\n", host);
	return;
    }
    sndbuf[0] = '\0';
    sprintf(tbuf, "cache_version SQUID/%s\n", version_string);
    strcat(sndbuf, tbuf);
    sprintf(tbuf, "Running on %s %d %d\n",
	getMyHostname(),
	Config.Port.http,
	Config.Port.icp);
    strcat(sndbuf, tbuf);
    if (Config.adminEmail) {
	sprintf(tbuf, "cache_admin: %s\n", Config.adminEmail);
	strcat(sndbuf, tbuf);
    }
    sprintf(tbuf, "generated %d [%s]\n",
	(int) squid_curtime,
	mkhttpdlogtime(&squid_curtime));
    strcat(sndbuf, tbuf);
    l = strlen(sndbuf);

    if ((file = Config.Announce.file)) {
	fd = file_open(file, NULL, O_RDONLY);
	if (fd > -1 && (n = read(fd, sndbuf + l, BUFSIZ - l - 1)) > 0) {
	    l += n;
	    sndbuf[l] = '\0';
	} else {
	    debug(27, 1, "send_announce: %s: %s\n", file, xstrerror());
	}
    }
    qdata = xcalloc(1, sizeof(icpUdpData));
    qdata->msg = xstrdup(sndbuf);
    qdata->len = strlen(sndbuf) + 1;
    qdata->address.sin_family = AF_INET;
    qdata->address.sin_port = htons(port);
    qdata->address.sin_addr = inaddrFromHostent(hp);
    AppendUdp(qdata);
    comm_set_select_handler(theOutIcpConnection,
	COMM_SELECT_WRITE,
	(PF) icpUdpReply,
	(void *) qdata);
}
