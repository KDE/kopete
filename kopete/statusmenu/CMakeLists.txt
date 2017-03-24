
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

ki18n_wrap_ui( kopetestatusmenu_SRCS
    kopetestatuseditwidget_base.ui
    )

add_library( kopetestatusmenu SHARED ${kopetestatusmenu_SRCS} )
target_link_libraries( kopetestatusmenu kopete  KF5::KCMUtils )
set_target_properties( kopetestatusmenu PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopetestatusmenu ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





