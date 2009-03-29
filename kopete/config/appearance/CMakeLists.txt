
include_directories( 
${KOPETE_INCLUDES} 
${KOPETE_SOURCE_DIR}/kopete/contactlist
${KOPETE_BINARY_DIR}/kopete/contactlist
${KOPETE_SOURCE_DIR}/kopete/config/appearance/layout
${KOPETE_BINARY_DIR}/kopete/config/appearance/layout
)

link_directories( ${KOPETE_BINARY_DIR}/kopete/contactlist )


########### next target ###############

set(kcm_kopete_appearanceconfig_PART_SRCS
   appearanceconfig.cpp 
   contactlistlayoutwidget.cpp
   contactlistlayouteditwidget.cpp
   contactlisttoken.cpp
   tooltipeditdialog.cpp
   layout/TokenDropTarget.cpp
   layout/Token.cpp
   layout/TokenPool.cpp
   layout/TokenWithLayout.cpp )

kde4_add_ui_files(kcm_kopete_appearanceconfig_PART_SRCS 
   appearanceconfig_colors.ui
   appearanceconfig_contactlist.ui
   appearanceconfig_advanced.ui
   contactlistlayoutwidget.ui
   tooltipeditwidget.ui )

kde4_add_plugin(kcm_kopete_appearanceconfig ${kcm_kopete_appearanceconfig_PART_SRCS})


target_link_libraries(kcm_kopete_appearanceconfig ${KDE4_KUTILS_LIBS} kopete kopetecontactlist )

install(TARGETS kcm_kopete_appearanceconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_appearanceconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})



