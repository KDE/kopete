METASOURCES = AUTO
SUBDIRS = libmimic
AM_CPPFLAGS =  -I$(srcdir)/libmimic \
	$(KOPETE_INCLUDES) \
	$(all_includes) \
	$(GLIB_CFLAGS)

noinst_LTLIBRARIES = libmimicwrapper.la

libmimicwrapper_la_SOURCES = mimicwrapper.cpp msnwebcamdialog.cpp
libmimicwrapper_la_LIBADD = ./libmimic/libmimic.la
libmimicwrapper_la_LDFLAGS = -no-undefined  $(GLIB_LIBS) $(all_libraries)


