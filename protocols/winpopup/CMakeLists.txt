
#add_subdirectory( ui ) 
add_subdirectory( icons ) 
#add_subdirectory( libwinpopup ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/libwinpopup/ 
)


########### next target ###############

set(kopete_wp_ui_SRCS)

kde4_add_ui_files(kopete_wp_ui_SRCS 
ui/wpaddcontactbase.ui 
ui/wpuserinfowidget.ui 
ui/wpeditaccountbase.ui 
)

set(libwinpopup_SRCS 
libwinpopup/libwinpopup.cpp 
)

set(kopete_wp_PART_SRCS 
   ${kopete_wp_ui_SRCS} 
   ${libwinpopup_SRCS} 
   wpprotocol.cpp 
   wpcontact.cpp 
   wpaddcontact.cpp 
   wpeditaccount.cpp 
   wpaccount.cpp 
   wpuserinfo.cpp )

kde4_automoc(${kopete_wp_PART_SRCS})

kde4_add_plugin(kopete_wp ${kopete_wp_PART_SRCS})


target_link_libraries(kopete_wp  ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_wp  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_wp.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(PROGRAMS winpopup-send.sh winpopup-install.sh  DESTINATION ${BIN_INSTALL_DIR})



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
