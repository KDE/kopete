kde4_header()

add_subdirectory( icons ) 
add_subdirectory( libgadu )

include_directories( ${KOPETE_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/libgadu/ 
${CMAKE_CURRENT_BINARY_DIR}/libgadu/ 
)


########### next target ###############

set(kopete_gaduui_SRCS)

kde4_add_ui3_files(kopete_gaduui_SRCS 
   ui/gaduadd.ui 
   ui/gaduawayui.ui 
   ui/gadueditaccountui.ui 
   ui/gaduregisteraccountui.ui 
   ui/gadusearch.ui 
)

set(kopete_gadu_PART_SRCS 
   ${kopete_gaduui_SRCS} 
   gaduaway.cpp 
   gadueditcontact.cpp 
   gaducommands.cpp 
   gadueditaccount.cpp 
   gadusession.cpp 
   gaducontact.cpp 
   gaduaddcontactpage.cpp 
   gaduprotocol.cpp 
   gaduaccount.cpp 
   gadupubdir.cpp 
   gaduregisteraccount.cpp 
   gaducontactlist.cpp 
   gadurichtextformat.cpp 
   gadudccserver.cpp 
   gadudcctransaction.cpp 
   gadudcc.cpp )

kde4_automoc(${kopete_gadu_PART_SRCS})

kde4_add_plugin(kopete_gadu ${kopete_gadu_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_gadu )

target_link_libraries(kopete_gadu ${KDE4_KDEUI_LIBS} ${QT_QTNETWORK_LIBRARY} gadu_kopete kopete )

install_targets(${PLUGIN_INSTALL_DIR} kopete_gadu )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_gadu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#if include_libggcopy
#LIBGADU_COPY=libgadu
#GG_INCLUDES=-I$(srcdir)/libgadu $(SSL_INCLUDES)
#GG_LIBS=$(top_builddir)/kopete/protocols/gadu/libgadu/libgadu_copy.la \
#	   $(LIBPTHREAD)
#else
#LIBGADU_COPY=
#GG_INCLUDES=$(LIBGG_INCLUDES)
#GG_LIBS=$(LIBGG_LIBS)
#endif
#
#
#SUBDIRS = ui icons $(LIBGADU_COPY)
#AM_CPPFLAGS = -I$(srcdir)/ui \
#	-I./ui \
#	-I./lib \
#	-I$(srcdir)/lib \
#	$(KOPETE_INCLUDES) \
#	$(all_includes) \
#	$(GG_INCLUDES)
#
#kde_module_LTLIBRARIES = kopete_gadu.la
#
#kopete_gadu_la_SOURCES = gaduaway.cpp gadueditcontact.cpp gaducommands.cpp \
#			 gadueditaccount.cpp gadusession.cpp gaducontact.cpp \
#			 gaduaddcontactpage.cpp gaduprotocol.cpp gaduaccount.cpp \
#			 gadupubdir.cpp gaduregisteraccount.cpp \
#			 gaducontactlist.cpp gadurichtextformat.cpp \
#			 gadudccserver.cpp gadudcctransaction.cpp gadudcc.cpp
#
#kopete_gadu_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_gadu_la_LIBADD  = ./ui/libgaduui.la \
#		       $(top_builddir)/kopete/libkopete/libkopete.la \
#		       $(GG_LIBS)
#
#service_DATA = kopete_gadu.desktop
#servicedir = $(kde_servicesdir)
