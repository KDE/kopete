noinst_LTLIBRARIES = libyahoo2.la

EXTRA_DIST = yahoo2.h yahoo2_types.h md5.h yahoo2_callbacks.h yahoo_connections.h \
	yahoo_httplib.h sha1.h yahoo_fn.h

libyahoo2_la_SOURCES = libyahoo2.c crypt.c md5.c \
        yahoo_httplib.c yahoo_util.c yahoo_list.c sha1.c yahoo_fn.c

libyahoo2_la_LDFLAGS = -module

libyahoo2_la_CFLAGS = -fno-inline
