
/* $Id: ftp.h,v 1.6 1996/05/01 22:38:55 wessels Exp $ */

extern int ftpStart _PARAMS((int unusedfd, char *url, StoreEntry * entry));
extern int ftpInitialize _PARAMS((void));
extern int ftpCachable _PARAMS((char *));
extern void ftpServerClose _PARAMS((void));
