kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkopetemeanwhileui.la
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#		-I$(srcdir)/.. \
#		$(all_includes)
#
#libkopetemeanwhileui_la_SOURCES = empty.cpp meanwhileeditaccountbase.ui meanwhileaddcontactbase.ui
