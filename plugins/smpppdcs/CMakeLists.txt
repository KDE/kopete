
add_subdirectory( icons ) 
add_subdirectory( libsmpppdclient ) 
add_subdirectory( unittest ) 




########### next target ###############

set(kopete_smpppdcs_PART_SRCS 
   smpppdcsplugin.cpp 
   onlineinquiry.cpp 
   detectordcop.cpp 
   detectorsmpppd.cpp 
   detectornetstat.cpp 
   detectornetworkstatus.cpp )

message(FATAL_ERROR "DCOP stuff was removed in kde4, convert it to dbus please")

#kde4_add_dcop_skels(kopete_smpppdcs_PART_SRCS smpppdcsiface.h )

#kde4_add_dcop_stubs(kopete_smpppdcs_PART_SRCS kinternetiface.h )

kde4_add_kcfg_files(kopete_smpppdcs_PART_SRCS smpppdcsconfig.kcfgc )

kde4_add_plugin(kopete_smpppdcs ${kopete_smpppdcs_PART_SRCS})


target_link_libraries(kopete_smpppdcs  ${KDE4_KDECORE_LIBS} smpppdclient kopete )

install(TARGETS kopete_smpppdcs  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_smpppdcs_PART_SRCS 
   smpppdcspreferences.cpp 
   smpppdsearcher.cpp 
   smpppdcsprefsimpl.cpp 
   smpppdlocationwidget.cpp )


kde4_add_ui_files(kcm_kopete_smpppdcs_PART_SRCS smpppdcsprefs.ui smpppdlocationui.ui )

kde4_add_kcfg_files(kcm_kopete_smpppdcs_PART_SRCS smpppdcsconfig.kcfgc )

kde4_add_plugin(kcm_kopete_smpppdcs ${kcm_kopete_smpppdcs_PART_SRCS})


target_link_libraries(kcm_kopete_smpppdcs  ${KDE4_KDECORE_LIBS} smpppdclient kopete ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_smpppdcs  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES smpppdcs.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_smpppdcs.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_smpppdcs_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




