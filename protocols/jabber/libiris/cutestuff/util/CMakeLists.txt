kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libcutestuff_util.la
#INCLUDES = $(all_includes)
#
#libcutestuff_util_la_LDFLAGS = $(all_libraries) -no-undefined
#libcutestuff_util_la_SOURCES = base64.cpp bytestream.cpp safedelete.cpp sha1.cpp
#
