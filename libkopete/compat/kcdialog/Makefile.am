INCLUDES = -I$(srcdir)/..  $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES   = libkopetecompat_kcdialog.la

# Needs KTrader for kregexpeditor
libkopetecompat_kcdialog_la_LIBADD  = $(LIB_KIO)
libkopetecompat_kcdialog_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
libkopetecompat_kcdialog_la_SOURCES = kcmodule.cpp kcmoduleinfo.cpp kcmoduleloader.cpp \
                        kcmultidialog.cpp  kselectentriesdialog.cpp \
                        kplugininfo.cpp kpluginselector.cpp ktabwidget.cpp ktabbar.cpp

# vim: et

