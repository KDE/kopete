SUBDIRS = oscarsocket . icq aim icons pics
METASOURCES = AUTO
AM_CPPFLAGS = -I../../libkopete/ui $(KOPETE_INCLUDES) \
	-I$(srcdir)/oscarsocket \
        -I./ui \
	-I$(srcdir)/ui \
	$(all_includes)

lib_LTLIBRARIES = libkopete_oscar.la

libkopete_oscar_la_SOURCES = oscaraccount.cpp oscarcontact.cpp

libkopete_oscar_la_LDFLAGS = -no-undefined $(all_libraries)
libkopete_oscar_la_LIBADD  = ./oscarsocket/libkopeteoscarsocket.la \
	 $(top_builddir)/kopete/libkopete/libkopete.la

