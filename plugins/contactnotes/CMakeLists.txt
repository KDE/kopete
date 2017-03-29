
include_directories( 
    ${KOPETE_INCLUDES}
    )


########### next target ###############

set(kopete_contactnotes_PART_SRCS contactnotesplugin.cpp contactnotesedit.cpp )

ecm_qt_declare_logging_category(kopete_contactnotes_PART_SRCS HEADER plugin_contactnotes_debug.h IDENTIFIER KOPETE_PLUGIN_CONTACTNOTES_LOG CATEGORY_NAME org.kde.plugin.contactnotes)

qt5_add_resources(kopete_contactnotes_PART_SRCS contactnotes.qrc)

add_library(kopete_contactnotes MODULE ${kopete_contactnotes_PART_SRCS})


target_link_libraries(kopete_contactnotes KF5::KIOCore kopete )

install(TARGETS kopete_contactnotes  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_contactnotes.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})



