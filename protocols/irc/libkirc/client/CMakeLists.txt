include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../kernel
	${KOPETE_INCLUDES}
)

set(libkirc_client_SRCS
	kircclientsocket.cpp 
	kirccommand.cpp
	kirccommandhandler.cpp
#	kircclientcommandhandler.cpp 
#	kircclientcommands.cpp 
#	kirctransfer.cpp 
#	kirctransferhandler.cpp 
#	kirctransferserver.cpp 

	tasks/kircclienttask.cpp
#	tasks/kirci18ntask.cpp
)


add_library(kirc_client SHARED ${libkirc_client_SRCS})

target_link_libraries(kirc_client kirc KF5::KDELibs4Support Qt5::Core Qt5::Network)
set_target_properties(kirc_client PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS kirc_client ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

