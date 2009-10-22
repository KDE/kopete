
include_directories( 
${KOPETE_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR}
)
add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )


########### next target ###############

set(kopete_history_PART_SRCS 
   historyplugin.cpp 
   historydialog.cpp 
   historylogger.cpp 
   converter.cpp 
   historyguiclient.cpp
   historyimport.cpp
   migrator.cpp
 #  gethistoryjob.cpp
)

kde4_add_ui_files(kopete_history_PART_SRCS historyviewer.ui )

kde4_add_kcfg_files(kopete_history_PART_SRCS historyconfig.kcfgc )


kde4_add_plugin(kopete_history ${kopete_history_PART_SRCS})


target_link_libraries(kopete_history  ${KDE4_KHTML_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_AKONADI_KMIME_LIBS} 
		      kopete history)

install(TARGETS kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_history_PART_SRCS historypreferences.cpp )

kde4_add_ui_files(kcm_kopete_history_PART_SRCS historyprefsui.ui )

kde4_add_kcfg_files(kcm_kopete_history_PART_SRCS historyconfig.kcfgc )


kde4_add_plugin(kcm_kopete_history ${kcm_kopete_history_PART_SRCS})


target_link_libraries(kcm_kopete_history  ${KDE4_KUTILS_LIBS} kopete )

install(TARGETS kcm_kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES historyconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_history.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES historyui.rc  historychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_history)
install( FILES kopete_history_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




