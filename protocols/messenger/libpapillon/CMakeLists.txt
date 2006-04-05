kde4_header()

add_subdirectory( tests )
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${QCA2_INCLUDE_DIR}
)


########### next target ###############

set(libpapillon_SRCS 
bytestream.cpp 
connector.cpp 
coreprotocol.cpp 
safedelete.cpp 
stream.cpp 
task.cpp 
transfer.cpp 
papillonclientstream.cpp 
qtconnector.cpp 
qtbytestream.cpp 
connection.cpp 
logintask.cpp 
securestream.cpp 
tweenerhandler.cpp 
client.cpp 
notifymessagetask.cpp 
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
