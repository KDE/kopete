add_subdirectory( icons )
add_subdirectory( libkirc )

include_directories( ${KOPETE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/libkirc/ ${CMAKE_CURRENT_BINARY_DIR}/libkirc/ )

set(kopeteirc_ui_SRCS 
  ui/channellist.cpp 
  ui/channellistdialog.cpp 
  ui/ircaddcontactpage.cpp 
  ui/ircdialogs.cpp 
  ui/irceditaccountwidget.cpp 
  ui/kcodecaction.cpp 
  ui/networkconfigwidget.cpp 
)

kde4_add_ui_files(kopeteirc_ui_SRCS 
  ui/ircadd.ui 
  ui/irceditaccount.ui 
  ui/networkconfig.ui 
)

set(kopete_irc_SRCS
  ${kopeteirc_ui_SRCS} 
  ircaccount.cpp 
  ircconst.cpp 
  irccontact_channel.cpp 
  irccontact.cpp 
  irccontact_server.cpp 
  irccontact_user.cpp 
  ircguiclient.cpp 
  ircnetwork.cpp 
  ircprotocol.cpp 
#  irctransferhandler.cpp 
  ksparser.cpp 
)

kde4_automoc( ${kopete_irc_SRCS} )

kde4_add_plugin(kopete_irc ${kopete_irc_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_irc )

target_link_libraries(kopete_irc ${KDE4_KIO_LIBS} kopete kirc )

install(TARGETS kopete_irc  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_irc.desktop irc.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES ircchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_irc)
install( FILES ircnetworks.xml  DESTINATION ${DATA_INSTALL_DIR}/kopete)
