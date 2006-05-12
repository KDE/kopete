
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_testbed_PART_SRCS 
   testbedprotocol.cpp 
   testbedcontact.cpp 
   testbedaccount.cpp 
   testbedaddcontactpage.cpp 
   testbededitaccountwidget.cpp 
   testbedfakeserver.cpp 
   testbedincomingmessage.cpp )

kde4_automoc(${kopete_testbed_PART_SRCS})

kde4_add_ui_files(kopete_testbed_PART_SRCS
testbedaddui.ui
)

kde4_add_ui_files(kopete_testbed_PART_SRCS
testbedaccountpreferences.ui
)

kde4_add_plugin(kopete_testbed ${kopete_testbed_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_testbed )

target_link_libraries(kopete_testbed  ${KDE4_KDECORE_LIBS} kopete kio )

install_targets(${PLUGIN_INSTALL_DIR} kopete_testbed )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_testbed.desktop )




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = . icons
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#noinst_HEADERS = testbedprotocol.h testbedcontact.h testbedaccount.h testbedaddcontactpage.h testbededitaccountwidget.h testbedfakeserver.h testbedincomingmessage.h
#kde_module_LTLIBRARIES =  kopete_testbed.la
#kopete_testbed_la_SOURCES = testbedprotocol.cpp testbedcontact.cpp testbedaccount.cpp testbedaddcontactpage.cpp testbedaddui.ui testbededitaccountwidget.cpp testbedaccountpreferences.ui testbedfakeserver.cpp testbedincomingmessage.cpp
#kopete_testbed_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_testbed_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KIO)
#
#service_DATA = kopete_testbed.desktop
#servicedir= $(kde_servicesdir)
