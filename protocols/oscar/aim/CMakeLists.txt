kde4_header()

#add_subdirectory( ui ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
${CMAKE_CURRENT_BINARY_DIR}/ui/ 
${CMAKE_CURRENT_SOURCE_DIR}/../ 
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/ 
)


########### next target ###############

set(kopete_aim_ui_SRCS ui/aimaddcontactpage.cpp ui/aimeditaccountwidget.cpp)

kde4_add_ui_files(kopete_aim_ui_SRCS 
ui/aimaddcontactui.ui 
ui/aiminfobase.ui 
ui/aimjoinchatbase.ui
ui/aimeditaccountui.ui 
)

set(kopete_aim_PART_SRCS 
   ${kopete_aim_ui_SRCS} 
   aimprotocol.cpp 
   aimaccount.cpp 
   aimcontact.cpp 
   aimuserinfo.cpp 
   aimjoinchat.cpp 
   aimchatsession.cpp )

kde4_automoc(${kopete_aim_PART_SRCS})

kde4_add_plugin(kopete_aim ${kopete_aim_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_aim )

target_link_libraries(kopete_aim  ${KDE4_KDECORE_LIBS} kopete_oscar kopete )

install_targets(${PLUGIN_INSTALL_DIR} kopete_aim )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_aim.desktop aim.protocol )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = ui
#METASOURCES = AUTO
#AM_CPPFLAGS = -I$(srcdir)/../ \
#	-I$(srcdir)/ui/ \
#	-I$(top_builddir)/kopete/protocols/oscar/aim/ui \
#	-I$(srcdir)/../liboscar \
#	$(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_aim.la
#
#kopete_aim_la_SOURCES = aimprotocol.cpp aimaccount.cpp aimcontact.cpp aimuserinfo.cpp aimjoinchat.cpp aimchatsession.cpp
#
#kopete_aim_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_aim_la_LIBADD  = ../libkopete_oscar.la ui/libkopeteaimui.la \
#	$(top_builddir)/kopete/libkopete/libkopete.la
#
#service_DATA = kopete_aim.desktop aim.protocol
#servicedir = $(kde_servicesdir)
