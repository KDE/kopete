kde4_header()

add_subdirectory( icons ) 
add_subdirectory( libkirc ) 
add_subdirectory( ui ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_irc_PART_SRCS 
   ircaccount.cpp 
   ircaddcontactpage.cpp 
   ircchannelcontact.cpp 
   irccontact.cpp 
   ircguiclient.cpp 
   ircprotocol.cpp 
   ircservercontact.cpp 
   ircsignalhandler.cpp 
   irctransferhandler.cpp 
   ircusercontact.cpp 
   irccontactmanager.cpp 
   kcodecaction.cpp 
   ksparser.cpp )

kde4_automoc(${kopete_irc_PART_SRCS})

kde4_add_plugin(kopete_irc ${kopete_irc_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_irc )

target_link_libraries(kopete_irc  ${KDE4_KDECORE_LIBS} kopete kopeteircui kirc kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_irc )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_irc.desktop irc.protocol )
install_files( ${DATA_INSTALL_DIR}/kopete FILES ircchatui.rc )
install_files( ${DATA_INSTALL_DIR}/kopete FILES ircnetworks.xml )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = icons libkirc ui
#AM_CPPFLAGS = -I$(srcdir)/ui $(KOPETE_INCLUDES) \
#		-I./ui \
#		-I$(srcdir)/libkirc \
#		$(all_includes)
#kde_module_LTLIBRARIES = kopete_irc.la
#
#kopete_irc_la_SOURCES = \
#	ircaccount.cpp  \
#	ircaddcontactpage.cpp \
#	ircchannelcontact.cpp \
#	irccontact.cpp \
#	ircguiclient.cpp \
#	ircprotocol.cpp \
#	ircservercontact.cpp \
#	ircsignalhandler.cpp \
#	irctransferhandler.cpp \
#	ircusercontact.cpp \
#	irccontactmanager.cpp \
#	kcodecaction.cpp \
#	ksparser.cpp
#
#kopete_irc_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries)
#kopete_irc_la_LIBADD      = ../../libkopete/libkopete.la \
#		./ui/libkopeteircui.la \
#		./libkirc/libkirc.la \
#		$(LIB_KIO)
#
#service_DATA = kopete_irc.desktop irc.protocol
#servicedir = $(kde_servicesdir)
#
#xmldata_DATA = ircnetworks.xml
#xmldatadir = $(kde_datadir)/kopete
#
#EXTRA_DIST = $(xmldata_DATA)
#
#mydatadir = $(kde_datadir)/kopete
#mydata_DATA = ircchatui.rc
