
#add_subdirectory( wrappers ) 

include_directories(
    ${KOPETE_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
)


########### next target ###############

set( kopete_javascript_PART_SRCS
    javascriptconfig.cpp
    javascriptfile.cpp
    javascriptplugin.cpp
)

kde4_automoc(${kopete_javascript_PART_SRCS})

kde4_add_plugin(kopete_javascript ${kopete_javascript_PART_SRCS})


target_link_libraries(kopete_javascript  ${KDE4_KDECORE_LIBS} kopete kjsembed )
#wrappers

install( TARGETS kopete_javascript DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set( kcm_kopete_javascript_PART_SRCS
    javascriptconfig.cpp
    javascriptfile.cpp
    javascriptpreferences.cpp
)

set( kcm_kopete_javascript_UI
    javascriptprefsbase.ui
)

kde4_add_ui_files(kcm_kopete_javascript_PART_SRCS ${kcm_kopete_javascript_UI} )

kde4_automoc(${kcm_kopete_javascript_PART_SRCS})

kde4_add_plugin(kcm_kopete_javascript ${kcm_kopete_javascript_PART_SRCS})


target_link_libraries(kcm_kopete_javascript ${QT_AND_KDECORE_LIBS} ${KDE4_KNEWSTUFF_LIBS} kopete )

install( TARGETS kcm_kopete_javascript DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_javascript.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kopete_javascript_config.desktop DESTINATION  ${SERVICES_INSTALL_DIR}/kconfiguredialog )



