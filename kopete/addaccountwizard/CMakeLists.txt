
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set( kopeteaddaccountwizard_SRCS 
addaccountwizard.cpp 
)

kde4_add_ui_files( kopeteaddaccountwizard_SRCS
    addaccountwizardpage1.ui 
    addaccountwizardpage2.ui 
)

kde4_automoc(${kopeteaddaccountwizard_SRCS})
kde4_add_library( kopeteaddaccountwizard SHARED ${kopeteaddaccountwizard_SRCS} )
target_link_libraries( kopeteaddaccountwizard kopete ${KDE4_KDEUI_LIBS} )
install(TARGETS kopeteaddaccountwizard  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############





