METASOURCES = AUTO

noinst_LTLIBRARIES = libkopeteoscarsocket.la

libkopeteoscarsocket_la_SOURCES = oscarsocket.cpp \
	oscarsocket.icq.cpp oscarsocket.aim.cpp \
	oncomingsocket.cpp buffer.cpp \
	md5.c aimbuddylist.cpp \
	aim.cpp ssidata.cpp \
	oscarconnection.cpp oscardirectconnection.cpp \
	oscarfilesendconnection.cpp

libkopeteoscarsocket_la_LIBADD = $(LIB_KIO)

#EXTRA_DIST =

AM_CPPFLAGS = -DDATADIR=\"$(kde_datadir)/kopete\" \
	$(KOPETE_INCLUDES) \
	-I$(top_srcdir)/kopete/protocols/oscar \
	-I$(top_srcdir)/kopete/protocols/oscar/ui \
	-I$(top_builddir)/kopete/protocols/oscar/ui \
	-I$(top_srcdir)/kopete/protocols/oscar/oscarsocket/ \
	$(all_includes)

AM_CFLAGS = -I$(top_srcdir)/kopete/protocols/oscar/oscarsocket/ $(all_includes)
