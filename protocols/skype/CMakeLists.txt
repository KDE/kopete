project( kopete_skype )

########### next target ###############

include_directories(
	${QT_QTDBUS_INCLUDE_DIR}

	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/libskype
	${CMAKE_CURRENT_SOURCE_DIR}/libskype/skypedbus
	${CMAKE_CURRENT_SOURCE_DIR}/skypebuttons
	${CMAKE_CURRENT_SOURCE_DIR}/skypebuttons/qtbrowserplugin

	${CMAKE_CURRENT_BUILD_DIR}
)

include(ECMOptionalAddSubdirectory)

add_definitions(-DSKYPE_DEBUG_GLOBAL=14311)

########### next target ###############

set(
	skypedbus_SRCS
	libskype/skypedbus/skypeconnection.cpp
)

qt5_add_dbus_adaptor(
	skypedbus_SRCS
	libskype/skypedbus/com.Skype.API.Client.xml
	libskype/skypedbus/skypeconnection.h
	SkypeConnection
	skypeadaptor
	SkypeAdaptor
)

set(
	libskype_SRCS

	libskype/skype.cpp
	libskype/skypewindow.cpp
)

set(
	kopete_skype_ui_SRCS
)

ki18n_wrap_ui(
	kopete_skype_ui_SRCS
	ui/skypeaddcontactbase.ui
	ui/skypecalldialogbase.ui
	ui/skypedetailsbase.ui
	ui/skypeeditaccountbase.ui
)

set(
	kopete_skype_SRCS
	${skypedbus_SRCS}
	${libskype_SRCS}
	${kopete_skype_ui_SRCS}
	skypeaccount.cpp
	skypeaddcontact.cpp
	skypecalldialog.cpp
#	skypeconference.cpp
	skypecontact.cpp
	skypedetails.cpp
	skypeeditaccount.cpp
	skypechatsession.cpp
	skypeprotocol.cpp
)

add_library( kopete_skype MODULE ${kopete_skype_SRCS} )

target_link_libraries(
	kopete_skype
	kopete
	
	${X11_X11_LIB}
)

########### next target ###############

add_subdirectory( icons )
ecm_optional_add_subdirectory( skypebuttons )

########### install files ###############

install( TARGETS kopete_skype DESTINATION ${KDE_INSTALL_PLUGINDIR} )
install( FILES kopete_skype.desktop skype.protocol callto.protocol tel.protocol DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install( FILES skypeui.rc skypechatui.rc DESTINATION ${KDE_INSTALL_DATADIR}/kopete_skype )

#Enable this for system bus dbus type communication with skype public api
#Linux skype client (version 2.0.0.72) still does not support it, so we do not need it now maybe in new skype version
#if (DBUS_SYSTEM_POLICY_DIR)
#	install( FILES libskype/skypedbus/com.Skype.API.Client.conf DESTINATION ${DBUS_SYSTEM_POLICY_DIR})
#else (DBUS_SYSTEM_POLICY_DIR)
#	message(STATUS "It is necessary to specify the directory where the client policy file for com.Skype.API.Client.conf is installed as DBUS_SYSTEM_POLICY_DIR.  Normally this is /etc/dbus-1/system.d on a Linux system")
#endif (DBUS_SYSTEM_POLICY_DIR)
