kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_netmeeting_PART_SRCS netmeetingplugin.cpp netmeetinginvitation.cpp netmeetingguiclient.cpp )

kde4_automoc(${kopete_netmeeting_PART_SRCS})

kde4_add_plugin(kopete_netmeeting ${kopete_netmeeting_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_netmeeting )

target_link_libraries(kopete_netmeeting  ${KDE4_KDECORE_LIBS} kopete kopete_msn_shared )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_netmeeting )


########### next target ###############

set(kcm_kopete_netmeeting_PART_SRCS netmeetingpreferences.cpp )

kde4_automoc(${kcm_kopete_netmeeting_PART_SRCS})

kde4_add_ui_files(kcm_kopete_netmeeting_PART_SRCS netmeetingprefs_ui.ui )

kde4_add_plugin(kcm_kopete_netmeeting ${kcm_kopete_netmeeting_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_netmeeting )

target_link_libraries(kcm_kopete_netmeeting  ${KDE4_KDECORE_LIBS} kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_netmeeting )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_netmeeting.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_netmeeting FILES netmeetingchatui.rc )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_netmeeting_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/msn $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_netmeeting.la kcm_kopete_netmeeting.la
#
#kopete_netmeeting_la_SOURCES     = netmeetingplugin.cpp netmeetinginvitation.cpp netmeetingguiclient.cpp
#kopete_netmeeting_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_netmeeting_la_LIBADD      = $(top_builddir)/kopete/libkopete/libkopete.la $(top_builddir)/kopete/protocols/msn/libkopete_msn_shared.la
#
#service_DATA = kopete_netmeeting.desktop
#servicedir = $(kde_servicesdir)
#
#mydatadir = $(kde_datadir)/kopete_netmeeting
#mydata_DATA = netmeetingchatui.rc
#
#kcm_kopete_netmeeting_la_SOURCES = netmeetingprefs_ui.ui netmeetingpreferences.cpp
#kcm_kopete_netmeeting_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_netmeeting_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#
#kcm_DATA = kopete_netmeeting_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
