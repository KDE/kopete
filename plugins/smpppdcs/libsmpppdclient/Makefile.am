AM_CPPFLAGS = $(all_includes)

noinst_LTLIBRARIES = libsmpppdclient.la
libsmpppdclient_la_LDFLAGS = -avoid-version $(all_libraries)

noinst_HEADERS = smpppdclient.h smpppdstate.h smpppdready.h smpppdunsettled.h
libsmpppdclient_la_SOURCES = smpppdclient.cpp smpppdstate.cpp smpppdready.cpp \
	smpppdunsettled.cpp

libsmpppdclient_la_LIBADD = -lcrypto
