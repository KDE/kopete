kde4_header()

add_subdirectory( libmimic ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#SUBDIRS = libmimic
#AM_CPPFLAGS =  -I$(srcdir)/libmimic \
#	$(KOPETE_INCLUDES) \
#	$(all_includes) \
#	$(GLIB_CFLAGS)
#
#noinst_LTLIBRARIES = libmimicwrapper.la
#
#libmimicwrapper_la_SOURCES = mimicwrapper.cpp msnwebcamdialog.cpp
#libmimicwrapper_la_LIBADD = ./libmimic/libmimic.la
#libmimicwrapper_la_LDFLAGS = -no-undefined  $(GLIB_LIBS) $(all_libraries)
#
#
