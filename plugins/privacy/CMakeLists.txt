
#add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_privacy_PART_SRCS privacyplugin.cpp privacymessagehandler.cpp privacypreferences.cpp)

kde4_add_kcfg_files(kopete_privacy_PART_SRCS privacyconfig.kcfgc )

kde4_automoc(${kopete_privacy_PART_SRCS})

kde4_add_plugin(kopete_privacy ${kopete_privacy_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_privacy )

target_link_libraries(kopete_privacy  ${KDE4_KDECORE_LIBS} kopete )

install(TARGETS kopete_privacy  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_privacy_PART_SRCS privacypreferences.cpp )

kde4_add_ui_files(kcm_kopete_privacy_PART_SRCS privacydialog.ui )

kde4_add_kcfg_files(kcm_kopete_privacy_PART_SRCS privacyconfig.kcfgc )

kde4_automoc(${kcm_kopete_privacy_PART_SRCS})

kde4_add_plugin(kcm_kopete_privacy ${kcm_kopete_privacy_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_privacy )

target_link_libraries(kcm_kopete_privacy  ${KDE4_KUTILS_LIBS} kopete )

install(TARGETS kcm_kopete_privacy  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_privacy.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_privacy_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)