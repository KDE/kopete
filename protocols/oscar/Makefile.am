SUBDIRS = oscarsocket ui . icq aim icons
METASOURCES = AUTO
AM_CPPFLAGS = -I../../libkopete/ui $(KOPETE_INCLUDES) \
	-I$(srcdir)/oscarsocket \
        -I./ui \
	-I$(srcdir)/ui \
	$(all_includes)

lib_LTLIBRARIES = libkopete_oscar.la

libkopete_oscar_la_SOURCES = oscaraccount.cpp oscarpreferences.cpp oscarprefswidget.ui oscarcontact.cpp

libkopete_oscar_la_LDFLAGS = -no-undefined $(all_libraries)
libkopete_oscar_la_LIBADD  = ./oscarsocket/libkopeteoscarsocket.la \
    ./ui/libkopeteoscarui.la $(top_builddir)/kopete/libkopete/libkopete.la

