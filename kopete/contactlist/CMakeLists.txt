
include_directories( 
${KOPETE_INCLUDES}
${QIMAGEBLITZ_INCLUDES}
)

########### next target ###############

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

kde4_add_ui_files(kopetecontactlist_SRCS
kopetegvipropswidget.ui
kopetemetalvipropswidget.ui
kopeteaddrbookexportui.ui
kabcexport_page1.ui
kabcexport_page2.ui
)

kde4_add_library( kopetecontactlist SHARED ${kopetecontactlist_SRCS} )
target_link_libraries( kopetecontactlist kopete ${KDE4_KDEUI_LIBS}
${KDE4_KDE3SUPPORT_LIBS}
${KDE4_KABC_LIBS}
${KDE4_KNOTIFYCONFIG_LIBS}
${QIMAGEBLITZ_LIBRARIES})
set_target_properties( kopetecontactlist PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopetecontactlist ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES DefaultContactListLayouts.xml CompactContactListLayouts.xml DESTINATION ${DATA_INSTALL_DIR}/kopete)
########### install files ###############





