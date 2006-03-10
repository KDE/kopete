kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libqca.la
#INCLUDES = $(all_includes)
#
#libqca_la_SOURCES = \
#	qca.cpp
#libqca_la_LIBADD = -lqt-mt
