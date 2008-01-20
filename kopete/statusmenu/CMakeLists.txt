
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set( kopetestatusmenu_SRCS
  kopetestatuseditaction.cpp
  kopetestatusgroupaction.cpp
  kopetestatusaction.cpp
  kopeteonlinestatusaction.cpp
  kopetestatusrootaction.cpp
)

kde4_add_ui_files( kopetestatusmenu_SRCS
  kopetestatuseditwidget_base.ui
)

kde4_add_library( kopetestatusmenu SHARED ${kopetestatusmenu_SRCS} )
target_link_libraries( kopetestatusmenu kopete ${KDE4_KDEUI_LIBS} )
set_target_properties( kopetestatusmenu PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopetestatusmenu DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############





