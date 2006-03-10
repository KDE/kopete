kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
#	$(KOPETE_INCLUDES) $(all_includes)
#
#noinst_LTLIBRARIES = libkopeteprivate.la
#
#libkopeteprivate_la_SOURCES = kopeteemoticons.cpp \
#	kopeteviewmanager.cpp kopeteutils_private.cpp
#libkopeteprivate_la_LDFLAGS = $(all_libraries)
#libkopeteprivate_la_LIBADD = $(LIB_KDEUI)
## vim: set noet:
#
