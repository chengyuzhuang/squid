
/*
 * $Id: errorpage.c,v 1.38 1996/09/14 08:45:52 wessels Exp $
 *
 * DEBUG: section 4     Error Generation
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

#define SQUID_ERROR_MSG_P1 "\
<HTML><HEAD>\n\
<TITLE>ERROR: The requested URL could not be retrieved</TITLE>\n\
</HEAD><BODY>\n\
<H1>ERROR</H1>\n\
<H2>The requested URL could not be retrieved</H2>\n\
<HR>\n\
<P>\n\
While trying to retrieve the URL:\n\
<A HREF=\"%s\">%s</A>\n\
<P>\n\
The following error was encountered:\n\
<UL>\n\
<LI><STRONG>%s</STRONG>\n\
</UL>\n\
"

#define SQUID_ERROR_MSG_P2 "\
<P>The system returned:\n\
<PRE><I>    %s</I></PRE>\n\
"

#define SQUID_ERROR_MSG_P3 "\
<P>This means that:\n\
<PRE>\n\
    %s\n\
</PRE>\n\
<P>\n\
%s\n\
<HR>\n\
<ADDRESS>\n\
Generated by %s/%s@%s\n\
</ADDRESS></BODY></HTML>\n\
\n"

typedef struct {
    char *tag;
    char *shrt;
    char *lng;
} error_data;

error_data ErrorData[] =
{
    {"ERR_READ_TIMEOUT",
	"Read Timeout",
	"The remote site or network may be down.  Please try again."},
    {"ERR_LIFETIME_EXP",
	"Transaction Timeout",
	"The network or remote site may be down or too slow.  Try again later."},
    {"ERR_NO_CLIENTS_BIG_OBJ",
	"No Client",
	"All Clients went away before tranmission completed and the object is too big to cache."},
    {"ERR_READ_ERROR",
	"Read Error",
	"The remote site or network may be down.  Please try again."},
    {"ERR_CLIENT_ABORT",
	"Client Aborted",
	"Client(s) dropped connection before transmission completed.\nObject fetching is aborted.",},
    {"ERR_CONNECT_FAIL",
	"Connection Failed",
	"The remote site or server may be down.  Please try again soon."},
    {"ERR_INVALID_REQUEST",
	"Invalid HTTP request",
	"Please double check it, or ask for assistance."},
    {"ERR_UNSUP_REQUEST",
	"Unsupported request",
	"This request method is not supported for this protocol."},
    {"ERR_INVALID_URL",
	"Invalid URL syntax",
	"Please double check it, or ask for assistance."},
    {"ERR_NO_FDS",
	"Out of file descriptors",
	"The cache is currently very busy.  Please try again."},
    {"ERR_DNS_FAIL",
	"DNS name lookup failure",
	"The named host probably does not exist."},
    {"ERR_NOT_IMPLEMENTED",
	"Protocol Not Supported",
	"The cache does not know about the requested protocol."},
    {"ERR_CANNOT_FETCH",
	"Unable to Retrieve",
	"The requested URL can not currently be retrieved."},
    {"ERR_NO_RELAY",
	"No WAIS Relay",
	"There is no WAIS relay host defined for this cache."},
    {"ERR_DISK_IO",
	"Cache Disk I/O Failure",
	"The system disk is out of space or failing."},
    {"ERR_ZERO_SIZE_OBJECT",
	"No Object Data",
	"The remote server closed the connection before sending any data."},
    {"ERR_PROXY_DENIED",
	"Access Denied",
	"You must authenticate yourself before accessing this cache."}
};

/* GLOBAL */
char *tmp_error_buf;

/* LOCAL */
static char *tbuf = NULL;
static char *auth_msg = NULL;

void
errorInitialize()
{
#ifndef USE_PROXY_AUTH
    tmp_error_buf = xmalloc(MAX_URL * 4);
#else
    tmp_error_buf = xmalloc(8192);
#endif /* USE_PROXY_AUTH */
    meta_data.misc += MAX_URL * 4;
    tbuf = xmalloc(MAX_URL * 3);
    meta_data.misc += MAX_URL * 3;
    auth_msg = xmalloc(MAX_URL * 3);
    meta_data.misc += MAX_URL * 3;
}

