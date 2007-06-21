
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set( kopeteaddidentitywizard_SRCS 
addidentitywizard.cpp 
)

kde4_add_ui_files( kopeteaddidentitywizard_SRCS
    addidentitywizardpage1.ui 
)

kde4_automoc(${kopeteaddidentitywizard_SRCS})
kde4_add_library( kopeteaddidentitywizard SHARED ${kopeteaddidentitywizard_SRCS} )
target_link_libraries( kopeteaddidentitywizard kopete ${KDE4_KDEUI_LIBS} )
set_target_properties( kopeteaddidentitywizard PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopeteaddidentitywizard  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############





