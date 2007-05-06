
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
  ui/avatarselectordialog.cpp
  ui/avatarselectorwidget.cpp
  ui/contactaddednotifydialog.cpp
  ui/editaccountwidget.cpp
  ui/kopetecontactaction.cpp
  ui/kopetefileconfirmdialog.cpp
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

set(kopete_contactlist_SRCS
  contactlist/kopetecontactliststorage.cpp
  contactlist/xmlcontactstorage.cpp
)

set(kopete_tasks_SRCS
  tasks/kopetetask.cpp
)

set(kopete_LIB_SRCS
   ${kopete_private_SRCS}
   ${kopete_ui_SRCS}
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
   kopetecontactproperty.cpp
   kopeteglobal.cpp
   kopetegroup.cpp
   kopetemessage.cpp
   kopetemessageevent.cpp
   kopetemessagehandlerchain.cpp
   kopetemessagehandler.cpp
   kopetemetacontact.cpp
   kopetemimesourcefactory.cpp
   kopetemimetypehandler.cpp
   kopeteonlinestatus.cpp
   kopeteonlinestatusmanager.cpp
   kopetepassword.cpp
   kopetepasswordedaccount.cpp
   kopetepicture.cpp
   kopeteplugin.cpp
   kopetepluginmanager.cpp
   kopeteproperties.cpp
   kopeteprotocol.cpp
   kopetesimplemessagehandler.cpp
   kopetestatusmessage.cpp
   kopetetransfermanager.cpp
   kopeteuiglobal.cpp
   kopeteutils.cpp
   kopetewalletmanager.cpp
   networkstatuscommon.h
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

kde4_automoc(${kopete_LIB_SRCS})

kde4_add_kcfg_files(kopete_LIB_SRCS GENERATE_MOC kopetegeneralsettings.kcfgc kopeteappearancesettings.kcfgc kopetebehaviorsettings.kcfgc )

kde4_add_library(kopete SHARED ${kopete_LIB_SRCS})

target_link_libraries(kopete ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KABC_LIBS} ${X11_Xss_LIB} )

if(X11_Xrender_FOUND)
  target_link_libraries(kopete ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(kopete PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
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
 ui/avatarselectordialog.h
 ui/avatarselectorwidget.h
 ui/editaccountwidget.h
 ui/kopetecontactaction.h
 ui/kopetefileconfirmdialog.h
 ui/kopetelistview.h
 ui/kopetelistviewitem.h
 ui/kopetelistviewsearchline.h
 ui/kopetepasswordwidget.h
 ui/kopetestdaction.h
 ui/kopeteview.h
 ui/kopeteviewplugin.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete/ui )


install( FILES
  kopeteappearancesettings.kcfg
  kopetebehaviorsettings.kcfg
  kopetegeneralsettings.kcfg
DESTINATION ${KCFG_INSTALL_DIR})

install( FILES kopetecommandui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete)
install( FILES kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kopeteappearancesettings.h
  ${CMAKE_CURRENT_BINARY_DIR}/kopetebehaviorsettings.h
  ${CMAKE_CURRENT_BINARY_DIR}/kopetegeneralsettings.h
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
  kopetecontactproperty.h
  kopete_export.h
  kopeteglobal.h
  kopetegroup.h
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
  kopeteprotocol.h
  kopetesimplemessagehandler.h
  kopetestatusmessage.h
  kopetetransfermanager.h
  kopeteuiglobal.h
  kopeteversion.h
  tasks/kopetetask.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete )

install( FILES org.kde.kopete.Client.xml DESTINATION ${DBUS_INTERFACES_DIR} )
