SUBDIRS = ui
INCLUDES = -I$(srcdir)/ui -I./ui $(all_includes)
lib_LTLIBRARIES = libkopete.la

# kopete_include_HEADERS = kopetecontact.h  improtocol.h kopete.h

libkopete_la_SOURCES = kopete.cpp improtocol.cpp kopetecontact.cpp \
						pluginloader.cpp plugin.cpp \
						messagelog.cpp kopetemessage.cpp kopetenotifier.cpp\
						kopeteevent.cpp
libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = ui/libkopeteui.la $(LIB_KDEUI)
libkopete_la_METASOURCES = AUTO
