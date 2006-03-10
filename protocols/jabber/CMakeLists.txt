kde4_header()

add_subdirectory( ui ) 
add_subdirectory( icons ) 
add_subdirectory( libiris ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(JINGLE)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(kopete_jabber_PART_SRCS 
   jabberprotocol.cpp 
   jabberaccount.cpp 
   jabberresource.cpp 
   jabberresourcepool.cpp 
   jabberbasecontact.cpp 
   jabbercontact.cpp 
   jabbergroupcontact.cpp 
   jabbergroupmembercontact.cpp 
   jabbercontactpool.cpp 
   jabberformtranslator.cpp 
   jabberformlineedit.cpp 
   jabberchatsession.cpp 
   jabbergroupchatmanager.cpp 
   jabberfiletransfer.cpp 
   jabbercapabilitiesmanager.cpp 
   jabbertransport.cpp 
   jabberbookmarks.cpp )

kde4_automoc(${kopete_jabber_PART_SRCS})

kde4_add_plugin(kopete_jabber ${kopete_jabber_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_jabber )

target_link_libraries(kopete_jabber  ${QT_AND_KDECORE_LIBS} kopete kopetejabberui cutestuff_util cutestuff_network iris iris_jabber iris_xmpp_core iris_xmpp_im jabberclient )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_jabber )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_jabber.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_jabber FILES jabberchatui.rc )

kde4_footer()



#original Makefile.am contents follow:

#if include_jingle
#JINGLE=jingle
#JINGLE_LIBS=jingle/libkopetejabberjingle.la
#JINGLE_INCLUDES=-I$(srcdir)/jingle -I$(top_builddir)/kopete/protocols/jabber/jingle
#endif
#
#METASOURCES = AUTO
## Disabled kioslave for now, anyway this is not used (-DarkShock)
#SUBDIRS = ui icons libiris $(JINGLE) . 
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(srcdir)/libiris/iris/include \
#	-I$(srcdir)/libiris/iris/xmpp-im \
#	-I$(srcdir)/libiris/iris/jabber \
#	-I$(srcdir)/libiris/cutestuff/util \
#	-I$(srcdir)/libiris/cutestuff/network \
#	-I$(srcdir)/ui \
#	-I./ui \
#	$(all_includes) $(JINGLE_INCLUDES) $(QCA_CFLAGS)
#
#noinst_LTLIBRARIES = libjabberclient.la
#libjabberclient_la_SOURCES = \
#	jabberclient.cpp \
#	jabberconnector.cpp \
#	jabberbytestream.cpp
#
#kde_module_LTLIBRARIES = kopete_jabber.la
#
#kopete_jabber_la_SOURCES = \
#	jabberprotocol.cpp \
#	jabberaccount.cpp \
#	jabberresource.cpp \
#	jabberresourcepool.cpp \
#	jabberbasecontact.cpp \
#	jabbercontact.cpp \
#	jabbergroupcontact.cpp \
#	jabbergroupmembercontact.cpp \
#	jabbercontactpool.cpp \
#	jabberformtranslator.cpp \
#	jabberformlineedit.cpp \
#	jabberchatsession.cpp \
#	jabbergroupchatmanager.cpp \
#	jabberfiletransfer.cpp \
#	jabbercapabilitiesmanager.cpp\
#	jabbertransport.cpp\
#	jabberbookmarks.cpp
#
#kopete_jabber_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_jabber_la_LIBADD  = $(top_builddir)/kopete/libkopete/libkopete.la \
#	ui/libkopetejabberui.la \
#	libiris/cutestuff/util/libcutestuff_util.la \
#	libiris/cutestuff/network/libcutestuff_network.la \
#	libiris/iris/include/libiris.la \
#	libiris/iris/jabber/libiris_jabber.la \
#	libiris/iris/xmpp-core/libiris_xmpp_core.la \
#    libiris/iris/xmpp-im/libiris_xmpp_im.la \
#	$(QCA_LIBS) \
#	libjabberclient.la \
#	$(JINGLE_LIBS)
#
#service_DATA = kopete_jabber.desktop
#servicedir = $(kde_servicesdir)
#
#mydatadir = $(kde_datadir)/kopete_jabber
#mydata_DATA = jabberchatui.rc
#
#noinst_HEADERS = jabberresourcepool.h jabbercontact.h jabbergroupcontact.h \
#	jabberclient.h
