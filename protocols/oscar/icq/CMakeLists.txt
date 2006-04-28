kde4_header()

#add_subdirectory( ui ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
${CMAKE_CURRENT_BINARY_DIR}/ui/ 
${CMAKE_CURRENT_SOURCE_DIR}/../ 
${CMAKE_CURRENT_BINARY_DIR}/../ 
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/ 
)


########### next target ###############

set(kopete_icq_ui_SRCS 
ui/icqeditaccountwidget.cpp 
ui/icquserinfowidget.cpp 
ui/icqauthreplydialog.cpp 
ui/icqaddcontactpage.cpp 
ui/icqsearchdialog.cpp
)

kde4_add_ui_files(kopete_icq_ui_SRCS 
ui/icqadd.ui 
ui/icqgeneralinfo.ui 
ui/icqotherinfowidget.ui 
ui/icqworkinfowidget.ui 
ui/icqinterestinfowidget.ui 
ui/icqauthreplyui.ui 
ui/icqsearchbase.ui 
ui/icqeditaccountui.ui 
)

set(kopete_icq_PART_SRCS 
   ${kopete_icq_ui_SRCS} 
   icqpresence.cpp 
   icqaccount.cpp 
   icqcontact.cpp 
   icqprotocol.cpp )

kde4_automoc(${kopete_icq_PART_SRCS})

kde4_add_plugin(kopete_icq ${kopete_icq_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_icq )

target_link_libraries(kopete_icq ${KDE4_KDECORE_LIBS} kopete_oscar kopete )

install_targets(${PLUGIN_INSTALL_DIR} kopete_icq )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_icq.desktop )
install_files( ${MIME_INSTALL_DIR}/application FILES x-icq.desktop )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = ui .
#METASOURCES = AUTO
#AM_CPPFLAGS = -I$(srcdir)/../ \
#	-I$(srcdir)/ui/ \
#	-I$(top_builddir)/kopete/protocols/oscar/icq/ui \
#	-I$(srcdir)/../liboscar \
#	$(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_icq.la
#
#kopete_icq_la_SOURCES = icqpresence.cpp icqaccount.cpp icqcontact.cpp icqprotocol.cpp
## icquserinfo.cpp icqreadaway.cpp icqsendsmsdialog.cpp
#
#kopete_icq_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_icq_la_LIBADD  = ../libkopete_oscar.la \
#	$(top_builddir)/kopete/libkopete/libkopete.la ui/libkopeteicqui.la
#
#service_DATA = kopete_icq.desktop
#servicedir = $(kde_servicesdir)
#
#mime_DATA = x-icq.desktop
#mimedir = $(kde_mimedir)/application
