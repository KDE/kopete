INCLUDES = -I$(top_srcdir)/src $(all_includes) -I../libsmpppdclient
METASOURCES = AUTO


check_PROGRAMS = smpppdcstests

smpppdcstests_SOURCES = main.cpp clienttest.cpp
smpppdcstests_LDFLAGS = $(KDE_RPATH) $(all_libraries)
smpppdcstests_LDADD = ../libsmpppdclient/libsmpppdclient.la -lkunittestgui

noinst_HEADERS = clienttest.h

check:
	kunittest ./smpppdcstests ClientTest

