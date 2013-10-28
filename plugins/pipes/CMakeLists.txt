
include_directories ( ${KOPETE_INCLUDES} )

#add_subdirectory ( tests )

########### next target ###############

set (kopete_pipes_PART_SRCS 
 pipesplugin.cpp
 pipesconfig.cpp )

kde4_add_plugin( kopete_pipes ${kopete_pipes_PART_SRCS})

target_link_libraries( kopete_pipes ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} kopete )

install( TARGETS kopete_pipes DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set ( kcm_kopete_pipes_PART_SRCS
  pipespreferences.cpp
  pipesdelegate.cpp
  pipesmodel.cpp
  pipesconfig.cpp )

kde4_add_ui_files( kcm_kopete_pipes_PART_SRCS pipesprefsbase.ui )

kde4_add_plugin( kcm_kopete_pipes ${kcm_kopete_pipes_PART_SRCS} )

target_link_libraries( kcm_kopete_pipes ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} kopete )

install( TARGETS kcm_kopete_pipes  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kopete_pipes.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kopete_pipes_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )

