kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(top_srcdir)/kopete/libkopete/private \
#	-I$(top_srcdir)/kopete/libkopete/ui \
#	-I$(top_srcdir)/kopete/kopete \
#	-I$(top_srcdir)/kopete/kopete/chatwindow \
#	-I$(top_srcdir)/kopete/kopete/addcontactwizard \
#	-I$(top_builddir)/kopete/kopete/addcontactwizard \
#	-I$(top_srcdir)/kopete/kopete/config/knotify \
#	$(all_includes)
#
#noinst_LTLIBRARIES = libkopetecontactlist.la
#
#libkopetecontactlist_la_SOURCES = kopetemetacontactlvi.cpp \
#	kopetestatusgroupviewitem.cpp kopetegroupviewitem.cpp kopetecontactlistview.cpp \
#	kopetegvipropswidget.ui kopetemetalvipropswidget.ui kopetelviprops.cpp \
#	kopeteaddrbookexport.cpp kopeteaddrbookexportui.ui  \
#	customnotificationprops.cpp kopetegrouplistaction.cpp kabcexport.cpp \
#	kabcexport_base.ui 
#
#libkopetecontactlist_la_LDFLAGS = $(all_libraries)
#libkopetecontactlist_la_LIBADD = -lkabc ../../libkopete/libkopete.la ../addcontactwizard/libkopeteaddcontactwizard.la $(LIB_KDEUI) $(LIB_XRENDER) 
#
#noinst_HEADERS = kopeteaddrbookexport.h customnotificationprops.h kabcexport.h 
#
#KDE_OPTIONS = nofinal
#
## vim: set noet:
