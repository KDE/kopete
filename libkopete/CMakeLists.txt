
add_subdirectory( private )
if(NOT WIN32)
  add_subdirectory( avdevice )
endif(NOT WIN32)
add_subdirectory( tests )


FIND_PACKAGE(Upnp REQUIRED)

include_directories( ${KOPETE_INCLUDES} ${UPNP_INCLUDE_DIR})

########### next target ###############

set(kopete_private_SRCS
  private/kopeteemoticons.cpp
  private/kopeteutils_private.cpp
  private/kopeteviewmanager.cpp
)

set(kopete_ui_SRCS
  ui/accountselector.cpp
  ui/addcontactpage.cpp
  ui/addressbooklinkwidget.cpp
  ui/addressbookselectordialog.cpp
  ui/addressbookselectorwidget.cpp
  ui/addresseeitem.cpp
  ui/avatardialog.cpp
  ui/avatarselectorwidget.cpp
  ui/contactaddednotifydialog.cpp
  ui/collapsiblewidget.cpp
  ui/editaccountwidget.cpp
  ui/kopetecontactaction.cpp
  ui/kopetefileconfirmdialog.cpp
  ui/kopeteinfodialog.cpp
  ui/kopetelistview.cpp
  ui/kopetelistviewitem.cpp
  ui/kopetelistviewsearchline.cpp
  ui/kopetepasswordwidget.cpp
  ui/kopetestdaction.cpp
  ui/kopeteview.cpp
  ui/kopeteviewplugin.cpp
  ui/metacontactselectorwidget.cpp
  ui/webcamwidget.cpp
)

set(kopete_upnp_SRCS
  upnp/action.cpp
  upnp/argument.cpp
  upnp/router.cpp
  upnp/service.cpp
  upnp/UPnp.cpp
  upnp/upnpRouter.cpp
  upnp/upnpRouterPrivate.cpp
  upnp/util_Xml.cpp
)

set(kopete_contactlist_SRCS
  contactlist/kopetecontactliststorage.cpp
  contactlist/xmlcontactstorage.cpp
)

set(kopete_tasks_SRCS
  tasks/kopetetask.cpp
  tasks/kopetecontacttaskbase.cpp
  tasks/kopetedeletecontacttask.cpp
)

set(kopete_LIB_SRCS
   ${kopete_private_SRCS}
   ${kopete_ui_SRCS}
   ${kopete_upnp_SRCS}
   ${kopete_contactlist_SRCS}
   ${kopete_tasks_SRCS}
   kabcpersistence.cpp
   kopeteaccount.cpp
   kopeteaccountmanager.cpp
   kopeteavatarmanager.cpp
   kopeteawayaction.cpp
   kopeteaway.cpp
   kopeteblacklister.cpp
   kopetechatsession.cpp
   kopetechatsessionmanager.cpp
   kopetecommand.cpp
   kopetecommandhandler.cpp
   kopetecontact.cpp
   kopetecontactlist.cpp
   kopetecontactlistelement.cpp
   kopeteproperty.cpp
   kopeteglobal.cpp
   kopetegroup.cpp
   kopeteidentity.cpp
   kopeteidentitymanager.cpp
   kopetemessage.cpp
   kopetemessageevent.cpp
   kopetemessagehandlerchain.cpp
   kopetemessagehandler.cpp
   kopetemetacontact.cpp
   kopetefileengine.cpp
   kopetemimetypehandler.cpp
   kopeteonlinestatus.cpp
   kopeteonlinestatusmanager.cpp
   kopetepassword.cpp
   kopetepasswordedaccount.cpp
   kopetepicture.cpp
   kopeteplugin.cpp
   kopetepluginmanager.cpp
   kopeteproperties.cpp
   kopetepropertycontainer.cpp
   kopeteprotocol.cpp
   kopetesimplemessagehandler.cpp
   kopetestatusmessage.cpp
   kopetetransfermanager.cpp
   kopeteuiglobal.cpp
   kopeteutils.cpp
   kopetewalletmanager.cpp
   networkstatuscommon.h
   chatsessionmemberslistmodel.cpp
# REMOVED FOR NOW
#   connectionmanager.cpp
#   managedconnectionaccount.cpp

)

kde4_add_ui_files(kopete_LIB_SRCS
 ui/addressbooklinkwidget_base.ui
 ui/addressbookselectorwidget_base.ui
 ui/avatarselectorwidget.ui
 ui/contactaddednotifywidget.ui
 ui/fileconfirmbase.ui
 ui/kopeteawaydialogbase.ui
 ui/kopetepasswordwidgetbase.ui
 ui/metacontactselectorwidget_base.ui
)

kde4_add_kcfg_files(kopete_LIB_SRCS GENERATE_MOC kopeteappearancesettings.kcfgc kopetebehaviorsettings.kcfgc )

kde4_add_library(kopete SHARED ${kopete_LIB_SRCS})

target_link_libraries(kopete ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KABC_LIBS} ${UPNP_LIBRARY} ${IXML_LIBRARY})
if(Q_WS_X11 AND X11_Xss_FOUND)
   target_link_libraries(kopete ${X11_Xss_LIB})
endif(Q_WS_X11 AND X11_Xss_FOUND)

set_target_properties(kopete PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
    )
if(MSVC)
  set_target_properties(kopete PROPERTIES
      OUTPUT_NAME libkopete
      )
endif(MSVC)
install(TARGETS kopete DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install(FILES
 ${CMAKE_CURRENT_BINARY_DIR}/ui_fileconfirmbase.h
 ${CMAKE_CURRENT_BINARY_DIR}/ui_kopeteawaydialogbase.h
 ${CMAKE_CURRENT_BINARY_DIR}/ui_kopetepasswordwidgetbase.h
 ui/accountselector.h
 ui/addcontactpage.h
 ui/addressbookselectordialog.h
 ui/addressbookselectorwidget.h
 ui/avatardialog.h
 ui/avatarselectorwidget.h
 ui/editaccountwidget.h
 ui/kopetecontactaction.h
 ui/kopetefileconfirmdialog.h
 ui/kopeteinfodialog.h
 ui/kopetelistview.h
 ui/kopetelistviewitem.h
 ui/kopetelistviewsearchline.h
 ui/kopetepasswordwidget.h
 ui/kopetestdaction.h
 ui/kopeteview.h
 ui/kopeteviewplugin.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete/ui )

install(FILES
  upnp/action.cpp
  upnp/argument.cpp
  upnp/router.cpp
  upnp/service.cpp
  upnp/UPnp.cpp
  upnp/upnpRouter.cpp
  upnp/upnpRouterPrivate.cpp
  upnp/util_Xml.cpp
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete/upnp)

install( FILES
  kopeteappearancesettings.kcfg
  kopetebehaviorsettings.kcfg
DESTINATION ${KCFG_INSTALL_DIR})

install( FILES kopetecommandui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete)
install( FILES kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kopeteappearancesettings.h
  ${CMAKE_CURRENT_BINARY_DIR}/kopetebehaviorsettings.h
  kabcpersistence.h
  kopeteaccount.h
  kopeteaccountmanager.h
  kopeteavatarmanager.h
  kopeteawayaction.h
  kopeteaway.h
  kopeteblacklister.h
  kopetechatsession.h
  kopetechatsessionmanager.h
  kopetecommand.h
  kopetecommandhandler.h
  kopetecontact.h
  kopetecontactlistelement.h
  kopetecontactlist.h
  kopeteproperty.h
  kopete_export.h
  kopeteglobal.h
  kopetegroup.h
  kopeteidentity.h
  kopeteidentitymanager.h
  kopetemessageevent.h
  kopetemessage.h
  kopetemessagehandlerchain.h
  kopetemessagehandler.h
  kopetemetacontact.h
  kopetemimetypehandler.h
  kopeteonlinestatus.h
  kopeteonlinestatusmanager.h
  kopetepasswordedaccount.h
  kopetepassword.h
  kopetepicture.h
  kopeteplugin.h
  kopetepluginmanager.h
  kopeteprotocol.h
  kopetepropertycontainer.h
  kopetesimplemessagehandler.h
  kopetestatusmessage.h
  kopetetransfermanager.h
  kopeteuiglobal.h
  kopeteversion.h
  tasks/kopetetask.h
  tasks/kopetecontacttaskbase.h
  tasks/kopetedeletecontacttask.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete )

install( FILES org.kde.kopete.Client.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
