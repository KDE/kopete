INCLUDES = $(all_includes) -I$(top_srcdir)/kopete/library
SUBDIRS = library protocols

lib_LTLIBRARIES = libkopetemain.la
libkopetemain_la_SOURCES = main.cpp
libkopetemain_la_LDFLAGS = -lX11 lpng -lz -lm -ljpeg -lqt -lXext -lkdecore -lkdeui lkfile -lXpm $(top_srcdir)/kopete/library/libkopete.la $(top_srcdir)/kopete/protocols/icq/libicqprotocol.la $(top_srcdir)/kopete/protocols/msn/libmsnprotocol.la

bin_PROGRAMS = kopete
kopete_SOURCES = dummy.cpp
kopete_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_LDADD = $(LIB_KFILE) libkopetemain.la

METASOURCES = AUTO

CLEANFILES = dummy.cpp

dummy.cpp:
	echo > dummy.cpp

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kopete.pot

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Utilities
desktop_DATA = kopete.desktop
