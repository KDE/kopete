#include"qrandom.h"

#include<stdlib.h>

uchar QRandom::randomChar()
{
	return rand();
}

uint QRandom::randomInt()
{
	QByteArray a = randomArray(sizeof(uint));
	uint x;
	memcpy(&x, a.data(), a.size());
	return x;
}

QByteArray QRandom::randomArray(uint size)
{
	QByteArray a(size);
	for(uint n = 0; n < size; ++n)
		a[n] = randomChar();
	return a;
}
