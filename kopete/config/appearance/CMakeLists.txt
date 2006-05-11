kde4_header()

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_SOURCE_DIR}/kopete/kopete/chatwindow 
)


########### next target ###############

set(kcm_kopete_appearanceconfig_PART_SRCS
   appearanceconfig.cpp 
   appearanceconfig_chatwindow.cpp 
   appearanceconfig_colors.cpp 
   appearanceconfig_contactlist.cpp 
   appearanceconfig_emoticons.cpp 
   tooltipeditdialog.cpp )

kde4_add_ui_files(kcm_kopete_appearanceconfig_PART_SRCS 
   appearanceconfig_chatwindow.ui 
   appearanceconfig_colors.ui
   appearanceconfig_contactlist.ui 
   appearanceconfig_emoticons.ui 
   tooltipeditwidget.ui )

kde4_automoc(${kcm_kopete_appearanceconfig_PART_SRCS})

kde4_add_plugin(kcm_kopete_appearanceconfig ${kcm_kopete_appearanceconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_appearanceconfig )

target_link_libraries(kcm_kopete_appearanceconfig ${KDE4_KUTILS_LIBS} kopete kopetechatwindow_shared ${KDE4_KNEWSTUFF_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_appearanceconfig )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_appearanceconfig.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
#	-I$(top_srcdir)/kopete/kopete/chatwindow $(KOPETE_COMPAT_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_kopete_appearanceconfig.la
#
#kcm_kopete_appearanceconfig_la_SOURCES = appearanceconfig_emoticons.ui \
#	appearanceconfig_colors.ui appearanceconfig_chatwindow.ui appearanceconfig_contactlist.ui \
#	appearanceconfig.cpp tooltipeditwidget.ui tooltipeditdialog.cpp
#
#kcm_kopete_appearanceconfig_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
#	$(all_libraries)
#kcm_kopete_appearanceconfig_la_LIBADD = ../../../libkopete/libkopete.la  \
#	../../../kopete/chatwindow/libkopetechatwindow.la \
#	-lktexteditor $(LIB_KOPETECOMPAT) $(LIB_KUTILS) $(LIB_KNEWSTUFF)
#
#service_DATA = kopete_appearanceconfig.desktop
#servicedir = $(kde_servicesdir)
#
## vim: set noet:
