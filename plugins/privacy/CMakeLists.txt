
#add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)

#common lib
set(kopete_privacy_LIB_SRCS privacymessagehandler.cpp privacyaccountlistmodel.cpp)
kde4_add_kcfg_files(kopete_privacy_LIB_SRCS privacyconfig.kcfgc )
kde4_automoc(${kopete_privacy_LIB_SRCS})
kde4_add_library( kopeteprivacy SHARED ${kopete_privacy_LIB_SRCS})
target_link_libraries( kopeteprivacy ${KDE4_KDECORE_LIBS} kopete )
install( TARGETS kopeteprivacy DESTINATION ${LIB_INSTALL_DIR})

########### next target ###############

set(kopete_privacy_PART_SRCS privacyplugin.cpp privacyguiclient.cpp)
kde4_automoc(${kopete_privacy_PART_SRCS})
kde4_add_plugin(kopete_privacy ${kopete_privacy_PART_SRCS})
target_link_libraries(kopete_privacy  ${KDE4_KDECORE_LIBS} kopete kopeteprivacy )
install(TARGETS kopete_privacy  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_privacy_PART_SRCS privacypreferences.cpp contactselectorwidget.cpp )
kde4_add_ui_files(kcm_kopete_privacy_PART_SRCS privacydialog.ui contactselectorwidget_base.ui )
kde4_automoc(${kcm_kopete_privacy_PART_SRCS})
kde4_add_plugin(kcm_kopete_privacy ${kcm_kopete_privacy_PART_SRCS})
target_link_libraries(kcm_kopete_privacy  ${KDE4_KUTILS_LIBS} kopete kopeteprivacy )
install(TARGETS kcm_kopete_privacy  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES privacyui.rc  privacychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_privacy)
install( FILES kopete_privacy.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_privacy_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
