kde4_header()




########### next target ###############

set(kopetetabwidgettest_SRCS kopetetabwidgettest.cpp )

kde4_automoc(${kopetetabwidgettest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kopetetabwidgettest ${kopetetabwidgettest_SRCS})

target_link_libraries(kopetetabwidgettest  ${KDE4_KDECORE_LIBS} kopetechatwindow )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_ASCII_CAST -DQT_NO_COMPAT \
#	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/kopete/chatwindow -I$(top_srcdir)/kopete/libkopete $(all_includes) -DSRCDIR=\"$(top_srcdir)/kopete/kopete/chatwindow/tests\"
#METASOURCES = AUTO
#check_PROGRAMS = kopetetabwidgettest
#check_LTLIBRARIES = kunittest_chatwindowstyle_test.la kunittest_chatwindowstylerendering_test.la
#
#kunittest_chatwindowstyle_test_la_SOURCES = chatwindowstyle_test.cpp
#kunittest_chatwindowstyle_test_la_LIBADD = -lkunittest ../libkopetechatwindow.la
#kunittest_chatwindowstyle_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#kunittest_chatwindowstylerendering_test_la_SOURCES = chatwindowstylerendering_test.cpp
#kunittest_chatwindowstylerendering_test_la_LIBADD = -lkunittest $(top_srcdir)/kopete/libkopete/libkopete.la ../libkopetechatwindow.la
#kunittest_chatwindowstylerendering_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#kopetetabwidgettest_SOURCES = kopetetabwidgettest.cpp
#kopetetabwidgettest_LDADD = ../libkopetechatwindow.la
#kopetetabwidgettest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#
#check-local: 
#	kunittestmodrunner
#guicheck:
#	kunittestmod $(PWD) 
