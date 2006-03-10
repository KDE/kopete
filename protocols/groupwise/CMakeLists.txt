kde4_header()

add_subdirectory( icons ) 
add_subdirectory( libgroupwise ) 
add_subdirectory( ui ) 

include_directories( ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/tasks ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/qca/src ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/ui ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_groupwise_PART_SRCS 
   gwprotocol.cpp 
   gwcontact.cpp 
   gwaccount.cpp 
   gwbytestream.cpp 
   gwconnector.cpp 
   gwmessagemanager.cpp 
   gwcontactlist.cpp )

kde4_automoc(${kopete_groupwise_PART_SRCS})

kde4_add_plugin(kopete_groupwise ${kopete_groupwise_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_groupwise )

target_link_libraries(kopete_groupwise  ${QT_AND_KDECORE_LIBS} kopetegroupwiseui groupwise kopete kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_groupwise )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_groupwise.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_groupwise FILES gwchatui.rc )

kde4_footer()



#original Makefile.am contents follow:

#
#
#SUBDIRS = icons libgroupwise ui
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = gwprotocol.h gwcontact.h gwaccount.h gwbytestream.h \
#		gwconnector.h gwmessagemanager.h gwcontactlist.h
#kde_module_LTLIBRARIES =  kopete_groupwise.la
#kopete_groupwise_la_SOURCES = gwprotocol.cpp gwcontact.cpp gwaccount.cpp \
#		gwbytestream.cpp gwconnector.cpp gwmessagemanager.cpp gwcontactlist.cpp
#kopete_groupwise_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
#	$(all_libraries)
#kopete_groupwise_la_LIBADD = ui/libkopetegroupwiseui.la \
#	libgroupwise/libgroupwise.la ../../libkopete/libkopete.la $(LIB_KIO)
#
#service_DATA = kopete_groupwise.desktop
#servicedir = $(kde_servicesdir)
#INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/tasks \
#	-I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise \
#	-I$(top_srcdir)/kopete/protocols/groupwise/ui -I$(top_builddir)/kopete/protocols/groupwise/ui
#
#mydatadir = $(kde_datadir)/kopete_groupwise
#mydata_DATA = gwchatui.rc
