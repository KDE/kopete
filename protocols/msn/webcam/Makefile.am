METASOURCES = AUTO
SUBDIRS = libmimic
AM_CPPFLAGS =  -I$(srcdir)/libmimic  $(all_includes) $(GLIB_CFLAGS)

noinst_LTLIBRARIES = libmimicwrapper.la

libmimicwrapper_la_SOURCES = mimicwrapper.cpp msnwebcamdialog.cpp
libmimicwrapper_la_LIBADD = ./libmimic/libmimic.la
libmimicwrapper_la_LDFLAGS = -no-undefined  -lglib-2.0 $(all_libraries)


