SUBDIRS = oscarsocket ui . icq aim icons
METASOURCES = AUTO
AM_CPPFLAGS = -I../../libkopete/ui $(KOPETE_INCLUDES) \
	-I$(srcdir)/oscarsocket \
	-I$(srcdir)/ui \
	$(all_includes)

lib_LTLIBRARIES = kopete_oscar.la

kopete_oscar_la_SOURCES = oscaraccount.cpp oscarpreferences.cpp oscarprefswidget.ui oscarcontact.cpp

kopete_oscar_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_oscar_la_LIBADD  = ./oscarsocket/libkopeteoscarsocket.la \
    ./ui/libkopeteoscarui.la $(top_builddir)/kopete/libkopete/libkopete.la

