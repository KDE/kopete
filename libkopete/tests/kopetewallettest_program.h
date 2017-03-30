#ifndef KOPETEWALLETTEST_H
#define KOPETEWALLETTEST_H

#include <qobject.h>

namespace KWallet {
class Wallet;
}

class WalletReciever : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void timer();
private Q_SLOTS:
    void gotWallet(KWallet::Wallet *w);
};

#endif
