#add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

#add_subdirectory( ui ) 
#add_subdirectory( icons ) 
#add_subdirectory( libiris ) 

#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(JINGLE)")

include_directories( 
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/include/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/jabber/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-core/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-im/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/util/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/network/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/
${CMAKE_CURRENT_SOURCE_DIR}/../
${CMAKE_CURRENT_SOURCE_DIR}/../tasks/
${CMAKE_CURRENT_SOURCE_DIR}/../kioslave/
${QCA2_INCLUDE_DIR}
${IDN_INCLUDE_DIR}
${ZLIB_INCLUDE_DIR}
)

#link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../libiris/iris/xmpp-im ./)


########### next target ###############

set(kopete_jabber_jingle_ui_SRCS
#	ui/jinglevoicesessiondialogbase.ui
	contentdialog.ui
)

QT4_WRAP_UI(kopete_jabber_jingle_ui_PART ${kopete_jabber_jingle_ui_SRCS})

set(kopete_jabber_jingle_PART_SRCS 
#	${kopete_jabber_jingle_ui_SRCS}
	jingleaudiomanager.cpp
	jinglecallsmanager.cpp
	contentdialog.cpp
#	jinglesessionmanager.cpp --> moved to iris !!!
)

set (kopete_jingle_Q_OBJECT_FILES
	jingleaudiomanager.h
	jinglecallsmanager.h
	contentdialog.h
#	jinglesessionmanager.h --> moved to iris !!!
)

qt4_wrap_cpp(kopete_jingle_MOC_FILES ${kopete_jingle_Q_OBJECT_FILES})

kde4_add_library(kopete_jabberjingle STATIC
	${kopete_jabber_jingle_PART_SRCS}
	${kopete_jingle_MOC_FILES}
	${kopete_jabber_jingle_ui_PART}
)

link_directories(
	${CMAKE_BINARY_DIR}/lib/
)

target_link_libraries(kopete_jabberjingle
#	${QCA2_LIBRARIES} speex ortp glib-2.0 gmodule-2.0 gthread-2.0 asound expat kopete iris_kopete 
	kopete iris_kopete
)


install(TARGETS kopete_jabberjingle  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

#install( FILES kopete_jabber.desktop xmpp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
#install( FILES jabberchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_jabber)
