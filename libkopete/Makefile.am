SUBDIRS = ui
INCLUDES = -I$(srcdir)/ui $(all_includes)
lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES =	kopete.cpp kopeteprotocol.cpp kopetecontact.cpp \
						pluginloader.cpp plugin.cpp \
						messagelog.cpp kopetemessage.cpp kopetenotifier.cpp\
						kopeteevent.cpp
libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = $(srcdir)/ui/libkopeteui.la $(LIB_KDEUI)
libkopete_la_METASOURCES = AUTO
