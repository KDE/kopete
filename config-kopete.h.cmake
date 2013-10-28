
/* Define if you have valgrind.h installed */
#cmakedefine HAVE_VALGRIND_H 1

/* Define if you have xmms libraries and header files. */
#cmakedefine HAVE_XMMS 1

/* Define to compile with GSM SMS support */
/* #undef INCLUDE_SMSGSM */

/* Glib is required for oRTP code and libmimic code */
#define HAVE_GLIB 1

/* TODO */
/* #undef HAVE_XSHM */

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#cmakedefine WORDS_BIGENDIAN ${CMAKE_WORDS_BIGENDIAN}

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG ${SIZEOF_LONG}

/* The size of a `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG ${SIZEOF_UNSIGNED_LONG}

/* Define to 1 if you want libv4l support */
#cmakedefine HAVE_LIBV4L2 1

#cmakedefine HAVE_V4L 1
