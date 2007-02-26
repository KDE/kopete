AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_ASCII_CAST -DQT_NO_COMPAT \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/kopete/chatwindow -I$(top_srcdir)/kopete/libkopete $(all_includes) -DSRCDIR=\"$(top_srcdir)/kopete/kopete/chatwindow/tests\"
METASOURCES = AUTO
check_LTLIBRARIES = kunittest_chatwindowstyle_test.la kunittest_chatwindowstylerendering_test.la

kunittest_chatwindowstyle_test_la_SOURCES = chatwindowstyle_test.cpp
kunittest_chatwindowstyle_test_la_LIBADD = -lkunittest ../libkopetechatwindow.la
kunittest_chatwindowstyle_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kunittest_chatwindowstylerendering_test_la_SOURCES = chatwindowstylerendering_test.cpp
kunittest_chatwindowstylerendering_test_la_LIBADD = -lkunittest ../../../libkopete/libkopete.la ../libkopetechatwindow.la
kunittest_chatwindowstylerendering_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

check-local:
	kunittestmodrunner
guicheck:
	kunittestmod $(PWD) 
