add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

#add_subdirectory( ui ) 
add_subdirectory( icons ) 
add_subdirectory( libiris ) 

# we can't use KDE_FULL_TEMPLATE_EXPORT_INSTANTIATION because libiris is an external lib
if(MSVC)
  add_definitions(-DIRIS_FULL_TEMPLATE_EXPORT_INSTANTIATION)
endif(MSVC)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(JINGLE)")
include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/
${CMAKE_CURRENT_SOURCE_DIR}/tasks/
${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/include/
${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/jabber/ 
${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-core/ 
${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-im/ 
${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/util/ 
${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/network/ 
${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/ 
${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/ 
${QCA2_INCLUDE_DIR}
)

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
    ui/dlgjabberchangepassword.cpp
    ui/dlgjabberchatroomslist.cpp
    ui/dlgjabberchatjoin.cpp 
)

kde4_add_ui_files(kopete_jabber_ui_SRCS 
   ui/dlgxmppconsole.ui
   ui/dlgjabberregisteraccount.ui 
   ui/dlgjabberchooseserver.ui 
   ui/dlgvcard.ui 
   ui/dlgservices.ui 
   ui/dlgaddcontact.ui 
   ui/dlgregister.ui 
   ui/dlgsearch.ui 
   ui/dlgchangepassword.ui 
   ui/dlgjabbereditaccountwidget.ui
   ui/dlgchatjoin.ui 
   ui/dlgchatroomslist.ui 
)

set(kopete_jabber_PART_SRCS 
   tasks/jt_getlastactivity.cpp
   tasks/jt_privatestorage.cpp
   tasks/jt_ahcommand.cpp
   tasks/jt_xsearch.cpp
   tasks/jt_xregister.cpp
   tasks/jt_pubsub.cpp
   tasks/mood.cpp
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
   jabberconnector.cpp 
   jabberbytestream.cpp 
)


kde4_add_plugin(kopete_jabber ${kopete_jabber_PART_SRCS})


target_link_libraries(kopete_jabber ${KDE4_KDE3SUPPORT_LIBS} ${QCA2_LIBRARIES} kopete iris_kopete )

install(TARGETS kopete_jabber  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_jabber.desktop xmpp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES jabberchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_jabber)
