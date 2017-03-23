
add_subdirectory( addaccountwizard )
add_subdirectory( statusmenu )
add_subdirectory( identity )
add_subdirectory( contactlist )
add_subdirectory( config )
add_subdirectory( chatwindow )
add_subdirectory( kconf_update ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/addaccountwizard 
${CMAKE_CURRENT_BINARY_DIR}/addaccountwizard
${CMAKE_CURRENT_SOURCE_DIR}/statusmenu/
${CMAKE_CURRENT_BINARY_DIR}/statusmenu/
${CMAKE_CURRENT_SOURCE_DIR}/identity 
${CMAKE_CURRENT_BINARY_DIR}/identity
${CMAKE_CURRENT_SOURCE_DIR}/contactlist/
${CMAKE_CURRENT_BINARY_DIR}/contactlist/
${CMAKE_CURRENT_SOURCE_DIR}/config/plugins/
${QIMAGEBLITZ_INCLUDES}
)

link_directories( 
${KOPETE_BINARY_DIR}/libkopete
${CMAKE_CURRENT_BINARY_DIR}/addaccountwizard 
${CMAKE_CURRENT_BINARY_DIR}/identity
${CMAKE_CURRENT_BINARY_DIR}/contactlist
)

########### next target ###############

# Contact List

set(kopete_SRCS 
   ${kopetecontactlist_SRCS}
   main.cpp
   kopeteapplication.cpp
   systemtray.cpp
   kopetewindow.cpp
   kopeteidentitystatusbaricon.cpp
   kopeteaccountstatusbaricon.cpp
   kopetedbusinterface.cpp
   kopetedbusinterface_p.cpp
   infoeventwidget.cpp
   )

qt5_add_dbus_adaptor(kopete_SRCS org.kde.Kopete.xml kopetedbusinterface.h KopeteDBusInterface)

ki18n_wrap_ui(kopete_SRCS groupkabcselectorwidget.ui infoeventbase.ui )

file(GLOB ICONS_SRCS "hi*-apps-kopete.png")
ecm_add_app_icon(kopete_SRCS ICONS ${ICONS_SRCS})

add_executable(kopete_bin ${kopete_SRCS})
set_target_properties(kopete_bin PROPERTIES RUNTIME_OUTPUT_NAME kopete)

target_link_libraries(kopete_bin
	KF5::Contacts
	KF5::TextEditor
	KF5::NotifyConfig
	${KDE4_SOLID_LIBRARY}
	${QIMAGEBLITZ_LIBRARIES}
	kopete
	kopeteaddaccountwizard
	kopetestatusmenu
	kopetecontactlist
	kopeteidentity
)

install(TARGETS kopete_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kopete.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
    kopeteui.rc
    DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/kopete)

install( FILES kopete.notifyrc  DESTINATION ${DATA_INSTALL_DIR}/kopete)
install( FILES kopeterc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES org.kde.Kopete.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

ecm_install_icons( ${ICON_INSTALL_DIR}   )

