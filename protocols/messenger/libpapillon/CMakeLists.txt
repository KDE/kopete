
# Propagate includes to tests subdirs
set(PAPILLON_INCLUDES 
${CMAKE_CURRENT_SOURCE_DIR}/base/ 
${CMAKE_CURRENT_SOURCE_DIR}/http/ 
${CMAKE_CURRENT_SOURCE_DIR}/tasks/ 
${QCA2_INCLUDE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory( tests )

include_directories(${PAPILLON_INCLUDES})


########### next target ###############

set(papillon_base_SRCS 
base/bytestream.cpp 
base/connector.cpp 
base/safedelete.cpp 
base/stream.cpp 
)

set(papillon_http_SRCS 
http/securestream.cpp 
http/tweenerhandler.cpp 
http/httptransfer.cpp 
http/httpcoreprotocol.cpp 
)

set(papillon_tasks_SRCS 
tasks/logintask.cpp 
tasks/notifymessagetask.cpp 
tasks/setpresencetask.cpp 
)

set(libpapillon_SRCS 
${papillon_base_SRCS} 
${papillon_http_SRCS} 
${papillon_tasks_SRCS} 
task.cpp 
messengercoreprotocol.cpp 
transfer.cpp 
papillonclientstream.cpp 
qtconnector.cpp 
qtbytestream.cpp 
connection.cpp 
client.cpp 
mimeheader.cpp 
contactlistmanager.cpp 
contactinfo.cpp 
papillonglobal.cpp 
)

kde4_automoc( ${libpapillon_SRCS} )

kde4_add_library(papillon_kopete SHARED ${libpapillon_SRCS})

target_link_libraries( papillon_kopete ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QCA2_LIBRARIES} )

install_targets( ${LIB_INSTALL_DIR} papillon_kopete )

