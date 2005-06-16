noinst_LTLIBRARIES = libskype.la

AM_CPPFLAGS = $(all_includes) $(KOPETE_INCLUDES) \
		-I$(top_srcdir)/kopete/protocols/skype/ \
		-I$(top_srcdir)/kopete/protocols/skype/libskype/skypedbus
METASOURCES = AUTO

libskype_la_SOURCES = skype.cpp
libskype_la_LDFLAGS = -module
libskype_la_LIBADD = skypedbus/libskypedbus.la

noinst_HEADERS = skype.h

SUBDIRS = skypedbus
mydatadir = $(kdedatadir)/kopete_skype
