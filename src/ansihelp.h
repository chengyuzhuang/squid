
/* $Id: ansihelp.h,v 1.2 1996/03/27 01:45:52 wessels Exp $ */

#ifndef _ANSIHELP_H_
#define _ANSIHELP_H_

/* functions available in proto library */
#ifndef _PARAMS
#if defined(__STDC__) || defined(__cplusplus) || defined(__STRICT_ANSI__)
#define _PARAMS(ARGS) ARGS
#else /* Traditional C */
#define _PARAMS(ARGS) ()
#endif /* __STDC__ */
#endif /* _PARAMS */

#endif /* _ANSIHELP_H_ */
