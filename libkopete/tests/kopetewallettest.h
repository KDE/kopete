#ifndef KOPETEWALLETTEST_H
#define KOPETEWALLETTEST_H

#include <qobject.h>

namespace KWallet { class Wallet; }

class WalletReciever : public QObject
{
	Q_OBJECT
public slots:
	void timer();
	void gotWallet( KWallet::Wallet *w );
};

#endif
