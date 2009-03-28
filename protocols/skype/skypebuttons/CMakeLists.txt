########### next target ###############

include ( ${QT_USE_FILE} )

set( skypeactionhandler_SRCS
	main.cpp
)

kde4_add_executable( skypeactionhandler ${skypeactionhandler_SRCS} )
set_target_properties( skypeactionhandler PROPERTIES OUTPUT_NAME skype-action-handler )
target_link_libraries( skypeactionhandler ${QT_LIBRARIES} ${QT_QTDBUS_LIBRARY} )

########### next target ###############

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default" )

if ( NOT MOZPLUGIN_INSTALL_DIR )
	set( MOZPLUGIN_INSTALL_DIR /usr/lib/firefox/plugins )
endif( NOT MOZPLUGIN_INSTALL_DIR )

message( STATUS "Mozilla plugin install dir is \"${MOZPLUGIN_INSTALL_DIR}\" to change use parm -DMOZPLUGIN_INSTALL_DIR=<directory>" )

set( skypebuttons_SRCS
	skypebuttons.cpp
	qtbrowserplugin/qtbrowserplugin.cpp
	qtbrowserplugin/qtbrowserplugin_x11.cpp
)

kde4_add_plugin( skypebuttons ${skypebuttons_SRCS} )
target_link_libraries( skypebuttons ${QT_LIBRARIES} )

########### install files ###############

install( TARGETS skypebuttons DESTINATION ${MOZPLUGIN_INSTALL_DIR} )
install( TARGETS skypeactionhandler ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES skype.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES callto.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES tel.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
