METASOURCES = AUTO
AM_CPPFLAGS = $(all_includes) $(KOPETE_INCLUDES) \
	-I$(top_srcdir)/kopete/protocols/oscar -I$(top_builddir)/kopete/protocols/oscar \
	-I$(top_srcdir)/kopete/protocols/oscar/oscarsocket/ \

noinst_LTLIBRARIES = liboscarsocket.la

libliboscarsocket_la_LDFLAGS = -version-info 0:0:0
liboscarsocket_la_SOURCES = oscarsocket.cpp \
	oscarsocket.icq.cpp oscarsocket.aim.cpp \
	oncomingsocket.cpp buffer.cpp \
	md5.c aimbuddylist.cpp \
	aim.cpp ssidata.cpp \
	oscarconnection.cpp oscardirectconnection.cpp \
	oscarfilesendconnection.cpp
