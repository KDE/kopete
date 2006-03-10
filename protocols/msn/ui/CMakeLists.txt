kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#    -I$(srcdir)/.. \
#    $(all_includes)
#
#noinst_LTLIBRARIES = libkopetemsnui.la
#
#libkopetemsnui_la_SOURCES = msnadd.ui msndebugrawcommand_base.ui msninfo.ui \
#	msneditaccountui.ui msneditaccountwidget.cpp
#
#EXTRA_DIST = msnadd.ui msninfo.ui
#
