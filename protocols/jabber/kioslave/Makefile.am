METASOURCES = AUTO

INCLUDES = \
	-I$(srcdir)/.. \
	-I$(srcdir)/../libiris/iris/include \
	-I$(srcdir)/../libiris/iris/xmpp-im \
	-I$(srcdir)/../libiris/iris/jabber \
	-I$(srcdir)/../libiris/qca/src \
	-I$(srcdir)/../libiris/cutestuff/util \
	-I$(srcdir)/../libiris/cutestuff/network \
	$(all_includes)
                                                        
kde_module_LTLIBRARIES = kio_jabberdisco.la

kio_jabberdisco_la_SOURCES = jabberdisco.cpp
kio_jabberdisco_la_LIBADD = ../libjabberclient.la ../libiris/qca/src/libqca.la ../libiris/iris/include/libiris.la ../libiris/iris/xmpp-im/libiris_xmpp_im.la ../libiris/iris/xmpp-core/libiris_xmpp_core.la ../libiris/iris/jabber/libiris_jabber.la ../libiris/cutestuff/util/libcutestuff_util.la ../libiris/cutestuff/network/libcutestuff_network.la $(LIB_KIO)
kio_jabberdisco_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)

noinst_HEADERS = jabberdisco.h

protocol_DATA = jabberdisco.protocol
protocoldir = $(kde_servicesdir)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kio_jabberdisco.pot
