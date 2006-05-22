include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
${CMAKE_CURRENT_BINARY_DIR}/ui/ 
${CMAKE_CURRENT_SOURCE_DIR}/libkric/ 
)

add_subdirectory( icons ) 
add_subdirectory( libkirc ) 
#add_subdirectory( ui ) 


########### next target ###############

set(kopete_irc_ui_SRCS 
ui/irceditaccountwidget.cpp 
ui/channellist.cpp 
ui/channellistdialog.cpp
)

kde4_add_ui3_files(kopete_irc_ui_SRCS 
ui/ircadd.ui 
ui/irceditaccount.ui 
ui/networkconfig.ui 
)

set(kopete_irc_PART_SRCS 
   ${kopete_irc_ui_SRCS} 
   ircaccount.cpp 
   ircaddcontactpage.cpp 
   ircchannelcontact.cpp 
   irccontact.cpp 
   ircguiclient.cpp 
   ircprotocol.cpp 
   ircservercontact.cpp 
   ircsignalhandler.cpp 
   irctransferhandler.cpp 
   ircusercontact.cpp 
   irccontactmanager.cpp 
   kcodecaction.cpp 
   ksparser.cpp )

kde4_automoc(${kopete_irc_PART_SRCS})

kde4_add_plugin(kopete_irc ${kopete_irc_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_irc )

target_link_libraries(kopete_irc ${KDE4_KIO_LIBS} kopete kirc )

install_targets(${PLUGIN_INSTALL_DIR} kopete_irc )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_irc.desktop irc.protocol )
install_files( ${DATA_INSTALL_DIR}/kopete FILES ircchatui.rc )
install_files( ${DATA_INSTALL_DIR}/kopete FILES ircnetworks.xml )
