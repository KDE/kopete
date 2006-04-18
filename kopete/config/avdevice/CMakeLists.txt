kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kopete_avdeviceconfig_PART_SRCS avdeviceconfig.cpp )

kde4_automoc(${kcm_kopete_avdeviceconfig_PART_SRCS})

kde4_add_ui_files(kcm_kopete_avdeviceconfig_PART_SRCS avdeviceconfig_videoconfig.ui )

kde4_add_plugin(kcm_kopete_avdeviceconfig ${kcm_kopete_avdeviceconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_avdeviceconfig )

target_link_libraries(kcm_kopete_avdeviceconfig  ${KDE4_KDECORE_LIBS} kopete kopete_videodevice kutils )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_avdeviceconfig )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_avdeviceconfig.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(top_srcdir)/kopete/libkopete/avdevice \
#	-I$(top_srcdir)/kopete/libkopete/private $(KOPETE_COMPAT_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_kopete_avdeviceconfig.la
#
#kcm_kopete_avdeviceconfig_la_SOURCES = avdeviceconfig.cpp \
#	avdeviceconfig_videoconfig.ui
#
#kcm_kopete_avdeviceconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#
#kcm_kopete_avdeviceconfig_la_LIBADD  = ../../../libkopete/libkopete.la \
#	../../../libkopete/avdevice/libkopete_videodevice.la \
#	$(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#service_DATA = kopete_avdeviceconfig.desktop
#servicedir = $(kde_servicesdir)
#
#kopeteicondir = $(kde_datadir)/kopete/icons
#kopeteicon_ICON = AUTO
#
## vim: set noet:
#noinst_HEADERS = avdeviceconfig_videoconfig.h
