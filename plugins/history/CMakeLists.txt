
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_history_PART_SRCS 
   historyplugin.cpp 
   historydialog.cpp 
   historylogger.cpp 
   converter.cpp 
   historyguiclient.cpp
   historyimport.cpp )

kconfig_add_kcfg_files(kopete_history_PART_SRCS historyconfig.kcfgc )

add_library(kopete_history MODULE ${kopete_history_PART_SRCS})
set_target_properties(kopete_history PROPERTIES AUTOMOC TRUE)

target_link_libraries(kopete_history Qt5::Widgets KF5::KHtml kopete )

install(TARGETS kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_history_PART_SRCS historypreferences.cpp )

kconfig_add_kcfg_files(kcm_kopete_history_PART_SRCS historyconfig.kcfgc )


add_library(kcm_kopete_history MODULE ${kcm_kopete_history_PART_SRCS})
set_target_properties(kcm_kopete_history PROPERTIES AUTOMOC TRUE)


target_link_libraries(kcm_kopete_history KF5::KCMUtils Qt5::Widgets kopete )

install(TARGETS kcm_kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES historyconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_history.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES historyui.rc  historychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_history)
install( FILES kopete_history_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




