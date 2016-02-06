
include_directories( 
${KOPETE_INCLUDES} 
${KOPETE_SOURCE_DIR}/kopete/contactlist
${KOPETE_BINARY_DIR}/kopete/contactlist
${KOPETE_SOURCE_DIR}/kopete/config/appearance/layout
${KOPETE_BINARY_DIR}/kopete/config/appearance/layout
)

link_directories( ${KOPETE_BINARY_DIR}/kopete/contactlist )


########### next target ###############

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS NotifyConfig)

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

ki18n_wrap_ui(kcm_kopete_appearanceconfig_PART_SRCS 
   appearanceconfig_colors.ui
   appearanceconfig_contactlist.ui
   appearanceconfig_advanced.ui
   contactlistlayoutwidget.ui
   tooltipeditwidget.ui )

add_library(kcm_kopete_appearanceconfig MODULE ${kcm_kopete_appearanceconfig_PART_SRCS})


target_link_libraries(kcm_kopete_appearanceconfig KF5::KHtml kopete kopetecontactlist )

install(TARGETS kcm_kopete_appearanceconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_appearanceconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})



