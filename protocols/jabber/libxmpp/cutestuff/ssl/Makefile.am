METASOURCES = AUTO

noinst_LTLIBRARIES = libcutestuff_ssl.la
INCLUDES = $(SSL_INCLUDES) $(all_includes)

libcutestuff_ssl_la_LIBADD = $(LIBSSL)
libcutestuff_ssl_la_LDFLAGS = $(SSL_LDFLAGS)

libcutestuff_ssl_la_SOURCES = \
	qssl.cpp
