
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


target_link_libraries(kopete_history  ${KDE4_KDECORE_LIBS} ${KDE4_KHTML_LIBS} kopete )

install(TARGETS kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_history_PART_SRCS historypreferences.cpp )

kde4_add_ui_files(kcm_kopete_history_PART_SRCS historyprefsui.ui )

kde4_add_kcfg_files(kcm_kopete_history_PART_SRCS historyconfig.kcfgc )

kde4_automoc(${kcm_kopete_history_PART_SRCS})

kde4_add_plugin(kcm_kopete_history ${kcm_kopete_history_PART_SRCS})


target_link_libraries(kcm_kopete_history  ${KDE4_KUTILS_LIBS} kopete )

install(TARGETS kcm_kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES historyconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_history.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES historyui.rc  historychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_history)
install( FILES kopete_history_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




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
