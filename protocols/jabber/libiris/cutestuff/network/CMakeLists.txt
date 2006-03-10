kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#KDE_CXXFLAGS = $(QCA_CFLAGS)
#
#noinst_LTLIBRARIES = libcutestuff_network.la
#INCLUDES = -I$(srcdir)/../util $(all_includes)
#
#libcutestuff_network_la_LDADD = $(QCA_LIBS) -I$(srcdir)/../util/libcutestuff_util.la
#libcutestuff_network_la_LDFLAGS = $(all_libraries) -no-undefined
#libcutestuff_network_la_SOURCES = \
#	bsocket.cpp \
#	httpconnect.cpp \
#	httppoll.cpp \
#	ndns.cpp \
#	servsock.cpp \
#	socks.cpp \
#	srvresolver.cpp
#
#KDE_OPTIONS = nofinal
#
