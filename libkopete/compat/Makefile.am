METASOURCES = AUTO
SUBDIRS = . kcdialog

INCLUDES = $(all_includes)
noinst_HEADERS = knotifydialog.h knotifywidgetbase.h knotifyclient.h
noinst_LTLIBRARIES  = libkopetecompat.la

libkopetecompat_la_SOURCES = knotifywidgetbase.ui knotifydialog.cpp knotifyclient.cpp
libkopetecompat_la_LDFLAGS = -no-undefined $(all_libraries)
libkopetecompat_la_LIBADD = kcdialog/libkopetecompat_kcdialog.la

