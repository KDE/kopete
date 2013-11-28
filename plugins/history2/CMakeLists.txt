include_directories(
${KOPETE_INCLUDES}
)


########### next target ###############

set(kopete_history2_PART_SRCS
   history2plugin.cpp
   history2dialog.cpp
   history2logger.cpp
   history2guiclient.cpp
   history2import.cpp
)

kde4_add_ui_files(kopete_history2_PART_SRCS history2viewer.ui )

kde4_add_kcfg_files(kopete_history2_PART_SRCS history2config.kcfgc )


kde4_add_plugin(kopete_history2 ${kopete_history2_PART_SRCS})


target_link_libraries(kopete_history2  ${KDE4_KHTML_LIBS} kopete ${QT_QTSQL_LIBRARY})

install(TARGETS kopete_history2  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_history2_PART_SRCS history2preferences.cpp history2logger.cpp history2import.cpp)

kde4_add_ui_files(kcm_kopete_history2_PART_SRCS history2prefsui.ui )

kde4_add_kcfg_files(kcm_kopete_history2_PART_SRCS history2config.kcfgc )


kde4_add_plugin(kcm_kopete_history2 ${kcm_kopete_history2_PART_SRCS})


target_link_libraries(kcm_kopete_history2  ${KDE4_KUTILS_LIBS} ${KDE4_KHTML_LIBS} kopete ${QT_QTSQL_LIBRARY})

install(TARGETS kcm_kopete_history2  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES history2config.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_history2.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES history2ui.rc  history2chatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_history2)
install( FILES kopete_history2_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




