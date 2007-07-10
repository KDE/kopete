add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES}
)


########### next target ###############

set(kopete_cryptography_PART_SRCS 
   cryptographyplugin.cpp 
   kgpginterface.cpp 
   cryptographyguiclient.cpp 
   cryptographyselectuserkey.cpp 
   popuppublic.cpp 
   selectkeydialog.cpp
   cryptographyconfig.cpp )

kde4_add_ui_files(kopete_cryptography_PART_SRCS 
		  cryptographyuserkey_ui.ui 
		  popuppublicbase.ui)

kde4_add_plugin(kopete_cryptography ${kopete_cryptography_PART_SRCS})


target_link_libraries(kopete_cryptography  ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kopete )

install(TARGETS kopete_cryptography  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_cryptography_PART_SRCS cryptographypreferences.cpp selectkeydialog.cpp cryptographyconfig.cpp )

kde4_add_ui_files(kcm_kopete_cryptography_PART_SRCS cryptographyprefsbase.ui )


kde4_add_plugin(kcm_kopete_cryptography ${kcm_kopete_cryptography_PART_SRCS})


target_link_libraries(kcm_kopete_cryptography ${KDE4_KUTILS_LIBS} kopete )

install(TARGETS kcm_kopete_cryptography  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_cryptography.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES cryptographyui.rc cryptographychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_cryptography)
install( FILES kopete_cryptography_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
