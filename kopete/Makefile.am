INCLUDES = $(all_includes) -I$(top_srcdir)/kopete/library -I$(top_srcdir)/kopete/library/ui -I$(top_srcdir)/kopete/protocols/icq -I$(top_srcdir)/kopete/protocols/icq/kxengine
SUBDIRS = library protocols pics

KDE_ICON = AUTO

lib_LTLIBRARIES = libkopetemain.la
libkopetemain_la_SOURCES = main.cpp
libkopetemain_la_LDFLAGS = $(all_libraries) $(LIB_KDEUI) $(KDE_RPATH)
libkopetemain_la_LIBADD = $(top_builddir)/kopete/library/libkopete.la

bin_PROGRAMS = kopete
kopete_SOURCES = dummy.cpp
kopete_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_LDADD = libkopetemain.la
METASOURCES = AUTO

CLEANFILES = dummy.cpp

icondir = $(kde_datadir)/kopete/pics
icon_DATA =

dummy.cpp:
	echo > dummy.cpp

data_DATA = kopeteui.rc
datadir = $(kde_datadir)/kopete

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop

# don't ask me which does what, the last one seems to work for me, mETz

#messages: rc.cpp
#	$(XGETTEXT) -ki18n -kI18N_NOOP *.cpp -o $(podir)/kopete.pot

#messages:
#	$(EXTRACTRC) `find . -name "*.rc"` > rc.cpp
#	$(XGETTEXT) rc.cpp `find . -name "*.cc" -o -name "*.cpp" -print` -o $(podir)/kopete.pot

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) -C -ki18n -kI18N_NOOP $$LIST -o ../po/kopete.pot; \
	fi

