kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libiris.la
#INCLUDES = -Ixmpp-core -Ixmpp-im -I../cutestuff/util -I../cutestuff/network -I../qca/src $(all_includes)
#
#libiris_la_LDFLAGS = $(all_libraries) -no-undefined
#libiris_la_SOURCES = empty.cpp
