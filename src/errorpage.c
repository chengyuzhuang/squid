/* $Id: errorpage.c,v 1.19 1996/04/11 22:52:27 wessels Exp $ */

/* DEBUG: Section 4             cached_error: Error printing routines */

#include "squid.h"



#define CACHED_ERROR_MSG_P1 "\
<TITLE>ERROR: The requested URL could not be retrieved</TITLE>\n\
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

#define CACHED_ERROR_MSG_P2 "\
<P>The system returned:\n\
<PRE><I>    %s</I></PRE>\n\
"

#define CACHED_ERROR_MSG_P3 "\
<P>This means that:\n\
<PRE>\n\
    %s\n\
</PRE>\n\
<P> <HR>\n\
<ADDRESS>\n\
Generated by cached/%s@%s\n\
</ADDRESS>\n\
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
    {"ERR_URL_BLOCKED",
	"Access Denied",
	"You are not allowed to access this URL."},
    {"ERR_ZERO_SIZE_OBJECT",
	"No Object Data",
	"The remote server closed the connection before sending any data."},
    {"ERR_MAX"
	"",
	""}
};

/* GLOBAL */
char *tmp_error_buf;

/* LOCAL */
static char *tbuf;

int log_errors = 1;

void errorInitialize()
{
    tmp_error_buf = (char *) xmalloc(MAX_URL * 4);
    tbuf = (char *) xmalloc(MAX_URL * 3);
}


void cached_error_entry(entry, type, msg)
     StoreEntry *entry;
     int type;
     char *msg;
{
    int index;
    if (type < ERR_MIN || type > ERR_MAX)
	fatal_dump("cached_error_entry: type out of range.");
    index = (int) (type - ERR_MIN);
    sprintf(tmp_error_buf, CACHED_ERROR_MSG_P1,
	entry->url,
	entry->url,
	ErrorData[index].shrt);
    if (msg) {
	sprintf(tbuf, CACHED_ERROR_MSG_P2, msg);
	strcat(tmp_error_buf, tbuf);
    }
    sprintf(tbuf, CACHED_ERROR_MSG_P3,
	ErrorData[index].lng,
	version_string,
	getMyHostname());
    strcat(tmp_error_buf, tbuf);
    entry->mem_obj->abort_code = type;
    if (entry->mem_obj->reply->code == 0)
	entry->mem_obj->reply->code = 400;
    storeAbort(entry, tmp_error_buf);
}



char *cached_error_url(url, method, type, address, code, msg)
     char *url;
     int method;
     int type;
     char *address;
     int code;
     char *msg;
{
    int index;

    *tmp_error_buf = '\0';
    if (type == ERR_MIN || type > ERR_MAX)
	fatal_dump("cached_error_url: type out of range.");
    index = (int) (type - ERR_MIN);
    sprintf(tmp_error_buf, "HTTP/1.0 %d Cache Detected Error\r\nContent-type: text/html\r\n\r\n", code);
    sprintf(tbuf, CACHED_ERROR_MSG_P1,
	url,
	url,
	ErrorData[index].shrt);
    strcat(tmp_error_buf, tbuf);
    if (msg) {
	sprintf(tbuf, CACHED_ERROR_MSG_P2, msg);
	strcat(tmp_error_buf, tbuf);
    }
    sprintf(tbuf, CACHED_ERROR_MSG_P3,
	ErrorData[index].lng,
	version_string,
	getMyHostname());
    if (!log_errors)
	return tmp_error_buf;
    return tmp_error_buf;
}


#define CACHED_REQUEST_ERROR_MSG "\
<TITLE>ERROR: Invalid HTTP Request</TITLE>\n\
<H2>Invalid HTTP Request</H2>\n\
<HR>\n\
<PRE>\n\
%s\n\
</PRE>\n\
<HR>\n\
<ADDRESS>\n\
Generated by cached/%s@%s\n\
</ADDRESS>\n\
\n"

char *cached_error_request(request, type, address, code)
     char *request;
     int type;
     char *address;
     int code;
{
    int index;

    *tmp_error_buf = '\0';
    if (type == ERR_MIN || type > ERR_MAX)
	fatal_dump("cached_error_url: type out of range.");
    index = (int) (type - ERR_MIN);

    sprintf(tmp_error_buf, "HTTP/1.0 %d Cache Detected Error\r\nContent-type: text/html\r\n\r\n", code);
    sprintf(tbuf, CACHED_REQUEST_ERROR_MSG,
	request,
	version_string,
	getMyHostname());
    strcat(tmp_error_buf, tbuf);
    if (!log_errors)
	return tmp_error_buf;
    return tmp_error_buf;
}

char *access_denied_msg(code, method, url, client)
     int code;
     int method;
     char *url;
     char *client;
{
    sprintf(tmp_error_buf, "\
HTTP/1.0 %d Cache Access Denied\r\n\
Content-type: text/html\r\n\
\r\n\
<TITLE>Cache Access Denied</TITLE>\n\
<H2>Access Denied</H2>\n\
<P>\n\
Sorry, you are not currently allowed to request\n\
<PRE>    %s</PRE>\n\
From this cache.  Please check with the cache administrator if you\n\
believe this is incorrect.\n\
<HR>\n\
<ADDRESS>\n\
Generated by cached/%s@%s\n\
</ADDRESS>\n\
\n",
	code,
	url,
	version_string,
	getMyHostname());
    return tmp_error_buf;
}
