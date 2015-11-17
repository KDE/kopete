add_definitions(-DIRIS_XMPP_JID_DEPRECATED)

add_subdirectory( icons ) 
add_subdirectory( libiris ) 

#FIXME:glib : necessary ?
include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/
${CMAKE_CURRENT_SOURCE_DIR}/tasks/
${CMAKE_CURRENT_SOURCE_DIR}/libiris/include/iris
${CMAKE_CURRENT_SOURCE_DIR}/libiris/src
${QCA2_INCLUDE_DIR}
)

if(BUILD_LIBJINGLE)
  add_subdirectory(libjingle)
  add_definitions(-DLIBJINGLE_SUPPORT)
endif(BUILD_LIBJINGLE)

IF(BUILD_JINGLE)
  add_definitions(-DJINGLE_SUPPORT)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/jingle)
  include_directories(${LIBORTP_INCLUDE_DIR} ${ALSA_INCLUDES} ${SPEEX_INCLUDE_DIR})
ENDIF(BUILD_JINGLE)

#link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libiris/ ${CMAKE_CURRENT_BINARY_DIR}/jingle/ )
link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libiris/ )

########### next target ###############

set(kopete_jabber_ui_SRCS 
    ui/dlgxmppconsole.cpp
    ui/jabberaddcontactpage.cpp 
    ui/dlgjabbervcard.cpp 
    ui/dlgjabberservices.cpp 
    ui/dlgregister.cpp 
    ui/dlgsearch.cpp 
    ui/dlgahcommand.cpp 
    ui/dlgahclist.cpp 
    ui/jabbereditaccountwidget.cpp 
    ui/jabberregisteraccount.cpp 
    ui/jabberchooseserver.cpp 
    ui/dlgjabberbookmarkeditor.cpp
    ui/dlgjabberchangepassword.cpp
    ui/dlgjabberchatroomslist.cpp
    ui/dlgjabberchatjoin.cpp 
    ui/privacylistblockedmodel.cpp
    ui/privacylistmodel.cpp
    ui/privacydlg.cpp
    ui/privacyruledlg.cpp
)

if(BUILD_LIBJINGLE)
   set(kopete_jabber_ui_SRCS
      ${kopete_jabber_ui_SRCS}
      libjinglecalldialog.cpp
   )
endif(BUILD_LIBJINGLE)

if(BUILD_JINGLE)	
  set(kopete_jabber_ui_SRCS
      ${kopete_jabber_ui_SRCS}
      jingle/jinglecontentdialog.cpp
      jingle/jinglecallsgui.cpp
  )
endif(BUILD_JINGLE)

set(kopete_jabber_ui_files
#   ${kopete_jabber_ui_SRCS}
   ui/dlgxmppconsole.ui
   ui/dlgjabberregisteraccount.ui 
   ui/dlgjabberchooseserver.ui 
   ui/dlgvcard.ui 
   ui/dlgservices.ui 
   ui/dlgaddcontact.ui 
   ui/dlgsearch.ui 
   ui/dlgchangepassword.ui 
   ui/dlgjabberbookmarkeditor.ui
   ui/dlgjabbereditaccountwidget.ui
   ui/dlgchatjoin.ui 
   ui/dlgchatroomslist.ui 
   ui/privacyrule.ui
   ui/privacy.ui
)

if(BUILD_LIBJINGLE)
   set(kopete_jabber_ui_files
      ${kopete_jabber_ui_files}
      libjinglecalldialog.ui
   )
endif(BUILD_LIBJINGLE)

if(BUILD_JINGLE)
  set(kopete_jabber_ui_files
      ${kopete_jabber_ui_files}
      jingle/jinglecontentdialog.ui
      jingle/jinglecallsgui.ui
  )
endif(BUILD_JINGLE)

kde4_add_ui_files(kopete_jabber_ui_SRCS ${kopete_jabber_ui_files})

set(kopete_jabber_PART_SRCS 
   tasks/jt_getlastactivity.cpp
   tasks/jt_privatestorage.cpp
   tasks/jt_ahcommand.cpp
   tasks/jt_xsearch.cpp
   tasks/jt_xregister.cpp
   tasks/jt_pubsub.cpp
   tasks/mood.cpp
   tasks/privacylistitem.cpp
   tasks/privacylist.cpp
   tasks/privacymanager.cpp
   ${kopete_jabber_ui_SRCS} 
   jabberprotocol.cpp 
   jabberaccount.cpp 
   jabberresource.cpp 
   jabberresourcepool.cpp 
   jabberbasecontact.cpp 
   jabbercontact.cpp 
   jabbergroupcontact.cpp 
   jabbergroupmembercontact.cpp 
   jabbercontactpool.cpp 
   jabberformtranslator.cpp 
   jabberxdatawidget.cpp 
   jabberformlineedit.cpp 
   jabberchatsession.cpp 
   jabbergroupchatmanager.cpp 
   jabberfiletransfer.cpp 
   jabbercapabilitiesmanager.cpp 
   jabbertransport.cpp 
   jabberbookmarks.cpp 
   jabberclient.cpp 
   jabberbobcache.cpp
)

if(BUILD_LIBJINGLE)
  set(kopete_jabber_PART_SRCS
      ${kopete_jabber_PART_SRCS}
      libjingle.cpp
  )
endif(BUILD_LIBJINGLE)

if(BUILD_JINGLE)	
  set(kopete_jabber_PART_SRCS
      ${kopete_jabber_PART_SRCS}
      jingle/jinglecallsmanager.cpp
      jingle/jabberjinglesession.cpp
      jingle/jabberjinglecontent.cpp
      jingle/jinglertpsession.cpp
      jingle/mediamanager.cpp
      jingle/mediasession.cpp
      jingle/alsaio.cpp
      jingle/speexio.cpp
      jingle/abstractio.cpp
  )
endif(BUILD_JINGLE)

kde4_add_plugin(kopete_jabber ${kopete_jabber_PART_SRCS})


target_link_libraries(kopete_jabber ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QCA2_LIBRARIES} ${KDE4_SOLID_LIBS} kopete iris_kopete)
if(BUILD_JINGLE)	
  target_link_libraries(kopete_jabber ${LIBORTP_LIBRARY} ${ASOUND_LIBRARY} ${SPEEX_LIBRARY})
endif(BUILD_JINGLE)

install(TARGETS kopete_jabber  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_jabber.desktop xmpp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES jabberchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_jabber)
