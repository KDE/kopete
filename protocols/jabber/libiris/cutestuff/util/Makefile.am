METASOURCES = AUTO

noinst_LTLIBRARIES = libcutestuff_util.la
INCLUDES = $(SSL_INCLUDES) $(all_includes)

libcutestuff_util_la_LIBADD = $(LIBSSL)
libcutestuff_util_la_LDFLAGS = $(SSL_LDFLAGS)

libcutestuff_util_la_SOURCES = \
	base64.cpp \
	bytestream.cpp \
	cipher.cpp \
	qrandom.cpp \
	safedelete.cpp \
	sha1.cpp \
	showtextdlg.cpp
