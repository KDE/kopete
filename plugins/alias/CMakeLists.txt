include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_alias_PART_SRCS aliasplugin.cpp )

kde4_add_plugin(kopete_alias ${kopete_alias_PART_SRCS})


target_link_libraries(kopete_alias  ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kopete )

install(TARGETS kopete_alias  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_alias_PART_SRCS aliaspreferences.cpp editaliasdialog.cpp )

kde4_add_ui_files(kcm_kopete_alias_PART_SRCS aliasdialogbase.ui aliasdialog.ui )


kde4_add_plugin(kcm_kopete_alias ${kcm_kopete_alias_PART_SRCS})


target_link_libraries(kcm_kopete_alias  ${KDE4_KUTILS_LIBS} kopete ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kcm_kopete_alias  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_alias.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_alias_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




