METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(top_srcdir)/kopete/protocols/oscar -I$(top_builddir)/kopete/protocols/oscar \
	-I$(top_srcdir)/kopete/protocols/oscar/oscarsocket $(all_includes)

noinst_LTLIBRARIES = liboscarsocket.la

liboscarsocket_la_SOURCES = oscarconnection.cpp oscarsocket.cpp \
	oscarsocket.icq.cpp oscarsocket.aim.cpp \
	oscar_fam01.cpp oscar_fam04.cpp oscar_fam19.cpp \
	oscarcaps.cpp buffer.cpp md5.c aim.cpp ssidata.cpp \
	rateclass.cpp oscarmessage.cpp rtf.cc

# Unused files
#
# oscarfilesendconnection.cpp
# oscardirectconnection.cpp
# oncomingsocket.cpp