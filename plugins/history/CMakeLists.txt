kde4_header()

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_history_PART_SRCS 
   historyplugin.cpp 
   historydialog.cpp 
   historylogger.cpp 
   converter.cpp 
   historyguiclient.cpp )

kde4_add_ui_files(kopete_history_PART_SRCS historyviewer.ui )

kde4_add_kcfg_files(kopete_history_PART_SRCS historyconfig.kcfgc )

kde4_automoc(${kopete_history_PART_SRCS})

kde4_add_plugin(kopete_history ${kopete_history_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_history )

target_link_libraries(kopete_history  ${KDE4_KDECORE_LIBS} ${KDE4_KHTML_LIBS} kopete )

install_targets(${PLUGIN_INSTALL_DIR} kopete_history )


########### next target ###############

set(kcm_kopete_history_PART_SRCS historypreferences.cpp )

kde4_add_ui_files(kcm_kopete_history_PART_SRCS historyprefsui.ui )

kde4_add_kcfg_files(kcm_kopete_history_PART_SRCS historyconfig.kcfgc )

kde4_automoc(${kcm_kopete_history_PART_SRCS})

kde4_add_plugin(kcm_kopete_history ${kcm_kopete_history_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_history )

target_link_libraries(kcm_kopete_history  ${KDE4_KUTILS_LIBS} kopete )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_history )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES historyconfig.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES kopete_history.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_history FILES historyui.rc  historychatui.rc )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_history_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_history.la kcm_kopete_history.la
#
#kopete_history_la_SOURCES = historyplugin.cpp historydialog.cpp historyviewer.ui\
#	historylogger.cpp converter.cpp historyguiclient.cpp historyconfig.kcfgc
#
#kopete_history_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_history_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_history_la_SOURCES = historyprefsui.ui historypreferences.cpp  historyconfig.kcfgc
#kcm_kopete_history_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_history_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_history.desktop
#servicedir = $(kde_servicesdir)
#
#mydatadir = $(kde_datadir)/kopete_history
#mydata_DATA = historyui.rc  historychatui.rc
#
#kcm_DATA = kopete_history_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#kde_kcfg_DATA = historyconfig.kcfg
