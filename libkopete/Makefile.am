SUBDIRS = ui
INCLUDES = -I$(srcdir)/ui -I./ui $(all_includes)
noinst_LTLIBRARIES = libkopete.la
#lib_LTLIBRARIES = libkopete.la

# kopete_include_HEADERS = imcontact.h  improtocol.h kopete.h

libkopete_la_SOURCES = kopete.cpp improtocol.cpp imcontact.cpp \
						pluginloader.cpp plugin.cpp \
						messagelog.cpp kopetemessage.cpp kopetenotifier.cpp\
						kopeteevent.cpp
libkopete_la_METASOURCES = AUTO
libkopete_la_LDFLAGS     = 
libkopete_la_LIBADD      = ui/libkopeteui.la $(LIB_KDECORE)
