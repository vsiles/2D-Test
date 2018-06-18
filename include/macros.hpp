#ifndef MACRO_H_INCLUDED
#define MACRO_H_INCLUDED

#ifdef __GNUC__
#  include <features.h>
#  if __GNUC_PREREQ(7,0)
#  define SUPPRESS_FALLTHROUGH  __attribute__((fallthrough))
#  else
#  define SUPPRESS_FALLTHROUGH  /* fall through */
#  endif
#else
#  define SUPPRESS_FALLTHROUGH  __attribute__((fallthrough))
#endif /* __GNUC__ */

#endif /* MACRO_H_INCLUDED */
