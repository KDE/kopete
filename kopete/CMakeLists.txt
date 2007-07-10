
add_subdirectory( addaccountwizard ) 
add_subdirectory( config )
add_subdirectory( chatwindow )
add_subdirectory( kconf_update ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/addaccountwizard 
${CMAKE_CURRENT_BINARY_DIR}/addaccountwizard
${CMAKE_CURRENT_SOURCE_DIR}/contactlist/ 
${CMAKE_CURRENT_BINARY_DIR}/contactlist/ 
${CMAKE_CURRENT_SOURCE_DIR}/config/plugins/ 
)

link_directories( 
${CMAKE_BINARY_DIR}/kopete/libkopete/ 
${CMAKE_CURRENT_BINARY_DIR}/addaccountwizard 
)

########### next target ###############

# Contact List
set(kopetecontactlist_SRCS 
contactlist/kopetemetacontactlvi.cpp 
contactlist/kopetestatusgroupviewitem.cpp 
contactlist/kopetegroupviewitem.cpp 
contactlist/kopetecontactlistview.cpp 
contactlist/kopetelviprops.cpp 
contactlist/kopeteaddrbookexport.cpp 
contactlist/customnotificationprops.cpp 
contactlist/kopetegrouplistaction.cpp 
contactlist/kabcexport.cpp 
)

kde4_add_ui_files(kopetecontactlist_SRCS
    contactlist/kopetegvipropswidget.ui
    contactlist/kopetemetalvipropswidget.ui
    contactlist/kopeteaddrbookexportui.ui
    contactlist/kabcexport_base.ui )

# Configure plugins
set(kopetepluginconfig_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/config/plugins/kopetepluginconfig.cpp)

set(kopete_SRCS 
   ${kopetecontactlist_SRCS}
   ${kopetepluginconfig_SRCS}
   main.cpp 
   kopeteapplication.cpp 
   systemtray.cpp 
   kopetewindow.cpp 
   kopeteaccountstatusbaricon.cpp 
#   kopeteiface.cpp 
#   kimifaceimpl.cpp 
   kopeteeditglobalidentitywidget.cpp )

kde4_add_ui_files(kopete_SRCS groupkabcselectorwidget.ui )


kde4_add_executable(kopete_bin ${kopete_SRCS})
set_target_properties(kopete_bin PROPERTIES OUTPUT_NAME kopete)

target_link_libraries(kopete_bin ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} kopete kopeteaddaccountwizard)

install(TARGETS kopete_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kopete.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kopeteui.rc kopete.notifyrc  DESTINATION ${DATA_INSTALL_DIR}/kopete)

kde4_install_icons( ${ICON_INSTALL_DIR}   )


