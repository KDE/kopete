



########### next target ###############

set(kcm_kopete_msn_PART_SRCS msnpreferences.cpp )

kde4_automoc(${kcm_kopete_msn_PART_SRCS})

kde4_add_ui_files(kcm_kopete_msn_PART_SRCS msnprefs.ui )

kde4_add_plugin(kcm_kopete_msn ${kcm_kopete_msn_PART_SRCS})


target_link_libraries(kcm_kopete_msn  ${KDE4_KDECORE_LIBS} kopete ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_msn  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_msn_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




