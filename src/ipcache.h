/*  $Id: ipcache.h,v 1.2 1996/03/27 01:46:12 wessels Exp $ */

#ifndef _IPCACHE_H_
#define _IPCACHE_H_

/*  typedef  int(*IPH) _PARAMS((int, ipcache_entry *, caddr_t)); */
typedef int (*IPH) _PARAMS((int, struct hostent *, caddr_t));

extern int ipcache_nbgethostbyname _PARAMS((char *, int, IPH, caddr_t));
extern struct hostent *ipcache_gethostbyname _PARAMS((char *));
extern void ipcache_flush _PARAMS((void));
extern void ipcache_init _PARAMS((void));
extern void stat_ipcache_get _PARAMS((StoreEntry *, cacheinfo *));


typedef struct _ipcache_entry {
    /* first two items must be equivalent to hash_link in hash.h */
    char *name;
    struct _ipcache_entry *next;
    long timestamp;
    long lastref;
    long ttl;
    unsigned char addr_count;
    unsigned char alias_count;
    enum {
	CACHED, PENDING, NEGATIVE_CACHED
    } status:3;
    struct hostent entry;
    struct _ip_pending *pending_head;
    struct _ip_pending *pending_tail;
} ipcache_entry;

#endif
