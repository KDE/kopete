#ifndef __LIB_EVA_H__
#define __LIB_EVA_H__

class QByteArray;

namespace Eva {
	short const Version = 0x0F15;

	char const Head = 0x02;
	char const Tail = 0x03;

	short const RequestLoginToken = 0x0062;
	

	/** 
	 * Header section for QQ packet
	 */
	QByteArray header( short const command, short const sequence );
	QByteArray loginToken( int id, short const sequence );
};


#endif
