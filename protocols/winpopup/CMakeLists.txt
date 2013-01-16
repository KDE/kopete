
#add_subdirectory( ui ) 
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/libwinpopup/ 
)


########### next target ###############

set(kopete_wp_ui_SRCS)

kde4_add_ui_files(kopete_wp_ui_SRCS 
ui/wpaddcontactbase.ui 
ui/wpuserinfowidget.ui 
ui/wpeditaccountbase.ui 
)

set(libwinpopup_SRCS 
libwinpopup/libwinpopup.cpp 
)

set(kopete_wp_PART_SRCS 
   ${kopete_wp_ui_SRCS} 
   ${libwinpopup_SRCS} 
   wpprotocol.cpp 
   wpcontact.cpp 
   wpaddcontact.cpp 
   wpeditaccount.cpp 
   wpaccount.cpp 
   wpuserinfo.cpp )


kde4_add_plugin(kopete_wp ${kopete_wp_PART_SRCS})


target_link_libraries(kopete_wp  ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_wp  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_wp.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(PROGRAMS winpopup-send winpopup-install  DESTINATION ${BIN_INSTALL_DIR})



