# QSSL qmake profile

TEMPLATE = lib
CONFIG  += qt thread release plugin
TARGET   = qssl

VER_MAJ = 2

# RH 9
INCLUDEPATH += /usr/kerberos/include

HEADERS = qssl.h qssl_p.h
SOURCES = qssl.cpp

# link with OpenSSL
LIBS += -lssl -lcrypto

