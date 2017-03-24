add_subdirectory( icons )
add_subdirectory( libkirc )

include_directories(
	${KOPETE_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}/libkirc/client
	${CMAKE_CURRENT_SOURCE_DIR}/libkirc/kernel
	${CMAKE_CURRENT_SOURCE_DIR}/ui/
	${CMAKE_CURRENT_BINARY_DIR}/ui/
)

set(kopeteirc_ui_SRCS 
#  ui/channellist.cpp 
#  ui/channellistdialog.cpp 
  ui/ircaddcontactpage.cpp 
#  ui/ircdialogs.cpp 
  ui/irceditaccountwidget.cpp 
  ui/networkconfigwidget.cpp 
)

ki18n_wrap_ui(kopeteirc_ui_SRCS 
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


add_library(kopete_irc MODULE ${kopete_irc_SRCS})

target_link_libraries(kopete_irc KF5::KIOCore ${QT_QT3SUPPORT_LIBRARY} kopete kirc_client kirc )

install(TARGETS kopete_irc  DESTINATION ${KDE_INSTALL_PLUGINDIR})

############ Test Files #################

########### install files ###############

install( FILES kopete_irc.desktop irc.protocol  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES ircchatui.rc  DESTINATION ${KDE_INSTALL_DATADIR}/kopete_irc)
install( FILES ircnetworks.xml  DESTINATION ${KDE_INSTALL_DATADIR}/kopete)

