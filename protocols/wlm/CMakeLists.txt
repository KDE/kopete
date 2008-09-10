add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

#ADD_SUBDIRECTORY(ui)
ADD_SUBDIRECTORY(icons)

include_directories(
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/ui
)

kde4_add_ui_files(kopete_wlm_ui_SRCS
    wlmaddui.ui
    wlmaccountpreferences.ui
)

set(kopete_wlm_PART_SRCS
   ${kopete_wlm_ui_SRCS}
   wlmprotocol.cpp
   wlmcontact.cpp
   wlmaccount.cpp
   wlmaddcontactpage.cpp
   wlmeditaccountwidget.cpp
   wlmlibmsn.cpp
   wlmsocket.cpp
   wlmserver.cpp
   wlmchatmanager.cpp
   wlmchatsession.cpp
   wlmtransfermanager.cpp
   ui/wlmwebcamdialog.cpp
)

kde4_add_plugin(kopete_wlm ${kopete_wlm_PART_SRCS})

target_link_libraries(kopete_wlm ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARAY} ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KIO_LIBRARY} kopete kopete_videodevice msn)

install(TARGETS kopete_wlm  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES  kopete_wlm.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  wlmchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_wlm)

#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = ui . icons
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(srcdir)/ui -Iui $(all_includes)
#
#noinst_HEADERS = wlmprotocol.h wlmcontact.h wlmaccount.h wlmaddcontactpage.h wlmeditaccountwidget.h wlmincomingmessage.h wlmlibmsn.h wlmchatmanager.h wlmchatsession.h wlmtransfermanager.h
#kde_module_LTLIBRARIES =  kopete_wlm.la
#kopete_wlm_la_SOURCES = wlmprotocol.cpp wlmcontact.cpp wlmaccount.cpp wlmaddcontactpage.cpp wlmaddui.ui wlmeditaccountwidget.cpp wlmaccountpreferences.ui wlmincomingmessage.cpp wlmlibmsn.cpp wlmsocket.cpp wlmserver.cpp wlmchatmanager.cpp wlmchatsession.cpp wlmtransfermanager.cpp
#kopete_wlm_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) /usr/lib/libmsn.so.0.0.0
#kopete_wlm_la_LIBADD = ../../libkopete/avdevice/libkopete_videodevice.la \
#	ui/libkopetewlmui.la ../../libkopete/libkopete.la $(LIB_KIO)
#
#service_DATA = kopete_wlm.desktop
#servicedir= $(kde_servicesdir)
#
#mydatadir = $(kde_datadir)/kopete_wlm
#mydata_DATA = wlmchatui.rc
#
