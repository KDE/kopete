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

ki18n_wrap_ui(kopete_history2_PART_SRCS history2viewer.ui )

kconfig_add_kcfg_files(kopete_history2_PART_SRCS history2config.kcfgc )


add_library(kopete_history2 MODULE ${kopete_history2_PART_SRCS})


target_link_libraries(kopete_history2  KF5::KHtml kopete Qt5::Sql)

install(TARGETS kopete_history2  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_history2_PART_SRCS history2preferences.cpp history2logger.cpp history2import.cpp)

ki18n_wrap_ui(kcm_kopete_history2_PART_SRCS history2prefsui.ui )

kconfig_add_kcfg_files(kcm_kopete_history2_PART_SRCS history2config.kcfgc )


add_library(kcm_kopete_history2 MODULE ${kcm_kopete_history2_PART_SRCS})


target_link_libraries(kcm_kopete_history2   KF5::KHtml kopete Qt5::Sql)

install(TARGETS kcm_kopete_history2  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES history2config.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES kopete_history2.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES history2ui.rc  history2chatui.rc  DESTINATION ${KDE_INSTALL_DATADIR}/kopete_history2)
install( FILES kopete_history2_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)




