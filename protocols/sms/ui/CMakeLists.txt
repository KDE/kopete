



########### next target ###############
kde4_add_ui3_files ( uiSrcs
  smsactprefs.ui
  smsuserprefs.ui
  smsadd.ui )
kde4_add_library( kopetesmsui SHARED ${uiSrcs} )
target_link_libraries(kopetesmsui  ${KDE4_KIO_LIBS} )

########### install files ###############
install(TARGETS kopetesmsui  DESTINATION ${PLUGIN_INSTALL_DIR})





