
include_directories ( ${KOPETE_INCLUDES} )

########### next target ###############

set(kopete_statistics_PART_SRCS 
   statisticsplugin.cpp 
   statisticsdb.cpp 
   statisticsdialog.cpp 
   statisticscontact.cpp )

ki18n_wrap_ui(kopete_statistics_PART_SRCS statisticswidgetbase.ui )

qt5_add_resources(kopete_statistics_PART_SRCS statistics.qrc)

qt5_add_dbus_adaptor(kopete_statistics_PART_SRCS org.kde.kopete.Statistics.xml
                     statisticsplugin.h StatisticsPlugin)

add_library(kopete_statistics MODULE ${kopete_statistics_PART_SRCS})


target_link_libraries(kopete_statistics  kopete Qt5::Sql KF5::KHtml )

install(TARGETS kopete_statistics  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############
install( FILES org.kde.kopete.Statistics.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )
install( FILES kopete_statistics.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
