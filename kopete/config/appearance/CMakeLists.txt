
include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_SOURCE_DIR}/kopete/kopete/contactlist
${CMAKE_BINARY_DIR}/kopete/kopete/contactlist
${CMAKE_SOURCE_DIR}/kopete/kopete/config/appearance/layout
${CMAKE_BINARY_DIR}/kopete/kopete/config/appearance/layout
)

link_directories( ${CMAKE_BINARY_DIR}/kopete/kopete/contactlist )


########### next target ###############

set(kcm_kopete_appearanceconfig_PART_SRCS
   appearanceconfig.cpp 
   contactlistlayoutwidget.cpp
   contactlistlayouteditwidget.cpp
   contactlisttoken.cpp
   tooltipeditdialog.cpp
   layout/DragStack.cpp
   layout/Token.cpp
   layout/TokenListWidget.cpp
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



