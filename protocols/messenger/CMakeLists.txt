kde4_header()

add_subdirectory( libpapillon ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_messenger_PART_SRCS messengerprotocol.cpp messengeraccount.cpp messengercontact.cpp )

kde4_automoc(${kopete_messenger_PART_SRCS})

kde4_add_plugin(kopete_messenger ${kopete_messenger_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_messenger )

target_link_libraries(kopete_messenger  ${KDE4_KIO_LIBS} kopete )

install_targets(${PLUGIN_INSTALL_DIR} kopete_messenger )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_messenger.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#SUBDIRS = libpapillon
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_messenger.la
#
#kopete_messenger_la_SOURCES = messengerprotocol.cpp messengeraccount.cpp messengercontact.cpp
#kopete_messenger_la_LIBADD = $(LIB_KIO) ../../libkopete/libkopete.la
#kopete_messenger_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#
#service_DATA = kopete_messenger.desktop
#servicedir = $(kde_servicesdir)
#
##mydatadir = $(kde_datadir)/kopete_messenger
##mydata_DATA = msnchatui.rc
#
