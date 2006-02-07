METASOURCES = AUTO

AM_CPPFLAGS = -DBOTAN_TYPES_QT -DBOTAN_NO_INIT_H -DBOTAN_NO_CONF_H -DBOTAN_TOOLS_ONLY -DBOTAN_MINIMAL_BIGINT \
	-DBOTAN_MP_WORD_BITS=32 -DBOTAN_VECTOR_OVER_ALLOCATE=4 -DQCA_SYSTEMSTORE_PATH=\"$(datadir)/qca/certs/rootcerts.pem\"

noinst_LTLIBRARIES = libqca2.la
INCLUDES = -I../include -I../include/QtCrypto -I./support/dirwatch -I./botantools -I./botantools/botan $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

libqca2_la_LIBADD = $(LIB_QTCORE)
libqca2_la_SOURCES = \
	qca_tools.cpp \
	qca_core.cpp \
	qca_textfilter.cpp \
	qca_plugin.cpp \
	qca_basic.cpp \
	qca_publickey.cpp \
	qca_cert.cpp \
	qca_keystore.cpp \
	qca_securelayer.cpp \
	qca_securemessage.cpp \
	qca_default.cpp \
	qca_systemstore_flatfile.cpp \
	support/synchronizer.cpp \
	support/dirwatch/dirwatch.cpp \
	support/dirwatch/dirwatch_unix.cpp \
	./botantools/botan/util.cpp \
	./botantools/botan/exceptn.cpp \
	./botantools/botan/mutex.cpp \
	./botantools/botan/mux_qt/mux_qt.cpp \
	./botantools/botan/secalloc.cpp \
	./botantools/botan/defalloc.cpp \
	./botantools/botan/allocate.cpp \
	./botantools/botan/mp_core.cpp \
	./botantools/botan/mp_comba.cpp \
	./botantools/botan/mp_fkmul.cpp \
	./botantools/botan/mp_mul.cpp \
	./botantools/botan/mp_smul.cpp \
	./botantools/botan/mp_shift.cpp \
	./botantools/botan/mp_misc.cpp \
	./botantools/botan/numthry.cpp \
	./botantools/botan/divide.cpp \
	./botantools/botan/big_base.cpp \
	./botantools/botan/big_code.cpp \
	./botantools/botan/big_io.cpp \
	./botantools/botan/big_ops2.cpp \
	./botantools/botan/big_ops3.cpp \
	./botantools/botan/ml_unix/mlock.cpp \
	./botantools/botan/alloc_mmap/mmap_mem.cpp

CLEANFILES = qca_publickey.moc qca_keystore.moc qca_securelayer.moc qca_securemessage.moc \
		qca_default.moc synchronizer.moc dirwatch.moc dirwatch_unix.moc

qca_publickey.lo: qca_publickey.moc
qca_publickey.moc: $(srcdir)/qca_publickey.cpp ../include/QtCrypto/qca_publickey.h
	${MOC} ../include/QtCrypto/qca_publickey.h > $@
	${MOC} $(srcdir)/qca_publickey.cpp >> $@

qca_keystore.lo: qca_keystore.moc
qca_keystore.moc: $(srcdir)/qca_keystore.cpp ../include/QtCrypto/qca_keystore.h
	${MOC} ../include/QtCrypto/qca_keystore.h > $@
	${MOC} $(srcdir)/qca_keystore.cpp >> $@

qca_securelayer.lo: qca_securelayer.moc
qca_securelayer.moc: $(srcdir)/qca_securelayer.cpp ../include/QtCrypto/qca_securelayer.h
	${MOC} ../include/QtCrypto/qca_securelayer.h > $@
	${MOC} $(srcdir)/qca_securelayer.cpp >> $@

qca_securemessage.lo: qca_securemessage.moc
qca_securemessage.moc: $(srcdir)/qca_securemessage.cpp ../include/QtCrypto/qca_securemessage.h
	${MOC} ../include/QtCrypto/qca_securemessage.h > $@
	${MOC} $(srcdir)/qca_securemessage.cpp >> $@

qca_default.lo: qca_default.moc
qca_default.moc: $(srcdir)/qca_default.cpp
	${MOC} $(srcdir)/qca_default.cpp > $@

synchronizer.lo: synchronizer.moc
synchronizer.moc: $(srcdir)/support/synchronizer.cpp
	${MOC} $(srcdir)/support/synchronizer.cpp > $@

dirwatch.lo: dirwatch.moc
dirwatch.moc: $(srcdir)/support/dirwatch/dirwatch.cpp $(srcdir)/support/dirwatch/dirwatch_p.h
	${MOC} $(srcdir)/support/dirwatch/dirwatch_p.h > $@
	${MOC} $(srcdir)/support/dirwatch/dirwatch.cpp >> $@

dirwatch_unix.lo: dirwatch_unix.moc
dirwatch_unix.moc: $(srcdir)/support/dirwatch/dirwatch_unix.cpp $(srcdir)/support/dirwatch/dirwatch_p.h
	${MOC} $(srcdir)/support/dirwatch/dirwatch_p.h > $@
	${MOC} $(srcdir)/support/dirwatch/dirwatch_unix.cpp >> $@