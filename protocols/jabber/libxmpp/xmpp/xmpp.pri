xmpp {
	INCLUDEPATH += $$XMPP_BASE/src

	HEADERS += \
		$$XMPP_BASE/src/xmpp_bytestream.h \
		$$XMPP_BASE/src/xmpp_xmlfilter.h \
		$$XMPP_BASE/src/xmpp_stream.h \
		$$XMPP_BASE/src/xmpp_jid.h \
		$$XMPP_BASE/src/xmpp_types.h \
		$$XMPP_BASE/src/xmpp_xmlcommon.h \
		$$XMPP_BASE/src/xmpp_vcard.h \
		$$XMPP_BASE/src/xmpp_message.h \
		$$XMPP_BASE/src/xmpp_tasks.h \
		$$XMPP_BASE/src/xmpp_jidlink.h \
		$$XMPP_BASE/src/xmpp_dtcp.h \
		$$XMPP_BASE/src/xmpp_ibb.h \
		$$XMPP_BASE/src/xmpp_client.h

	SOURCES += \
		$$XMPP_BASE/src/xmpp_bytestream.cpp \
		$$XMPP_BASE/src/xmpp_xmlfilter.cpp \
		$$XMPP_BASE/src/xmpp_stream.cpp \
		$$XMPP_BASE/src/xmpp_jid.cpp \
		$$XMPP_BASE/src/xmpp_types.cpp \
		$$XMPP_BASE/src/xmpp_xmlcommon.cpp \
		$$XMPP_BASE/src/xmpp_vcard.cpp \
		$$XMPP_BASE/src/xmpp_message.cpp \
		$$XMPP_BASE/src/xmpp_tasks.cpp \
		$$XMPP_BASE/src/xmpp_jidlink.cpp \
		$$XMPP_BASE/src/xmpp_dtcp.cpp \
		$$XMPP_BASE/src/xmpp_ibb.cpp \
		$$XMPP_BASE/src/xmpp_client.cpp
}

