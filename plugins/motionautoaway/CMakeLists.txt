



########### next target ###############

set(kopete_motionaway_PART_SRCS motionawayplugin.cpp )

kde4_add_kcfg_files(kopete_motionaway_PART_SRCS motionawayconfig.kcfgc )

kde4_add_plugin(kopete_motionaway ${kopete_motionaway_PART_SRCS})


target_link_libraries(kopete_motionaway  ${KDE4_KDECORE_LIBS} kopete )

install(TARGETS kopete_motionaway  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_motionaway_PART_SRCS motionawaypreferences.cpp )

kde4_automoc(${kcm_kopete_motionaway_PART_SRCS})

kde4_add_ui_files(kcm_kopete_motionaway_PART_SRCS motionawayprefs.ui )

kde4_add_kcfg_files(kcm_kopete_motionaway_PART_SRCS motionawayconfig.kcfgc )

kde4_add_plugin(kcm_kopete_motionaway ${kcm_kopete_motionaway_PART_SRCS})


target_link_libraries(kcm_kopete_motionaway  ${KDE4_KDECORE_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_motionaway  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES motionawayconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_motionaway.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_motionaway_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




