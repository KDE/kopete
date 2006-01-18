INCLUDES = $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libdiscovery_kopete.la
libdiscovery_kopete_la_LDFLAGS = -avoid-version -module $(all_libraries)
libdiscovery_kopete_la_LIBADD = -lksrvdisco
noinst_HEADERS = kopeteengine.h kopetepublisher.h kopetelister.h 
libdiscovery_kopete_la_SOURCES = kopeteengine.cpp kopetepublisher.cpp kopeteservicelister.cpp kopeteservicelister.skel kopetelister.cpp \
	datatypes.cpp srvdiscoiface.stub kopetelister.skel kopetepublisher.skel
kde_services_DATA = srvdisco_engine_kopete.desktop