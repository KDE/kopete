kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#    -I$(srcdir)/.. \
#    -I.. \
#    $(all_includes)
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#noinst_LTLIBRARIES = libkopetesmsservices.la
#
#
#libkopetesmsservices_la_SOURCES = smssend.cpp smssendprefs.ui smssendprovider.cpp \
#	smsclient.cpp smsclientprefs.ui gsmlib.cpp gsmlibprefs.ui kopete_unix_serial.cpp
#
#if include_smsgsm
#libkopetesmsservices_la_LIBADD = -lgsmme
#endif
#
