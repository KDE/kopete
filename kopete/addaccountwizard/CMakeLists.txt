
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set( kopeteaddaccountwizard_SRCS 
addaccountwizard.cpp 
)

ki18n_wrap_ui( kopeteaddaccountwizard_SRCS
    addaccountwizardpage1.ui 
    addaccountwizardpage2.ui 
)

add_library( kopeteaddaccountwizard SHARED ${kopeteaddaccountwizard_SRCS} )
target_link_libraries( kopeteaddaccountwizard kopete )
set_target_properties( kopeteaddaccountwizard PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopeteaddaccountwizard ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





