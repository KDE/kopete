
include_directories( 
${KOPETE_INCLUDES} 
)

########### next target ###############

set(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfig.cpp globalidentitiesmanager.cpp )

kde4_add_ui_files(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfigbase.ui )

kde4_add_kcfg_files(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfigpreferences.kcfgc )

kde4_automoc(kcm_kopete_identityconfig ${kcm_kopete_identityconfig_PART_SRCS})

kde4_add_plugin(kcm_kopete_identityconfig ${kcm_kopete_identityconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_identityconfig )

target_link_libraries(kcm_kopete_identityconfig  ${KDE4_KUTILS_LIBS} ${KDE4_KABC_LIBS} kopete)

install(TARGETS kcm_kopete_identityconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopeteidentityconfigpreferences.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_identityconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

