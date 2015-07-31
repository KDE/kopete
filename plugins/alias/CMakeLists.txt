include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_alias_PART_SRCS aliasplugin.cpp )

add_library(kopete_alias MODULE ${kopete_alias_PART_SRCS})


target_link_libraries(kopete_alias  KF5::KDELibs4Support kopete )

install(TARGETS kopete_alias  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_alias_PART_SRCS aliaspreferences.cpp editaliasdialog.cpp )

ki18n_wrap_ui(kcm_kopete_alias_PART_SRCS aliasdialogbase.ui aliasdialog.ui )


add_library(kcm_kopete_alias MODULE ${kcm_kopete_alias_PART_SRCS})


target_link_libraries(kcm_kopete_alias   kopete )

install(TARGETS kcm_kopete_alias  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_alias.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_alias_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