void
squid_error_entry(StoreEntry * entry, log_type type, char *msg)
{
    int index;

    if (type < ERR_MIN || type > ERR_MAX)
	fatal_dump("squid_error_entry: type out of range.");
    index = (int) (type - ERR_MIN);
    debug(4, 1, "%s: %s\n", ErrorData[index].tag, entry->url);
    sprintf(tmp_error_buf, SQUID_ERROR_MSG_P1,
	entry->url,
	entry->url,
	ErrorData[index].shrt);
    if (msg) {
	sprintf(tbuf, SQUID_ERROR_MSG_P2, msg);
	strcat(tmp_error_buf, tbuf);
    }
    sprintf(tbuf, SQUID_ERROR_MSG_P3,
	ErrorData[index].lng,
	Config.errHtmlText,
	appname,
	version_string,
	getMyHostname());
    strcat(tmp_error_buf, tbuf);
    if (entry->mem_obj) {
	entry->mem_obj->abort_code = type;
	if (entry->mem_obj->reply->code == 0)
	    entry->mem_obj->reply->code = 400;
    }
    storeAbort(entry, tmp_error_buf);
}



char *
squid_error_url(char *url, int method, int type, char *address, int code, char *msg)
{
    int index;

    *tmp_error_buf = '\0';
    if (type < ERR_MIN || type > ERR_MAX)
	fatal_dump("squid_error_url: type out of range.");
    index = (int) (type - ERR_MIN);
    debug(4, 1, "%s: %s\n", ErrorData[index].tag, url);
    sprintf(tmp_error_buf, "HTTP/1.0 %d Cache Detected Error\r\nContent-type: text/html\r\n\r\n", code);
    sprintf(tbuf, SQUID_ERROR_MSG_P1,
	url,
	url,
	ErrorData[index].shrt);
    strcat(tmp_error_buf, tbuf);
    if (msg) {
	sprintf(tbuf, SQUID_ERROR_MSG_P2, msg);
	strcat(tmp_error_buf, tbuf);
    }
    sprintf(tbuf, SQUID_ERROR_MSG_P3,
	ErrorData[index].lng,
	Config.errHtmlText,
	appname,
	version_string,
	getMyHostname());
    strcat(tmp_error_buf, tbuf);
    return tmp_error_buf;
}


#define SQUID_REQUEST_ERROR_MSG "\
<HTML><HEAD><TITLE>ERROR: Invalid HTTP Request</TITLE></HEAD>\n\
<BODY><H1>ERROR</H1>\n\
<H2>Invalid HTTP Request</H2>\n\
<HR>\n\
<PRE>\n\
%s\n\
</PRE>\n\
<P>\n\
%s\n\
<HR>\n\
<ADDRESS>\n\
Generated by %s/%s@%s\n\
</ADDRESS></BODY></HTML>\n\
\n"

char *
squid_error_request(char *request, int type, char *address, int code)
{
    int index;

    *tmp_error_buf = '\0';
    if (type < ERR_MIN || type > ERR_MAX)
	fatal_dump("squid_error_request: type out of range.");
    index = (int) (type - ERR_MIN);
    debug(4, 1, "%s: %s\n", ErrorData[index].tag, request);
    sprintf(tmp_error_buf, "HTTP/1.0 %d Cache Detected Error\r\nContent-type: text/html\r\n\r\n", code);
    sprintf(tbuf, SQUID_REQUEST_ERROR_MSG,
	request,
	Config.errHtmlText,
	appname,
	version_string,
	getMyHostname());
    strcat(tmp_error_buf, tbuf);
    return tmp_error_buf;
}

char *
access_denied_msg(int code, int method, char *url, char *client)
{
    sprintf(tmp_error_buf, "\
HTTP/1.0 %d Cache Access Denied\r\n\
Content-type: text/html\r\n\
\r\n\
<HTML><HEAD><TITLE>Cache Access Denied</TITLE></HEAD>\n\
<BODY><H1>Error</H1>\n\
<H2>Access Denied</H2>\n\
<P>\n\
Sorry, you are not currently allowed to request\n\
<PRE>    %s</PRE>\n\
From this cache.  Please check with the\n\
<A HREF=\"mailto:%s\">cache administrator</A>\n\
if you believe this is incorrect.\n\
<P>\n\
%s\n\
<HR>\n\
<ADDRESS>\n\
Generated by %s/%s@%s\n\
</ADDRESS>\n\
\n",
	code,
	url,
	Config.adminEmail,
	Config.errHtmlText,
	appname,
	version_string,
	getMyHostname());
    return tmp_error_buf;
}

