
if(NOT PAPILLON_INSIDE_KOPETE)
project(libpapillon)

# search packages used by KDE (TODO: Use Qt only)
find_package(KDE4 REQUIRED)
find_package(QCA2 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

link_directories (${KDE4_LIB_DIR})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

endif(NOT PAPILLON_INSIDE_KOPETE)

# Propagate includes to tests subdirs
set(PAPILLON_INCLUDES 
${CMAKE_CURRENT_SOURCE_DIR}/include/
${CMAKE_CURRENT_SOURCE_DIR}/contactlist/
${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Base/
${QCA2_INCLUDE_DIR}
)

add_subdirectory( tests )

add_definitions( ${QCA2_DEFINITIONS} )

include_directories(${PAPILLON_INCLUDES})

include(PapillonMoc.cmake)

########### next target ###############

set(papillon_base_SRCS
base/bytestream.cpp
base/connector.cpp
base/safedelete.cpp
base/stream.cpp
)

set(papillon_private_contactlist_SRCS
contactlist/fetchcontactlistjob.cpp
contactlist/sharingservicebinding.cpp
)

set(papillon_http_SRCS
http/securestream.cpp
http/tweenerhandler.cpp
http/httptransfer.cpp
http/httpcoreprotocol.cpp
http/httpconnection.cpp
)

set(papillon_tasks_SRCS
tasks/challengetask.cpp
tasks/logintask.cpp
tasks/notifymessagetask.cpp
tasks/notifypresencetask.cpp
tasks/notifystatusmessagetask.cpp
tasks/setpresencetask.cpp
tasks/setpersonalinformationtask.cpp
tasks/setstatusmessagetask.cpp
)

set(libpapillon_SRCS
${papillon_base_SRCS}
${papillon_private_contactlist_SRCS}
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
contactlist.cpp
contact.cpp
papillonglobal.cpp
papillonstatusmessage.cpp
usercontact.cpp
)

kde4_add_library(papillon_kopete SHARED ${libpapillon_SRCS})

target_link_libraries( papillon_kopete ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QCA2_LIBRARIES} )

set_target_properties( papillon_kopete PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install(TARGETS papillon_kopete  DESTINATION ${LIB_INSTALL_DIR})

