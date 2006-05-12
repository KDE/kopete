
add_subdirectory( ui ) 
add_subdirectory( icons ) 




########### next target ###############

set(kopete_meanwhile_PART_SRCS 
   meanwhileprotocol.cpp 
   meanwhileaddcontactpage.cpp 
   meanwhileeditaccountwidget.cpp 
   meanwhileaccount.cpp 
   meanwhilecontact.cpp 
   meanwhilesession.cpp 
   meanwhileplugin.cpp )

kde4_automoc(${kopete_meanwhile_PART_SRCS})

kde4_add_plugin(kopete_meanwhile ${kopete_meanwhile_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_meanwhile )

target_link_libraries(kopete_meanwhile  ${KDE4_KDECORE_LIBS} kopete kopetemeanwhileui )

install_targets(${PLUGIN_INSTALL_DIR} kopete_meanwhile )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_meanwhile.desktop )




#original Makefile.am contents follow:

#METASOURCES = AUTO
#SUBDIRS = ui icons
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(srcdir)/ui \
#	-I./ui \
#	$(all_includes) \
#	$(MEANWHILE_CFLAGS)
#
#noinst_HEADERS = \
#	meanwhileprotocol.h \
#	meanwhileaddcontactpage.h \
#	meanwhileeditaccountwidget.h \
#	meanwhileaccount.h \
#	meanwhilecontact.h \
#	meanwhilesession.h \
#	meanwhileplugin.h
#
#kde_module_LTLIBRARIES =  kopete_meanwhile.la
#
#kopete_meanwhile_la_SOURCES = \
#	meanwhileprotocol.cpp \
#	meanwhileaddcontactpage.cpp \
#	meanwhileeditaccountwidget.cpp \
#	meanwhileaccount.cpp \
#	meanwhilecontact.cpp \
#	meanwhilesession.cpp \
#	meanwhileplugin.cpp
#
#kopete_meanwhile_la_LDFLAGS = -no-undefined -module \
#	$(KDE_PLUGIN) $(all_libraries) 
#
#kopete_meanwhile_la_LIBADD = $(top_builddir)/kopete/libkopete/libkopete.la \
#	ui/libkopetemeanwhileui.la $(MEANWHILE_LIBS)
#
#service_DATA = kopete_meanwhile.desktop
#servicedir= $(kde_servicesdir)
