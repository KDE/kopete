
include_directories(
${KOPETE_INCLUDES}

)


########### next target ###############

set(kcm_kopete_avdeviceconfig_PART_SRCS
  avdeviceconfig.cpp IdGuiElements.cpp )
# avdeviceconfig_videodevicec.cpp )

kde4_add_ui_files(kcm_kopete_avdeviceconfig_PART_SRCS avdeviceconfig_videodevice.ui )

kde4_add_plugin(kcm_kopete_avdeviceconfig ${kcm_kopete_avdeviceconfig_PART_SRCS})


target_link_libraries(kcm_kopete_avdeviceconfig  ${KDE4_KUTILS_LIBS} kopete kopete_videodevice)

install(TARGETS kcm_kopete_avdeviceconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_avdeviceconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

kde4_install_icons( ${DATA_INSTALL_DIR}/kopete/icons  )
