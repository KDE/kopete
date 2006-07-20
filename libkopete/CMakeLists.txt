
add_subdirectory( private ) 
add_subdirectory( ui ) 
add_subdirectory( avdevice ) 

include_directories( ${KOPETE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/private ${CMAKE_CURRENT_SOURCE_DIR}/ui ${CMAKE_CURRENT_BINARY_DIR}/ui )

if(X11_Xrender_FOUND)
  include_directories(${X11_Xrender_INCLUDE_PATH})
endif(X11_Xrender_FOUND)

########### next target ###############

set(kopete_private_SRCS 
  private/kopeteemoticons.cpp 
  private/kopeteviewmanager.cpp 
  private/kopeteutils_private.cpp
)

set(kopete_ui_SRCS 
  ui/kopetecontactaction.cpp 
  ui/addcontactpage.cpp 
  ui/editaccountwidget.cpp 
  ui/kopetestdaction.cpp 
  ui/kopetefileconfirmdialog.cpp 
  ui/kopeteview.cpp 
  ui/kopetepasswordwidget.cpp 
  ui/accountselector.cpp 
  ui/kopeteviewplugin.cpp 
  ui/addresseeitem.cpp 
  ui/addressbookselectordialog.cpp 
  ui/addressbookselectorwidget.cpp  
  ui/metacontactselectorwidget.cpp 
  ui/kopetelistview.cpp 
  ui/kopetelistviewitem.cpp 
  ui/kopetelistviewsearchline.cpp 
  ui/contactaddednotifydialog.cpp 
  ui/addressbooklinkwidget.cpp
  ui/webcamwidget.cpp 
)

set(kopete_contactliststorage_SRCS
  contactliststorage/xmlcontactstorage.cpp 
)

set(kopete_LIB_SRCS 
   ${kopete_private_SRCS} 
   ${kopete_ui_SRCS} 
   ${kopete_contactliststorage_SRCS}
   kopeteonlinestatus.cpp 
   kopeteonlinestatusmanager.cpp 
   kopeteprotocol.cpp 
   kopetecontact.cpp 
   kopetepluginmanager.cpp 
   kopeteplugin.cpp 
   kopetemessage.cpp 
   kopetechatsession.cpp 
   kopetechatsessionmanager.cpp 
   kopetecontactlist.cpp 
   kopetecontactliststorage.cpp 
   kopetemetacontact.cpp 
   kopetetransfermanager.cpp 
   kopetegroup.cpp 
   kopeteaccountmanager.cpp 
   kopeteaccount.cpp 
   kopetecontactlistelement.cpp 
   kopetecommand.cpp 
   kopetecommandhandler.cpp 
   kopeteaway.cpp 
   kopeteawayaction.cpp 
   kopetewalletmanager.cpp 
   kopetecontactproperty.cpp 
   kopetepassword.cpp 
   kopeteglobal.cpp 
   kopeteuiglobal.cpp 
   kopetepasswordedaccount.cpp 
   kopetemimetypehandler.cpp 
   kopetetask.cpp 
   kopetemimesourcefactory.cpp 
   kopeteblacklister.cpp 
   kopetemessageevent.cpp 
   kopetemessagehandler.cpp 
   kopetemessagehandlerchain.cpp 
   kopetesimplemessagehandler.cpp 
   kopeteproperties.cpp 
   kabcpersistence.cpp 
   networkstatuscommon.h 
   kopeteutils.cpp 
   kopetepicture.cpp 
   kopetestatusmessage.cpp 

# REMOVED FOR NOW
#   connectionmanager.cpp 
#   managedconnectionaccount.cpp 

)


kde4_add_ui_files(kopete_LIB_SRCS
    ui/kopetepassworddialog.ui
    ui/metacontactselectorwidget_base.ui
    ui/fileconfirmbase.ui
    ui/kopetepasswordwidgetbase.ui
    ui/contactaddednotifywidget.ui 
    ui/addressbookselectorwidget_base.ui
    ui/addressbooklinkwidget_base.ui
    ui/kopeteawaydialogbase.ui )

kde4_automoc(${kopete_LIB_SRCS})

kde4_add_kcfg_files(kopete_LIB_SRCS GENERATE_MOC kopetegeneralsettings.kcfgc kopeteappearancesettings.kcfgc kopetebehaviorsettings.kcfgc )

kde4_add_library(kopete SHARED ${kopete_LIB_SRCS})

target_link_libraries(kopete ${KDE4_KDE3SUPPORT_LIBS} kabc ${X11_Xss_LIB} )

if(X11_Xrender_FOUND)
  target_link_libraries(kopete ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(kopete PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopete  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES kopetegeneralsettings.kcfg kopeteappearancesettings.kcfg kopetebehaviorsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopetecommandui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete)
install( FILES kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES 
  kopeteaccount.h kopeteaccountmanager.h kopeteawayaction.h kopeteaway.h kopeteblacklister.h 
  kopetecommand.h kopetecommandhandler.h kopetecontact.h kopetecontactlistelement.h 
  kopetecontactlist.h kopetecontactproperty.h kopete_export.h kopeteglobal.h kopetegroup.h 
  kopetemessageevent.h kopetemessage.h kopetemessagehandlerchain.h kopetemessagehandler.h 
  kopetechatsession.h kopetechatsessionmanager.h kopetemetacontact.h kopetemimetypehandler.h 
  kopeteonlinestatus.h kopeteonlinestatusmanager.h kopetepasswordedaccount.h kopetepassword.h 
  kopeteplugin.h kopeteprotocol.h kopetesimplemessagehandler.h kopetetask.h kopetetransfermanager.h 
  kopeteuiglobal.h kabcpersistence.h kopeteversion.h kopetepicture.h ${CMAKE_CURRENT_BINARY_DIR}/kopeteappearancesettings.h 
  ${CMAKE_CURRENT_BINARY_DIR}/kopetebehaviorsettings.h ${CMAKE_CURRENT_BINARY_DIR}/kopetegeneralsettings.h kopetestatusmessage.h 
  DESTINATION ${INCLUDE_INSTALL_DIR}/kopete )




#original Makefile.am contents follow:

#include ../../admin/Doxyfile.am
#DOXYGEN_REFERENCES = kio kdecore kdeui
#DOXYGEN_EXCLUDE = compat
#DOXYGEN_SET_PROJECT_NAME = libkopete
#
#SUBDIRS = private ui . avdevice
#
#METASOURCES = AUTO
#
#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
#	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
#	-I$(top_srcdir)/kopete/libkopete/ui $(all_includes)
#
#lib_LTLIBRARIES = libkopete.la
#
#libkopete_la_SOURCES = connectionmanager.cpp 	kopeteonlinestatus.cpp \
#	kopeteonlinestatusmanager.cpp kopeteprotocol.cpp kopetecontact.cpp 	kopetepluginmanager.cpp \
#	kopeteplugin.cpp kopetemessage.cpp kopetechatsession.cpp 	kopetechatsessionmanager.cpp \
#	kopetecontactlist.cpp kopetemetacontact.cpp 	kopetetransfermanager.cpp kopetegroup.cpp \
#	kopeteaccountmanager.cpp kopeteaccount.cpp 	kopetecontactlistelement.cpp kopetecommand.cpp \
#	kopetecommandhandler.cpp kopeteaway.cpp kopeteawayaction.cpp 	kopetewalletmanager.cpp \
#	kopetecontactproperty.cpp kopetepassword.cpp kopeteglobal.cpp 	kopeteuiglobal.cpp \
#	kopetepasswordedaccount.cpp kopetemimetypehandler.cpp kopetetask.cpp 	kopetemimesourcefactory.cpp \
#		kopeteblacklister.cpp kopetemessageevent.cpp kopetemessagehandler.cpp \
#		kopetemessagehandlerchain.cpp kopetesimplemessagehandler.cpp kopeteproperties.cpp \
#		kabcpersistence.cpp connectionmanager.skel clientiface.stub managedconnectionaccount.cpp \
#		networkstatuscommon.h kopetegeneralsettings.kcfgc kopeteutils.cpp \
#	kopetepicture.cpp 	kopeteappearancesettings.kcfgc kopetebehaviorsettings.kcfgc \
#	kopetestatusmessage.cpp
#
#libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
#libkopete_la_LIBADD = -lkabc ui/libkopeteui.la $(LIB_KIO) $(LIB_XSS)
#
#kde_kcfg_DATA = kopetegeneralsettings.kcfg kopeteappearancesettings.kcfg kopetebehaviorsettings.kcfg
#
##AM_CXXFLAGS = -DQT_PLUGIN
##kde_widget_LTLIBRARIES = libkopetewidgets.la
##libkopetewidgets_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
##libkopetewidgets_la_LIBADD = $(LIB_KIO) libkopete.la ui/libkopeteui.la
##libkopetewidgets_la_SOURCES = ui/kopetewidgets.cpp
#
##kopetewidgets.cpp: $(srcdir)/kopete.widgets
##	$(MAKEKDEWIDGETS) -o kopetewidgets.cpp $(srcdir)/kopete.widgets
#
#rcdir = $(kde_datadir)/kopete
#rc_DATA = kopetecommandui.rc
#
#servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop
#servicetypedir = $(kde_servicetypesdir)
#
#kopeteincludedir = $(includedir)/kopete
#kopeteinclude_HEADERS = kopeteaccount.h kopeteaccountmanager.h \
#	kopeteawayaction.h kopeteaway.h 	kopeteblacklister.h kopetecommand.h \
#	kopetecommandhandler.h kopetecontact.h kopetecontactlistelement.h kopetecontactlist.h \
#		kopetecontactproperty.h kopete_export.h kopeteglobal.h kopetegroup.h 	kopetemessageevent.h \
#	kopetemessage.h kopetemessagehandlerchain.h kopetemessagehandler.h 	kopetechatsession.h \
#	kopetechatsessionmanager.h kopetemetacontact.h kopetemimetypehandler.h 	kopeteonlinestatus.h \
#	kopeteonlinestatusmanager.h kopetepasswordedaccount.h 	kopetepassword.h kopeteplugin.h \
#	kopeteprotocol.h kopetesimplemessagehandler.h kopetetask.h 	kopetetransfermanager.h \
#	kopeteuiglobal.h kabcpersistence.h managedconnectionaccount.h 	kopeteversion.h \
#	kopetepicture.h kopeteappearancesettings.h kopetebehaviorsettings.h \
#	kopetegeneralsettings.h kopetestatusmessage.h
#
## vim: set noet:
#
#noinst_HEADERS = kopeteblacklister.h
