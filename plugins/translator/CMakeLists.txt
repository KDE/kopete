
include_directories(
${KOPETE_INCLUDES}
)


########### next target ###############

set(kopete_translator_PART_SRCS
   translatorplugin.cpp
   translatordialog.cpp
   translatorguiclient.cpp
   translatorlanguages.cpp )

kconfig_add_kcfg_files(kopete_translator_PART_SRCS translatorconfig.kcfgc )


add_library(kopete_translator MODULE ${kopete_translator_PART_SRCS})


target_link_libraries(kopete_translator  KF5::KIOCore kopete )

install(TARGETS kopete_translator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_translator_PART_SRCS translatorpreferences.cpp translatorlanguages.cpp )

ki18n_wrap_ui(kcm_kopete_translator_PART_SRCS translatorprefsbase.ui )

kconfig_add_kcfg_files(kcm_kopete_translator_PART_SRCS translatorconfig.kcfgc )


add_library(kcm_kopete_translator MODULE ${kcm_kopete_translator_PART_SRCS})


target_link_libraries(kcm_kopete_translator   kopete )

install(TARGETS kcm_kopete_translator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES translatorconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_translator.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES translatorui.rc translatorchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_translator)
install( FILES kopete_translator_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




