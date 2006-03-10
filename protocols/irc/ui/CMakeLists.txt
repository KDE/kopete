kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkopeteircui.la
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(srcdir)/..\
#	-I$(srcdir)/../libkirc \
#	$(all_includes)
#
#
#libkopeteircui_la_SOURCES = ircadd.ui empty.cpp irceditaccountwidget.cpp \
#	irceditaccount.ui channellist.cpp channellistdialog.cpp networkconfig.ui
#EXTRA_DIST = ircadd.ui ircprefs.ui empty.cpp
