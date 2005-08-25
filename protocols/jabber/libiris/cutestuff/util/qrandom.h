#ifndef CS_QRANDOM_H
#define CS_QRANDOM_H

#include<q3cstring.h>

class QRandom
{
public:
	static uchar randomChar();
	static uint randomInt();
	static QByteArray randomArray(uint size);
};

#endif
