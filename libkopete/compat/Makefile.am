METASOURCES = AUTO
AM_CPPFLAGS = $(all_includes)

SUBDIRS = . kcdialog

noinst_LTLIBRARIES  = libkopetecompat.la

libkopetecompat_la_SOURCES =
libkopetecompat_la_LDFLAGS = -no-undefined $(all_libraries)
libkopetecompat_la_LIBADD = kcdialog/libkopetecompat_kcdialog.la

