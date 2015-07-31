
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/client
	${CMAKE_CURRENT_SOURCE_DIR}/kernel
	${KOPETE_INCLUDES}
)

set(libkirc_SRCS
	kernel/kircbytearrayescaper.cpp
#	kernel/kirccommand.cpp
	kernel/kircconst.cpp 
	kernel/kirccontext.cpp
	kernel/kircentity.cpp
	kernel/kircevent.cpp 
	kernel/kirchandler.cpp
	kernel/kirchandlerfactory.cpp
	kernel/kirchandlerplugin.cpp
	kernel/kircmessage.cpp
	kernel/kircmessageutil.cpp 
	kernel/kircsocket.cpp 
	kernel/kircstdmessages.cpp 
)

add_library(kirc SHARED ${libkirc_SRCS})

target_link_libraries(kirc Qt5::Core Qt5::Network KF5::KDELibs4Support)
set_target_properties(kirc PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS kirc ${INSTALL_TARGETS_DEFAULT_ARGS})

set(libkirc_client_SRCS
	client/kircclienthandler.cpp

	client/kircclientchannelhandler.cpp
	client/kircclientlisthandler.cpp
	client/kircclientmotdhandler.cpp
	client/kircclientpingponghandler.cpp
	client/kircclientwhohandler.cpp

	client/kircclientsocket.cpp 
)

add_library(kirc_client SHARED ${libkirc_client_SRCS})

target_link_libraries(kirc_client kirc KF5::KDELibs4Support Qt5::Core Qt5::Network)
set_target_properties(kirc_client PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS kirc_client ${INSTALL_TARGETS_DEFAULT_ARGS})

# add_subdirectory( plugins )

