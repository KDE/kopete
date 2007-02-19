
add_subdirectory( private ) 
if(NOT WIN32)
  add_subdirectory( avdevice ) 
endif(NOT WIN32)
add_subdirectory( tests )

include_directories( ${KOPETE_INCLUDES} )

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
  ui/avatarselectorwidget.cpp
)

set(kopete_contactlist_SRCS
  contactlist/kopetecontactliststorage.cpp
  contactlist/xmlcontactstorage.cpp
)

set(kopete_LIB_SRCS
   ${kopete_private_SRCS}
   ${kopete_ui_SRCS}
   ${kopete_contactlist_SRCS}
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
   kopeteavatarmanager.cpp
# REMOVED FOR NOW
#   connectionmanager.cpp 
#   managedconnectionaccount.cpp 

)


kde4_add_ui_files(kopete_LIB_SRCS
 ui/metacontactselectorwidget_base.ui
 ui/fileconfirmbase.ui
 ui/kopetepasswordwidgetbase.ui
 ui/contactaddednotifywidget.ui 
 ui/addressbookselectorwidget_base.ui
 ui/addressbooklinkwidget_base.ui
 ui/kopeteawaydialogbase.ui
 ui/avatarselectorwidget.ui
)

kde4_automoc(${kopete_LIB_SRCS})

kde4_add_kcfg_files(kopete_LIB_SRCS GENERATE_MOC kopetegeneralsettings.kcfgc kopeteappearancesettings.kcfgc kopetebehaviorsettings.kcfgc )

kde4_add_library(kopete SHARED ${kopete_LIB_SRCS})

target_link_libraries(kopete ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KABC_LIBS} ${X11_Xss_LIB} )

if(X11_Xrender_FOUND)
  target_link_libraries(kopete ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(kopete PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kopete  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install(FILES
 ui/accountselector.h
 ${CMAKE_CURRENT_BINARY_DIR}/ui_fileconfirmbase.h
 ui/kopetefileconfirmdialog.h
 ui/kopetepasswordwidget.h
 ui/kopeteview.h
 ui/addcontactpage.h
 ${CMAKE_CURRENT_BINARY_DIR}/ui_kopeteawaydialogbase.h
 ${CMAKE_CURRENT_BINARY_DIR}/ui_kopetepasswordwidgetbase.h
 ui/kopeteviewplugin.h
 ui/editaccountwidget.h
 ui/kopetecontactaction.h
 ui/kopetestdaction.h
 ui/addressbookselectordialog.h
 ui/addressbookselectorwidget.h
 ui/kopetelistview.h
 ui/kopetelistviewitem.h
 ui/kopetelistviewsearchline.h
 ui/avatarselectorwidget.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete/ui )


install( FILES kopetegeneralsettings.kcfg kopeteappearancesettings.kcfg kopetebehaviorsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopetecommandui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete)
install( FILES kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES
  kopeteaccount.h
  kopeteaccountmanager.h
  kopeteavatarmanager.h
  kopeteawayaction.h
  kopeteaway.h
  kopeteblacklister.h
  kopetecommand.h
  kopetecommandhandler.h
  kopetecontact.h
  kopetecontactlistelement.h
  kopetecontactlist.h
  kopetecontactproperty.h
  kopete_export.h
  kopeteglobal.h
  kopetegroup.h
  kopetemessageevent.h
  kopetemessage.h
  kopetemessagehandlerchain.h
  kopetemessagehandler.h
  kopetechatsession.h
  kopetechatsessionmanager.h
  kopetemetacontact.h
  kopetemimetypehandler.h
  kopeteonlinestatus.h
  kopeteonlinestatusmanager.h
  kopetepasswordedaccount.h
  kopetepassword.h
  kopeteplugin.h
  kopeteprotocol.h
  kopetesimplemessagehandler.h
  kopetetask.h
  kopetetransfermanager.h
  kopeteuiglobal.h
  kabcpersistence.h
  kopeteversion.h
  kopetepicture.h
  ${CMAKE_CURRENT_BINARY_DIR}/kopeteappearancesettings.h
  ${CMAKE_CURRENT_BINARY_DIR}/kopetebehaviorsettings.h 
  ${CMAKE_CURRENT_BINARY_DIR}/kopetegeneralsettings.h kopetestatusmessage.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete )



install( FILES org.kde.kopete.Client.xml DESTINATION ${DBUS_INTERFACES_DIR} )
