
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

kde4_add_library( kopeteaddaccountwizard SHARED ${kopeteaddaccountwizard_SRCS} )
target_link_libraries( kopeteaddaccountwizard kopete ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} )
set_target_properties( kopeteaddaccountwizard PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopeteaddaccountwizard ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





