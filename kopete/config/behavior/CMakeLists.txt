kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kopete_behaviorconfig_PART_SRCS behaviorconfig.cpp )

kde4_automoc(${kcm_kopete_behaviorconfig_PART_SRCS})

kde4_add_ui_files(kcm_kopete_behaviorconfig_PART_SRCS 
   kopeteawayconfigbase.ui 
   behaviorconfig_chat.ui 
   behaviorconfig_general.ui 
   behaviorconfig_events.ui )

kde4_add_plugin(kcm_kopete_behaviorconfig ${kcm_kopete_behaviorconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_behaviorconfig )

target_link_libraries(kcm_kopete_behaviorconfig  ${KDE4_KDECORE_LIBS} kopete kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_behaviorconfig )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_behaviorconfig.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(KOPETE_COMPAT_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_kopete_behaviorconfig.la
#
#kcm_kopete_behaviorconfig_la_SOURCES = \
#	kopeteawayconfigbase.ui  \
#	behaviorconfig_chat.ui behaviorconfig_general.ui behaviorconfig_events.ui behaviorconfig.cpp
#
#kcm_kopete_behaviorconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#
#kcm_kopete_behaviorconfig_la_LIBADD  = ../../../libkopete/libkopete.la \
#	$(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#service_DATA = kopete_behaviorconfig.desktop
#servicedir = $(kde_servicesdir)
#
## vim: set noet:
