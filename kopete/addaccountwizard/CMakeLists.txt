kde4_header()

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set( kopeteaddaccountwizard_SRCS 
addaccountwizard.cpp 
)

#addaccountwizardpage3.ui not needed?
kde4_add_ui_files( kopeteaddaccountwizard_SRCS
    addaccountwizardpage1.ui 
    addaccountwizardpage2.ui 
)

kde4_automoc( ${kopeteaddaccountwizard_SRCS} )
kde4_add_library( kopeteaddaccountwizard SHARED ${kopeteaddaccountwizard_SRCS} )
target_link_libraries( kopeteaddaccountwizard kopete ${KDE4_KDEUI_LIBS} )
install_targets( ${LIB_INSTALL_DIR} kopeteaddaccountwizard )

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#noinst_LTLIBRARIES = libkopeteaddaccountwizard.la
#noinst_HEADERS = addaccountwizard.h
#
#libkopeteaddaccountwizard_la_SOURCES = addaccountwizardpage1.ui addaccountwizardpage2.ui addaccountwizardpage3.ui addaccountwizard.cpp
#libkopeteaddaccountwizard_la_LDFLAGS = $(all_libraries) -no-undefined
#libkopeteaddaccountwizard_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KDEUI) 
#
## vim: set noet:
#
