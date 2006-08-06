include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../core
	${KOPETE_INCLUDES}
)

set(libkirc_client_SRCS
  kircclient.cpp 
  kircclientcommandhandler.cpp 
  kircclientcommands.cpp 
#	kirctransfer.cpp 
#	kirctransferhandler.cpp 
#	kirctransferserver.cpp 
)

kde4_automoc( ${libkirc_client_SRCS} )

kde4_add_library( kirc_client SHARED ${libkirc_client_SRCS} )

target_link_libraries( kirc_client
	kirc
	${KDE4_KIO_LIBS}
	${QT_QTNETWORK_LIBRARY}
)

install(TARGETS kirc_client  DESTINATION ${LIB_INSTALL_DIR})

