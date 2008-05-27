
include_directories ( ${KOPETE_INCLUDES} )

########### next target ###############

set(kopete_statistics_PART_SRCS 
   statisticsplugin.cpp 
   statisticsdb.cpp 
   statisticsdialog.cpp 
   statisticscontact.cpp )

kde4_add_ui_files(kopete_statistics_PART_SRCS statisticswidgetbase.ui )

qt4_add_dbus_adaptor(kopete_statistics_PART_SRCS org.kde.kopete.Statistics.xml
                     statisticsplugin.h StatisticsPlugin)

kde4_add_plugin(kopete_statistics ${kopete_statistics_PART_SRCS})


target_link_libraries(kopete_statistics  kopete ${QT_QTSQL_LIBRARY} ${KDE4_KHTML_LIBS} )

install(TARGETS kopete_statistics  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
install( FILES org.kde.kopete.Statistics.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
install( FILES kopete_statistics.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES statisticsui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_statistics)
