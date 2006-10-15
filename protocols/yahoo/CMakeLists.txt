
add_subdirectory( libkyahoo ) 
#add_subdirectory( ui ) 
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui 
${CMAKE_CURRENT_SOURCE_DIR}/libkyahoo 
)

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libkyahoo )

########### next target ###############

set(kopete_yahoo_ui_SRCS 
ui/yahooinvitelistimpl.cpp 
ui/yahoowebcamdialog.cpp 
ui/yahoouserinfodialog.cpp 
ui/yahoochatselectorwidget.cpp
)

kde4_add_ui_files(kopete_yahoo_ui_SRCS 
ui/yahooadd.ui 
ui/yahooinvitelistbase.ui 
ui/yahooverifyaccountbase.ui 
ui/yahoostealthsetting.ui 
ui/yahoogeneralinfowidget.ui 
ui/yahooworkinfowidget.ui 
ui/yahoootherinfowidget.ui 
ui/yahooeditaccountbase.ui
ui/yahoochatselectorwidgetbase.ui
)

set(kopete_yahoo_PART_SRCS 
   ${kopete_yahoo_ui_SRCS} 
   yahooprotocol.cpp 
   yahoocontact.cpp 
   yahooaddcontact.cpp 
   yahooaccount.cpp 
   yahooeditaccount.cpp 
   yahooconferencemessagemanager.cpp 
   yahoochatchatsession.cpp
   yahoochatsession.cpp 
   yahooverifyaccount.cpp 
   yahoowebcam.cpp )

kde4_automoc(kopete_yahoo ${kopete_yahoo_PART_SRCS})

kde4_add_plugin(kopete_yahoo ${kopete_yahoo_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_yahoo )

target_link_libraries(kopete_yahoo ${KDE4_KDEUI_LIBS} kopete_videodevice kopete kyahoo)

install(TARGETS kopete_yahoo  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_yahoo.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES yahooconferenceui.rc yahooimui.rc yahoochatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_yahoo)




#original Makefile.am contents follow:

#SUBDIRS = libkyahoo ui icons
#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(srcdir)/ui \
#	-Iui \
#	-I$(srcdir)/libkyahoo \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kopete_yahoo.la
#
#kopete_yahoo_la_SOURCES = yahooprotocol.cpp yahoocontact.cpp \
#	yahooaddcontact.cpp yahooaccount.cpp yahooeditaccount.cpp yahooconferencemessagemanager.cpp \
#	yahoochatsession.cpp yahooverifyaccount.cpp yahoowebcam.cpp
#kopete_yahoo_la_LDFLAGS     = -module $(KDE_PLUGIN)
#kopete_yahoo_la_LIBADD = ../../libkopete/avdevice/libkopete_videodevice.la \
#	$(top_builddir)/kopete/libkopete/libkopete.la ui/libkopeteyahooui.la libkyahoo/libkyahoo.la
#
#service_DATA = kopete_yahoo.desktop
#servicedir = $(kde_servicesdir)
#
#
#mydatadir = $(kde_datadir)/kopete_yahoo
#mydata_DATA = yahooconferenceui.rc yahoochatui.rc
#
#
