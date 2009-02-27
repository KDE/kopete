add_definitions( -DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libgroupwise/tasks ${CMAKE_CURRENT_SOURCE_DIR}/libgroupwise ${CMAKE_CURRENT_SOURCE_DIR}/ui ${QCA2_INCLUDE_DIR} )

add_subdirectory( libgroupwise ) 
add_subdirectory( icons ) 


########### ui parts ####################

set( GROUPWISE_DEBUG_GLOBAL 14190 )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=${GROUPWISE_DEBUG_GLOBAL} )

set( kopete_groupwise_ui_SRCS
	ui/gwaddcontactpage.cpp
	ui/gweditaccountwidget.cpp
	ui/gwreceiveinvitationdialog.cpp
	ui/gwcontactproperties.cpp
	ui/gwprivacydialog.cpp
	ui/gwsearch.cpp
	ui/gwchatsearchdialog.cpp
	ui/gwchatpropsdialog.cpp
)

kde4_add_ui_files(kopete_groupwise_ui_SRCS
	ui/gwaccountpreferences.ui
	ui/gwaddui.ui
	ui/gwchatprops.ui
	ui/gwchatsearch.ui
	ui/gwshowinvitation.ui
	ui/gwcontactprops.ui
	ui/gwprivacy.ui
	ui/gwcustomstatus.ui
	ui/gwcustomstatusedit.ui
	ui/gwcontactprops.ui
	ui/gwcontactsearch.ui
	ui/gwcustomstatus.ui
	ui/gwcustomstatusedit.ui
	ui/gwcontactsearch.ui
	ui/gwchatsearch.ui
)

########### next target ###############

set(kopete_groupwise_PART_SRCS
   ${kopete_groupwise_ui_SRCS}
   gwprotocol.cpp
   gwcontact.cpp
   gwaccount.cpp
   gwbytestream.cpp
   gwconnector.cpp
   gwmessagemanager.cpp
   gwcontactlist.cpp
)

kde4_add_plugin(kopete_groupwise ${kopete_groupwise_PART_SRCS})

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libgroupwise )

target_link_libraries(kopete_groupwise  ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} qgroupwise kopete ${QCA2_LIBRARIES})

install(TARGETS kopete_groupwise  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_groupwise.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES gwchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_groupwise)
