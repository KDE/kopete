kde4_header()

#add_subdirectory( addaccountwizard ) 
#add_subdirectory( addcontactwizard ) 
#add_subdirectory( config ) RENABLE
#add_subdirectory( contactlist ) 
#add_subdirectory( chatwindow ) RENABLE
#add_subdirectory( kconf_update ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${CMAKE_SOURCE_DIR}/kopete/libkopete/ 
${CMAKE_BINARY_DIR}/kopete/libkopete/ 
${CMAKE_SOURCE_DIR}/kopete/libkopete/ui/ 
${CMAKE_BINARY_DIR}/kopete/libkopete/ui/ 
${CMAKE_SOURCE_DIR}/kopete/libkopete/private 
${CMAKE_CURRENT_SOURCE_DIR}/addaccountwizard 
${CMAKE_CURRENT_SOURCE_DIR}/contactlist/ 
${CMAKE_CURRENT_BINARY_DIR}/contactlist/ 
${CMAKE_CURRENT_SOURCE_DIR}/config/plugins/ 
)

link_directories( ${CMAKE_BINARY_DIR}/kopete/libkopete/ )

########### next target ###############

# Add account Wizard
set( addaccountwizard_SRCS addaccountwizard/addaccountwizard.cpp )
kde4_add_ui3_files(addaccountwizard_SRCS addaccountwizard/addaccountwizardpage1.ui addaccountwizard/addaccountwizardpage2.ui addaccountwizard/addaccountwizardpage3.ui)

# Contact List
set(kopetecontactlist_SRCS 
contactlist/kopetemetacontactlvi.cpp 
contactlist/kopetestatusgroupviewitem.cpp 
contactlist/kopetegroupviewitem.cpp 
contactlist/kopetecontactlistview.cpp 
contactlist/kopetelviprops.cpp 
contactlist/kopeteaddrbookexport.cpp 
contactlist/customnotificationprops.cpp 
contactlist/kopetegrouplistaction.cpp 
contactlist/kabcexport.cpp 
)
kde4_add_ui3_files(kopetecontactlist_SRCS contactlist/kopetegvipropswidget.ui contactlist/kopetemetalvipropswidget.ui contactlist/kopeteaddrbookexportui.ui contactlist/kabcexport_base.ui)

# Configure plugins
set(kopetepluginconfig_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/config/plugins/kopetepluginconfig.cpp)

set(kopete_SRCS 
   ${addaccountwizard_SRCS}
   ${kopetecontactlist_SRCS}
   ${kopetepluginconfig_SRCS}
   main.cpp 
   kopeteapplication.cpp 
   kopeteiface.cpp 
   systemtray.cpp 
   kopeteballoon.cpp 
   kopetewindow.cpp 
   kopeteaccountstatusbaricon.cpp 
   kimifaceimpl.cpp 
   kopeteeditglobalidentitywidget.cpp )

kde4_automoc(${kopete_SRCS})

kde4_add_ui3_files(kopete_SRCS groupkabcselectorwidget.ui )

kde4_add_dcop_skels(kopete_SRCS kopeteiface.h kimiface.h )

kde4_add_executable(kopete_bin ${kopete_SRCS})
set_target_properties(kopete_bin PROPERTIES OUTPUT_NAME kopete)

target_link_libraries(kopete_bin ${KDE4_KDE3SUPPORT_LIBS} ktexteditor knotifyconfig kopete )

if(X11_Xrender_FOUND)
  target_link_libraries(kopete_bin ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

install_targets(/bin kopete_bin )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kopete.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete FILES kopeteui.rc kopete.notifyrc )
install_files( ${MIME_INSTALL_DIR}/application FILES x-kopete-emoticons.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#kopete_srcdir=$(top_srcdir)/kopete
#kopete_builddir=$(top_builddir)/kopete
#
#INCLUDES = -I$(kopete_srcdir)/kopete
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(kopete_srcdir)/kopete/config \
#	-I$(kopete_srcdir)/kopete/addaccountwizard \
#	-I$(kopete_srcdir)/kopete/addcontactwizard \
#	-I$(kopete_srcdir)/kopete/config/accounts \
#	-I$(kopete_builddir)/kopete/addcontactwizard \
#	-I$(kopete_srcdir)/libkopete/ui \
#	-I$(kopete_srcdir)/kopete/contactlist \
#	-I$(kopete_builddir)/kopete/contactlist \
#	-I$(kopete_srcdir)/kopete/chatwindow \
#	-I$(kopete_srcdir)/kopete/config/plugins \
#	-I$(kopete_builddir)/kopete/config \
#	-I$(kopete_srcdir)/libkopete/private \
#	-I$(kopete_srcdir)/libkopete/avdevice \
#	-I$(kopete_srcdir)/kopete/config/knotify \
#	-I$(kopete_srcdir) \
#	$(all_includes)
#KDE_ICON = AUTO
#METASOURCES = AUTO
#
#SUBDIRS = addaccountwizard addcontactwizard config contactlist chatwindow . kconf_update
#
#bin_PROGRAMS = kopete
#kopete_SOURCES = main.cpp 	kopeteapplication.cpp kopeteiface.cpp \
#		kopeteiface.skel 	systemtray.cpp 	kopeteballoon.cpp kopetewindow.cpp \
#			kopeteaccountstatusbaricon.cpp 	kimifaceimpl.cpp kimiface.skel kopeteeditglobalidentitywidget.cpp \
#	groupkabcselectorwidget.ui
#
#kimiface_DIR = $(kde_includes)
#
#kopete_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH) -lktexteditor -lknotifyconfig
#
#kopete_LDADD = \
#	addcontactwizard/libkopeteaddcontactwizard.la \
#	addaccountwizard/libkopeteaddaccountwizard.la \
#	contactlist/libkopetecontactlist.la \
#	config/plugins/libkopetepluginconfig.la \
#	../libkopete/libkopete.la \
#	../libkopete/ui/libkopeteui.la
#
#mydatadir    = $(kde_datadir)/kopete
#mydata_DATA  = kopeteui.rc kopete.notifyrc
#
#mimedir      = $(kde_mimedir)/application
#mime_DATA    = x-kopete-emoticons.desktop
#
#xdg_apps_DATA = kopete.desktop
#
## vim: set noet:
#noinst_HEADERS = kimifaceimpl.h
