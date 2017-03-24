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

ki18n_wrap_ui(kopete_groupwise_ui_SRCS
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

add_library(kopete_groupwise MODULE ${kopete_groupwise_PART_SRCS})

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libgroupwise )

target_link_libraries(kopete_groupwise  KF5::KDELibs4Support qgroupwise kopete ${QCA2_LIBRARIES})

install(TARGETS kopete_groupwise  DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install( FILES kopete_groupwise.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES gwchatui.rc  DESTINATION ${KDE_INSTALL_DATADIR}/kopete_groupwise)
