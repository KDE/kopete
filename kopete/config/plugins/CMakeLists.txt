kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#noinst_LTLIBRARIES = libkopetepluginconfig.la
#
#libkopetepluginconfig_la_SOURCES = kopetepluginconfig.cpp
#libkopetepluginconfig_la_LDFLAGS = $(all_libraries)
#libkopetepluginconfig_la_LIBADD = $(LIB_KUTILS)
#
## vim: set noet:
#
