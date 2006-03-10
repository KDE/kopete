kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#noinst_LTLIBRARIES = libkopeteaddaccountwizard.la
#noinst_HEADERS = addaccountwizard.h
#
#libkopeteaddaccountwizard_la_SOURCES = addaccountwizardpage1.ui addaccountwizardpage2.ui addaccountwizardpage3.ui addaccountwizard.cpp
#libkopeteaddaccountwizard_la_LDFLAGS = $(all_libraries) -no-undefined
#libkopeteaddaccountwizard_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KDEUI) 
#
## vim: set noet:
#