/* maex@space.net (06.09.1996)
 *    the message that is sent on deny_info
 *      add a Location: and for old browsers a HREF to the info page
 */
char *
access_denied_redirect(int code, int method, char *url, char *client, char *redirect)
{
    sprintf(tmp_error_buf, "\
HTTP/1.0 %d Cache Access Deny Redirect\r\n\
Location: %s\r\n\
Content-type: text/html\r\n\
\r\n\
<HTML><HEAD><TITLE>Cache Access Denied</TITLE></HEAD>\n\
<BODY><H1>Error</H1>\n\
<H2>Access Denied</H2>\n\
<P>\n\
Sorry, you are not currently allowed to request\n\
<PRE>    %s</PRE>\n\
from this cache.\n\
<P>\n\
You may take a look at\n\
<PRE> <A HREF=\"%s\">%s</A></PRE>\n\
or check with the cache administrator if you\n\
believe this is incorrect.\n\
<P>\n\
%s\n\
<HR>\n\
<ADDRESS>\n\
Generated by %s/%s@%s\n\
</ADDRESS></BODY></HTML>\n\
\n",
	code,
	redirect,
	url,
	redirect,
	redirect,
	Config.errHtmlText,
	appname,
	version_string,
	getMyHostname());
    return tmp_error_buf;
}

char *
authorization_needed_msg(request_t * request, char *realm)
{
    sprintf(auth_msg, "<HTML><HEAD><TITLE>Authorization needed</TITLE>\n\
</HEAD><BODY><H1>Authorization needed</H1>\n\
<P>Sorry, you have to authorize yourself to request\n\
<PRE>    ftp://%s@%s%s</PRE>\n\
<P>from this cache.  Please check with the\n\
<A HREF=\"mailto:%s\">cache administrator</A>\n\
if you believe this is incorrect.\n\
<P>\n\
%s\n\
<HR>\n\
<ADDRESS>\n\
Generated by %s/%s@%s\n\
</ADDRESS></BODY></HTML>\n\
\n",
	request->login,
	request->host,
	request->urlpath,
	Config.adminEmail,
	Config.errHtmlText,
	appname,
	version_string,
	getMyHostname());

    mk_mime_hdr(tbuf,
	"text/html",
	strlen(auth_msg),
	squid_curtime,
	squid_curtime + Config.negativeTtl);
    sprintf(tmp_error_buf, "HTTP/1.0 401 Unauthorized\r\n\
%s\
WWW-Authenticate: Basic realm=\"%s\"\r\n\
\r\n\
%s",
	tbuf, realm, auth_msg);
    return tmp_error_buf;
}


#define PROXY_AUTH_ERR_MSG "\
HTTP/1.0 %d Cache Access Denied\r\n\
Proxy-Authenticate: Basic realm=\"Squid proxy-caching web server\"\r\n\
Content-type: text/html\r\n\
\r\n\
<TITLE>Cache Access Denied</TITLE>\n\
<H2>Cache Access Denied</H2>\n\
<P>\n\
Sorry, you are not currently allowed to request\n\
<PRE>    %s</PRE>\n\
from this cache until you have authenticated yourself.\n\
\n<p>\
You need to use Netscape version 2.0 or greater, or Microsoft Internet Explorer 3.0\n\
or an HTTP/1.1 compliant browser for this to work.\n\
Please contact the <a href=\"mailto:%s\">cache administrator</a>\n\
if you have difficulties authenticating yourself, or\n\
<a href=\"http://%s/cgi-bin/chpasswd.cgi\">change</a>\n\
your default password.\n\
<P>\n\
%s\n\
<HR>\n\
<ADDRESS>\n\
Generated by %s/%s@%s\n\
</ADDRESS>\n\
"

char *
proxy_denied_msg(int code, int method, char *url, char *client)
{
    sprintf(tmp_error_buf, PROXY_AUTH_ERR_MSG,
	code,
	url,
	Config.adminEmail,
	getMyHostname(),
	Config.errHtmlText,
	appname,
	version_string,
	getMyHostname());
    return tmp_error_buf;
}
