noinst_LTLIBRARIES = libyahoo2.la
INCLUDES = -I$(top_srcdir) $(GLIB_INCLUDES) -I../libproxy $(all_includes)                       
AM_CFLAGS = $(GLIB_CFLAGS)

EXTRA_DIST = yahoo2.h yahoo2_types.h md5.h yahoo2_callbacks.h yahoo_connections.h \
	yahoo_httplib.h

libyahoo2_la_SOURCES = libyahoo2.c crypt.c md5.c yahoo_connections.c \
	yahoo_httplib.c

libyahoo2_la_LIBADD = $(GLIB_LIBADD)
libyahoo2_la_LDFLAGS = -no-undefined $(all_libraries)

