########### next target ###############

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default" )

string( LENGTH "$ENV{HOME}" LENGTH_OF_HOME )
if( LENGTH_OF_HOME GREATER 0 )
	string( REGEX MATCHALL ^$ENV{HOME}.*$ IS_HOME ${CMAKE_INSTALL_PREFIX} )
else( LENGTH_OF_HOME GREATER 0 )
	set( IS_HOME FALSE )
endif( LENGTH_OF_HOME GREATER 0 )
string( COMPARE EQUAL /usr ${CMAKE_INSTALL_PREFIX} IS_USR )
string( COMPARE EQUAL /usr/local ${CMAKE_INSTALL_PREFIX} IS_LOCAL )

if( NOT MOZPLUGIN_INSTALL_DIR )
	if( IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR $ENV{HOME}/.mozilla/plugins )
	endif( IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
	if( NOT IS_HOME AND IS_USR AND NOT IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR /usr/lib${LIB_SUFFIX}/mozilla/plugins )
	endif( NOT IS_HOME AND IS_USR AND NOT IS_LOCAL )
	if( NOT IS_HOME AND NOT IS_USR AND IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR /usr/local/netscape/plugins )
	endif( NOT IS_HOME AND NOT IS_USR AND IS_LOCAL )
	if( NOT IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/mozilla/plugins )
	endif( NOT IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
endif( NOT MOZPLUGIN_INSTALL_DIR )

mark_as_advanced( MOZPLUGIN_INSTALL_DIR )

message( STATUS "Mozilla plugin install dir is \"${MOZPLUGIN_INSTALL_DIR}\" to change use parm -DMOZPLUGIN_INSTALL_DIR=<directory>" )

#TODO: Check if QtBrowerPlugin (or QtNsPlugin?) installed and use it

set( skypebuttons_SRCS
	skypebuttons.cpp
	qtbrowserplugin/qtbrowserplugin.cpp
	qtbrowserplugin/qtbrowserplugin_x11.cpp
)

QT4_GENERATE_MOC( ${CMAKE_CURRENT_SOURCE_DIR}/skypebuttons.cpp ${CMAKE_CURRENT_BINARY_DIR}/skypebuttons.moc )

kde4_add_plugin( skypebuttons ${skypebuttons_SRCS} ${qtbrowserplugin_SRC} )
target_link_libraries( skypebuttons ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

########### install files ###############

install( TARGETS skypebuttons DESTINATION ${MOZPLUGIN_INSTALL_DIR} )

