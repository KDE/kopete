
include_directories( ${KOPETE_INCLUDES} )


########### next target ###############

set(kcm_kopete_behaviorconfig_PART_SRCS
   behaviorconfig.cpp 
   behaviorconfig_chat.cpp 
   behaviorconfig_events.cpp 
   behaviorconfig_general.cpp 
   kopeteawayconfigbase.cpp 
   )

kde4_add_ui_files(kcm_kopete_behaviorconfig_PART_SRCS 
   behaviorconfig_chat.ui 
   behaviorconfig_events.ui 
   behaviorconfig_general.ui 
   kopeteawayconfigbase.ui )

kde4_automoc(${kcm_kopete_behaviorconfig_PART_SRCS})

kde4_add_plugin(kcm_kopete_behaviorconfig ${kcm_kopete_behaviorconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_behaviorconfig )

target_link_libraries(kcm_kopete_behaviorconfig  ${KDE4_KUTILS_LIBS} kopete)

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_behaviorconfig )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_behaviorconfig.desktop )




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
