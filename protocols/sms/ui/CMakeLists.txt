



########### next target ###############
set( uiSrcs empty.cpp )
kde4_add_ui3_files ( uiSrcs
  smsactprefs.ui
  smsuserprefs.ui
  smsadd.ui )
kde4_add_library( kopetesmsui SHARED ${uiSrcs} )
target_link_libraries(kopetesmsui  ${KDE4_KIO_LIBS} )

########### install files ###############
install(TARGETS kopetesmsui  DESTINATION ${PLUGIN_INSTALL_DIR})





#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#    -I$(srcdir)/.. \
#    $(all_includes)
#
#noinst_LTLIBRARIES = libkopetesmsui.la
#
#libkopetesmsui_la_SOURCES = smsadd.ui smsactprefs.ui smsuserprefs.ui empty.cpp
