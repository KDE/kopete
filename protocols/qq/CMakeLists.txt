
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
)


########### next target ###############

set(kopete_qq_PART_SRCS 
   qqprotocol.cpp 
   qqcontact.cpp 
   qqaccount.cpp 
   qqaddcontactpage.cpp 
   qqeditaccountwidget.cpp 
   qqfakeserver.cpp 
   qqincomingmessage.cpp 
   ui/qqwebcamdialog.cpp 
)

kde4_automoc(${kopete_qq_PART_SRCS})

kde4_add_ui_files(kopete_qq_PART_SRCS
qqaddui.ui
)

kde4_add_ui_files(kopete_qq_PART_SRCS
qqaccountpreferences.ui
)

kde4_add_plugin(kopete_qq ${kopete_qq_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_qq)

target_link_libraries(kopete_qq ${KDE4_KIO_LIBS} kopete kopete_videodevice )

install_targets(${PLUGIN_INSTALL_DIR} kopete_qq )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_qq.desktop )
