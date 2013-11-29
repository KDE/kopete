
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

QT4_ADD_DBUS_ADAPTOR(kopete_SRCS org.kde.Kopete.xml kopetedbusinterface.h KopeteDBusInterface)

kde4_add_ui_files(kopete_SRCS groupkabcselectorwidget.ui infoeventbase.ui )

kde4_add_app_icon(kopete_SRCS "hi*-apps-kopete.png")

kde4_add_executable(kopete_bin ${kopete_SRCS})
set_target_properties(kopete_bin PROPERTIES RUNTIME_OUTPUT_NAME kopete)

target_link_libraries(kopete_bin 
	${KDE4_KTEXTEDITOR_LIBS} 
	${KDE4_KNOTIFYCONFIG_LIBS}
	${KDE4_KDE3SUPPORT_LIBS}
	${KDE4_KABC_LIBS}
	${KDE4_SOLID_LIBRARY}
	${QIMAGEBLITZ_LIBRARIES}
	kopete 
	kopeteaddaccountwizard
	kopetestatusmenu
	kopetecontactlist
	kopeteidentity)

install(TARGETS kopete_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kopete.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kopeteui.rc kopete.notifyrc  DESTINATION ${DATA_INSTALL_DIR}/kopete)
install( FILES kopeterc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES org.kde.Kopete.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

