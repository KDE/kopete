kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#libcricketp2pclient_la_SOURCES = sessionclient.cc \
#				 basicportallocator.cc \
#				 socketmonitor.cc
#
#noinst_HEADERS =                 basicportallocator.h \
#				 sessionclient.h \
#				 socketmonitor.h 
#
#AM_CPPFLAGS = -I$(srcdir)/../../.. -DLINUX -DPOSIX -DINTERNAL_BUILD
#
#noinst_LTLIBRARIES = libcricketp2pclient.la
