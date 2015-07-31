
include_directories( 
${KOPETE_INCLUDES}
${QIMAGEBLITZ_INCLUDES}
)

########### next target ###############

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS NotifyConfig)

set(kopetecontactlist_SRCS 
#kopetemetacontactitem.cpp
#kopetestatusgroupviewitem.cpp
#kopetegroupviewitem.cpp
kopetecontactlistview.cpp
kopetelviprops.cpp
kopeteaddrbookexport.cpp
customnotificationprops.cpp
kopetegrouplistaction.cpp
kabcexport.cpp
contactlistmodel.cpp
contactlisttreemodel.cpp
contactlistplainmodel.cpp
contactlistproxymodel.cpp
kopeteitemdelegate.cpp
contactlistlayoutmanager.cpp
contactlistlayoutitemconfig.cpp
)

ki18n_wrap_ui(kopetecontactlist_SRCS
kopetegvipropswidget.ui
kopetemetalvipropswidget.ui
kopeteaddrbookexportui.ui
kabcexport_page1.ui
kabcexport_page2.ui
)

add_library( kopetecontactlist SHARED ${kopetecontactlist_SRCS} )
target_link_libraries(
	kopetecontactlist
	kopete
	${KDE4_KABC_LIBS}
	KF5::NotifyConfig
	${QIMAGEBLITZ_LIBRARIES}
)
set_target_properties( kopetecontactlist PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopetecontactlist ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES DefaultContactListLayouts.xml CompactContactListLayouts.xml DESTINATION ${DATA_INSTALL_DIR}/kopete)
########### install files ###############





