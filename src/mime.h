/*  $Id: mime.h,v 1.2 1996/03/27 01:46:14 wessels Exp $ */

#ifndef MIME_H
#define MIME_H

#define MAX_MIME 4096

typedef struct _ext_table_entry {
    char *name;
    char *mime_type;
    char *mime_encoding;
    char *icon;
} ext_table_entry;

extern void mime_process _PARAMS((char *mime));
extern int mime_refresh_request _PARAMS((char *mime));
extern ext_table_entry *mime_ext_to_type _PARAMS((char *extension));
extern int mk_mime_hdr _PARAMS((char *, time_t, int, time_t, char *));

#endif
