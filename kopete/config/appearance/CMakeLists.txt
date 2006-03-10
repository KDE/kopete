kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kopete_appearanceconfig_PART_SRCS appearanceconfig.cpp tooltipeditdialog.cpp )

kde4_automoc(${kcm_kopete_appearanceconfig_PART_SRCS})

kde4_add_ui_files(kcm_kopete_appearanceconfig_PART_SRCS 
   appearanceconfig_emoticons.ui 
   appearanceconfig_colors.ui 
   appearanceconfig_chatwindow.ui 
   appearanceconfig_contactlist.ui 
   tooltipeditwidget.ui )

kde4_add_plugin(kcm_kopete_appearanceconfig ${kcm_kopete_appearanceconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_appearanceconfig )

target_link_libraries(kcm_kopete_appearanceconfig  ${QT_AND_KDECORE_LIBS} kopete kopetechatwindow ktexteditor kutils knewstuff )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_appearanceconfig )


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
