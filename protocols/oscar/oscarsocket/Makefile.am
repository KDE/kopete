METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(top_srcdir)/kopete/protocols/oscar -I$(top_builddir)/kopete/protocols/oscar \
	-I$(top_srcdir)/kopete/protocols/oscar/oscarsocket $(all_includes)

noinst_LTLIBRARIES = liboscarsocket.la

liboscarsocket_la_SOURCES = oscarsocket.cpp oscarsocket.icq.cpp oscarsocket.aim.cpp \
	oscarcaps.cpp oncomingsocket.cpp buffer.cpp md5.c aim.cpp ssidata.cpp \
	oscarconnection.cpp oscardirectconnection.cpp \
	oscarfilesendconnection.cpp rateclass.cpp oscarmessage.cpp rtf.cc
