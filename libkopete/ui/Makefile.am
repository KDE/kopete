#SUBDIRS =
noinst_LIBRARIES = libui.a
CXXFLAGS = -g -O2 -Wall
METASOURCES = AUTO
libui_a_METASOURCES = AUTO
# set the include path for X, qt and KDE
INCLUDES = $(all_includes)
DEFS = -I. -I$(srcdir) -I$(top_srcdir)
libui_a_SOURCES = \
		DlgPrefs.ui \
		preferencesdialog.cpp
EXTRA_DIST = \
		preferencesdialog.cpp \
		preferencesdialog.h
