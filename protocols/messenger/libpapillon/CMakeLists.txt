kde4_header()

# Propagate includes to tests subdirs
set(PAPILLON_INCLUDES 
${QT_INCLUDES} 
${CMAKE_SOURCE_DIR}/kopete/protocols/messenger/libpapillon/ 
${CMAKE_SOURCE_DIR}/kopete/protocols/messenger/libpapillon/base/ 
${CMAKE_SOURCE_DIR}/kopete/protocols/messenger/libpapillon/http/ 
${CMAKE_SOURCE_DIR}/kopete/protocols/messenger/libpapillon/tasks/ 
${QCA2_INCLUDE_DIR} 
)

add_subdirectory( tests )


include_directories(${CMAKE_CURRENT_BINARY_DIR} ${PAPILLON_INCLUDES})


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
)

set(papillon_tasks_SRCS 
tasks/logintask.cpp 
tasks/notifymessagetask.cpp 
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
)

kde4_automoc( ${libpapillon_SRCS} )

kde4_add_library(papillon_kopete SHARED ${libpapillon_SRCS})

target_link_libraries( papillon_kopete ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QCA2_LIBRARIES} )

install_targets( ${LIB_INSTALL_DIR} papillon_kopete )

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libpapillon.la
#
#AM_CPPFLAGS = $(all_includes)
#
#METASOURCES = AUTO
#
#libpapillon_la_SOURCES = bytestream.cpp connector.cpp coreprotocol.cpp \
#		safedelete.cpp stream.cpp task.cpp transfer.cpp notificationstream.cpp qtconnector.cpp \
#	qtbytestream.cpp
#libpapillon_la_LIBADD = $(LIB_QTCORE) $(LIB_QTNETWORK)
#libpapillon_la_LDFLAGS = -no-undefined $(all_libraries)
#
#noinst_HEADERS = papillon_macros.h transfer.h notificationstream.h \
#	qtconnector.h qtbytestream.h
#
