



########### next target ###############
#kde4_add_ui3_files ( uiSrcs
#  smsactprefs.ui
#  smsuserprefs.ui
#  smsadd.ui )
add_library( kopetesmsui SHARED ${uiSrcs} )
target_link_libraries(kopetesmsui  KF5::KIOCore )

########### install files ###############
install(TARGETS kopetesmsui  DESTINATION ${PLUGIN_INSTALL_DIR})





