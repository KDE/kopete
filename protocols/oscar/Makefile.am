SUBDIRS = liboscar . aim icq icons
METASOURCES = AUTO
AM_CPPFLAGS = -I./ui -I$(srcdir)/ui \
	-I./liboscar -I$(srcdir)/liboscar \
	 $(KOPETE_INCLUDES) $(all_includes)

lib_LTLIBRARIES = libkopete_oscar.la

libkopete_oscar_la_SOURCES = oscaraccount.cpp oscarcontact.cpp oscarmyselfcontact.cpp
libkopete_oscar_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_oscar_la_LIBADD  = $(LIB_KIO) $(top_builddir)/kopete/libkopete/libkopete.la ./liboscar/liboscar.la
