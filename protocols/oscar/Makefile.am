SUBDIRS = oscarsocket . icq aim icons pics
METASOURCES = AUTO
AM_CPPFLAGS = -I../../libkopete/ui \
	-I$(srcdir)/oscarsocket \
	-I./ui -I$(srcdir)/ui \
	 $(KOPETE_INCLUDES) $(all_includes)

lib_LTLIBRARIES = libkopete_oscar.la

libkopete_oscar_la_SOURCES = oscaraccount.cpp oscarcontact.cpp oscarchangestatus.cpp

libkopete_oscar_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_oscar_la_LIBADD  = ./oscarsocket/libkopeteoscarsocket.la \
	 $(top_builddir)/kopete/libkopete/libkopete.la

