kde4_header()

add_subdirectory( ui ) 
add_subdirectory( icons ) 
add_subdirectory( libwinpopup ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_wp_PART_SRCS 
   wpprotocol.cpp 
   wpcontact.cpp 
   wpaddcontact.cpp 
   wpeditaccount.cpp 
   wpaccount.cpp 
   wpuserinfo.cpp )

kde4_automoc(${kopete_wp_PART_SRCS})

kde4_add_plugin(kopete_wp ${kopete_wp_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_wp )

target_link_libraries(kopete_wp  ${KDE4_KDECORE_LIBS} kopete kopetewpui winpopup kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_wp )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_wp.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#SUBDIRS = ui icons libwinpopup
#AM_CPPFLAGS = -I$(srcdir)/ui \
#	-I./ui \
#	-I$(srcdir)/libwinpopup \
#	$(KOPETE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kopete_wp.la
#
#noinst_HEADERS = wpprotocol.h wpeditaccount.h wpaccount.h wpuserinfo.h wpcontact.h wpaddcontact.h
#
#kopete_wp_la_SOURCES = wpprotocol.cpp wpcontact.cpp  wpaddcontact.cpp wpeditaccount.cpp  wpaccount.cpp wpuserinfo.cpp
#kopete_wp_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_wp_la_LIBADD = ../../libkopete/libkopete.la ./ui/libkopetewpui.la \
#	./libwinpopup/libwinpopup.la $(LIB_KIO)
#
#service_DATA = kopete_wp.desktop
#servicedir = $(kde_servicesdir)
#
#bin_SCRIPTS = winpopup-send.sh winpopup-install.sh
#
