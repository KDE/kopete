kde4_header()

add_subdirectory( libkyahoo ) 
add_subdirectory( ui ) 
add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_yahoo_PART_SRCS 
   yahooprotocol.cpp 
   yahoocontact.cpp 
   yahooaddcontact.cpp 
   yahooaccount.cpp 
   yahooeditaccount.cpp 
   yahooconferencemessagemanager.cpp 
   yahoochatsession.cpp 
   yahooverifyaccount.cpp 
   yahoowebcam.cpp )

kde4_automoc(${kopete_yahoo_PART_SRCS})

kde4_add_plugin(kopete_yahoo ${kopete_yahoo_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_yahoo )

target_link_libraries(kopete_yahoo  ${KDE4_KDECORE_LIBS} kopete_videodevice kopete kopeteyahooui kyahoo )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_yahoo )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_yahoo.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_yahoo FILES yahooconferenceui.rc yahoochatui.rc )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = libkyahoo ui icons
#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#	-I$(srcdir)/ui \
#	-Iui \
#	-I$(srcdir)/libkyahoo \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kopete_yahoo.la
#
#kopete_yahoo_la_SOURCES = yahooprotocol.cpp yahoocontact.cpp \
#	yahooaddcontact.cpp yahooaccount.cpp yahooeditaccount.cpp yahooconferencemessagemanager.cpp \
#	yahoochatsession.cpp yahooverifyaccount.cpp yahoowebcam.cpp
#kopete_yahoo_la_LDFLAGS     = -module $(KDE_PLUGIN)
#kopete_yahoo_la_LIBADD = ../../libkopete/avdevice/libkopete_videodevice.la \
#	$(top_builddir)/kopete/libkopete/libkopete.la ui/libkopeteyahooui.la libkyahoo/libkyahoo.la
#
#service_DATA = kopete_yahoo.desktop
#servicedir = $(kde_servicesdir)
#
#
#mydatadir = $(kde_datadir)/kopete_yahoo
#mydata_DATA = yahooconferenceui.rc yahoochatui.rc
#
#
