AM_CPPFLAGS = -I$(top_builddir)/lib $(all_includes) 

METASOURCES = AUTO

kde_module_LTLIBRARIES = kio_kopete.la

kio_kopete_la_SOURCES = kio_kopete.cpp

kio_kopete_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)

kio_kopete_la_LIBADD = $(LIB_KIO)

datadir = $(kde_servicesdir)
data_DATA = kopete.protocol

#check_PROGRAMS = linktest
#linktest_SOURCES = $(kio_kopete_la_SOURCES) linktest.cpp
#
#linktest.cpp:
#	echo "int main() {};" > linktest.cpp
#
#linktest_LDFLAGS  = $(all_libraries)
#linktest_LDADD = $(kio_kopete_la_LIBADD)
