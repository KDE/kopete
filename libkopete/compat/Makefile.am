METASOURCES = AUTO
AM_CPPFLAGS = $(all_includes)

SUBDIRS = kcdialog ksettings .

noinst_LTLIBRARIES  = libkopetecompat.la

libkopetecompat_la_SOURCES = dummy.cpp kautoconfig.cpp
libkopetecompat_la_LDFLAGS = -no-undefined $(LIB_KDEUI) $(LIB_KDECORE) $(all_libraries)
libkopetecompat_la_LIBADD = kcdialog/libkopetecompat_kcdialog.la ksettings/libksettings.la

