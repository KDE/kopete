noinst_LTLIBRARIES = libkirc.la
libkirc_la_SOURCES = kirc.cpp dcchandler.cpp
libkirc_la_LIBADD   = $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET)
libkirc_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES= $(all_includes)
METASOURCES = AUTO

#Generates some warnings!!!
KDE_ICON = AUTO

#messages:
#	 $(XGETTEXT) -ki18n -C $(kmerlin_SOURCES)  -o $(top_srcdir)/po/kmerlin.pot

