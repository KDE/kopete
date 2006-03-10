kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkopetewpui.la
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(srcdir)/.. \
#	$(all_includes)
#
#
#libkopetewpui_la_SOURCES = wpaddcontactbase.ui wpeditaccountbase.ui empty.cpp wpuserinfowidget.ui
#EXTRA_DIST = wpaddcontactbase.ui wpeditaccountbase.ui
