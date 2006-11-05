
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_contactnotes_PART_SRCS contactnotesplugin.cpp contactnotesedit.cpp )

kde4_automoc(${kopete_contactnotes_PART_SRCS})

kde4_add_plugin(kopete_contactnotes ${kopete_contactnotes_PART_SRCS})


target_link_libraries(kopete_contactnotes ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_contactnotes  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_contactnotes.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES contactnotesui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_contactnotes)




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_contactnotes.la
#
#kopete_contactnotes_la_SOURCES     = contactnotesplugin.cpp contactnotesedit.cpp
#kopete_contactnotes_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_contactnotes_la_LIBADD      = ../../libkopete/libkopete.la -lkio
#
#service_DATA = kopete_contactnotes.desktop
#servicedir = $(kde_servicesdir)
#
#mydatadir = $(kde_datadir)/kopete_contactnotes
#mydata_DATA = contactnotesui.rc
