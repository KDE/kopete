
include_directories( 
${KOPETE_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR}
)
add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )


########### next target ###############

set(akonadi_kopete_history_PART_SRCS 
   akonadihistoryplugin.cpp

)

#TODO:The UI file for search and view of chat history
kde4_add_ui_files(akonadi_kopete_history_PART_SRCS historyviewer.ui )

#TODO:The config files, to de done later
kde4_add_kcfg_files(akonadi_kopete_history_PART_SRCS historyconfig.kcfgc )


kde4_add_plugin(akonadi_kopete_history ${akonadi_kopete_history_PART_SRCS})


target_link_libraries(akonadi_kopete_history  ${KDE4_KHTML_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_AKONADI_KMIME_LIBS} 
		      kopete history)

install(TARGETS akonadi_kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

#TODO:History preference is something that we do not need at the moment
#set(kcm_akonadi_kopete_history_PART_SRCS historypreferences.cpp )
#kde4_add_ui_files(kcm_akonadi_kopete_history_PART_SRCS historyprefsui.ui )
#kde4_add_kcfg_files(kcm_akonadi_kopete_history_PART_SRCS historyconfig.kcfgc )


#kde4_add_plugin(kcm_akonadi_kopete_history ${kcm_akonadi_kopete_history_PART_SRCS})


#target_link_libraries(kcm_akonadi_kopete_history  ${KDE4_KUTILS_LIBS} kopete )

#install(TARGETS kcm_akonadi_kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#install( FILES historyconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES akonadi_kopete_history.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
#install( FILES historyui.rc  historychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/akonadi_kopete_history)
install( FILES akonadi_kopete_history_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

