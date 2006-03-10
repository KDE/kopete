kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#libcricketxmllite_la_SOURCES = qname.cc \
#			       xmlbuilder.cc \
#			       xmlconstants.cc \
# 			       xmlelement.cc \
#			       xmlnsstack.cc \
#			       xmlparser.cc \
#			       xmlprinter.cc
#
#noinst_HEADERS =               qname.h \
#			       xmlbuilder.h \
#			       xmlconstants.h \
#			       xmlelement.h \
#			       xmlnsstack.h \
#			       xmlparser.h \
#			       xmlprinter.h
#AM_CPPFLAGS = -DPOSIX -I$(srcdir)/../..
#
#noinst_LTLIBRARIES = libcricketxmllite.la
