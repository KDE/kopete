SUBDIRS = ui
INCLUDES = -I$(top_srcdir)/kopete/library -I$(top_srcdir)/kopete/library/ui -I$(top_srcdir)/kopete/protocols/icq -I$(top_srcdir)/kopete/protocols/icq/kxengine $(all_includes)
noinst_LTLIBRARIES = libkopete.la
#lib_LTLIBRARIES = libkopete.la

# kopete_include_HEADERS = imcontact.h  improtocol.h kopete.h pluginmanager.h

libkopete_la_SOURCES = kopete.cpp improtocol.cpp imcontact.cpp \
                       pluginmanager.cpp pluginloader.cpp plugin.cpp \
                       messagelog.cpp kopetemessage.cpp
libkopete_la_METASOURCES = AUTO
libkopete_la_LDFLAGS     = 
libkopete_la_LIBADD      = $(top_srcdir)/kopete/library/ui/libkopeteui.la
CXXFLAGS = -g -O2 -Wall
