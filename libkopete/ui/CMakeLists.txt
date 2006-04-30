kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} )


########### next target ###############


########### install files ###############

install_files( /include/kopete/ui FILES accountselector.h
${CMAKE_CURRENT_BINARY_DIR}/../ui_fileconfirmbase.h kopetefileconfirmdialog.h
kopetepasswordwidget.h kopeteview.h addcontactpage.h
${CMAKE_CURRENT_BINARY_DIR}/../ui_kopeteawaydialogbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui_kopetepasswordwidgetbase.h kopeteviewplugin.h
editaccountwidget.h kopetecontactaction.h  ${CMAKE_CURRENT_BINARY_DIR}/../ui_kopetepassworddialog.h kopetestdaction.h  	addressbookselectordialog.h addressbookselectorwidget.h kopetelistview.h kopetelistviewitem.h kopetelistviewsearchline.h )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
#	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(all_includes)
#
#noinst_LTLIBRARIES = libkopeteui.la
#
#libkopeteui_la_SOURCES = kopetecontactaction.cpp addcontactpage.cpp \
#		editaccountwidget.cpp kopetepassworddialog.ui kopetestdaction.cpp 	kopeteawaydialogbase.ui \
#	kopetefileconfirmdialog.cpp 	fileconfirmbase.ui kopeteview.cpp \
#	kopetepasswordwidgetbase.ui 	kopetepasswordwidget.cpp accountselector.cpp kopeteviewplugin.cpp \
#	addresseeitem.cpp addressbookselectorwidget_base.ui addressbookselectordialog.cpp \
#		addressbookselectorwidget.cpp metacontactselectorwidget_base.ui 	metacontactselectorwidget.cpp \
#	kopetelistview.cpp kopetelistviewitem.cpp kopetelistviewsearchline.cpp \
#	contactaddednotifywidget.ui contactaddednotifydialog.cpp
#
#libkopeteui_la_LDFLAGS = $(all_libraries) -lkabc
#libkopeteui_la_LIBADD = ../private/libkopeteprivate.la $(LIB_KHTML)
#
#kopeteincludedir = $(includedir)/kopete/ui
#kopeteinclude_HEADERS = accountselector.h fileconfirmbase.h kopetefileconfirmdialog.h \
#	kopetepasswordwidget.h kopeteview.h addcontactpage.h kopeteawaydialogbase.h  \
#	kopetepasswordwidgetbase.h kopeteviewplugin.h \
#	editaccountwidget.h kopetecontactaction.h kopetepassworddialog.h kopetestdaction.h \
#	addressbookselectordialog.h addressbookselectorwidget.h kopetelistview.h kopetelistviewitem.h kopetelistviewsearchline.h
#
#noinst_HEADERS = addresseeitem.h contactaddednotifywidget.h
#
## vim: set noet:
#
