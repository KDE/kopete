kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkopeteyahooui.la
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#		-I$(srcdir)/.. \
#		$(all_includes)
#
#
#libkopeteyahooui_la_SOURCES = yahooadd.ui yahooeditaccountbase.ui \
#	yahooinvitelistbase.ui yahooinvitelistimpl.cpp empty.cpp yahooverifyaccountbase.ui \
#	yahoostealthsetting.ui yahoowebcamdialog.cpp yahoogeneralinfowidget.ui yahoouserinfodialog.cpp \
#	yahooworkinfowidget.ui yahoootherinfowidget.ui
#EXTRA_DIST = dlgrename.ui
#noinst_HEADERS = yahoouserinfodialog.h
