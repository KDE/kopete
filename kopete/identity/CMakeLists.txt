
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set( kopeteidentity_SRCS 
addidentitydialog.cpp 
identitydialog.cpp
)

kde4_add_ui_files( kopeteidentity_SRCS
    addidentitybase.ui 
    identitygeneral.ui
)

kde4_automoc(${kopeteidentity_SRCS})
kde4_add_library( kopeteidentity SHARED ${kopeteidentity_SRCS} )
target_link_libraries( kopeteidentity kopete ${KDE4_KDEUI_LIBS} )
set_target_properties( kopeteidentity PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopeteidentity DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############





