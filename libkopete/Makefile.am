SUBDIRS = ui
INCLUDES = -I$(top_srcdir)/kopete/library -I$(top_srcdir)/kopete/library/ui -I$(top_srcdir)/kopete/protocols/icq -I$(top_srcdir)/kopete/protocols/icq/kxengine $(all_includes)
lib_LTLIBRARIES = libkopete.la
# kopete_include_HEADERS = imcontact.h  improtocol.h kopete.h pluginmanager.h

libkopete_la_SOURCES = kopete.cpp improtocol.cpp imcontact.cpp pluginmanager.cpp pluginloader.cpp plugin.cpp
libkopete_la_METASOURCES = AUTO
libkopete_la_LDFLAGS     =  -lDCOP
libkopete_la_LIBADD      =  $(top_srcdir)/kopete/library/ui/libui.la
