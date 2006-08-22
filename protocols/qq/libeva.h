#ifndef __LIB_EVA_H__
#define __LIB_EVA_H__

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <string>
#include <list>

namespace Eva {
	// NOTICE: the length of the following data declarations are:
	// uchar : 8bit
	// ushort: 16bit
	// uint  : 32bit
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;

	// magic number 
	static const ushort Version = 0x0F15;
	static const uchar Head = 0x02;
	static const uchar Tail = 0x03;

	// command
	static const ushort Logout = 0x0001;
	static const ushort Heartbeat = 0x0002;
	static const ushort UpdateInfo = 0x0004;
	static const ushort Search = 0x0005;
	static const ushort UserInfo = 0x0006;
	static const ushort AddFriend = 0x0009;
	static const ushort RemoveFriend = 0x000A;
	static const ushort AuthInvite = 0x000B;
	static const ushort ChangeStatus = 0x000D;
	static const ushort AckSysMsg = 0x0012;
	static const ushort SendMsg = 0x0016;
	static const ushort ReceiveMsg = 0x0017;
	static const ushort RemoveMe = 0x001C;
	static const ushort RequestKey = 0x001D;
	static const ushort GetCell = 0x0021;
	static const ushort Login = 0x0022;
	static const ushort ContactList = 0x0026;
	static const ushort ContactsOnline = 0x0027;
	static const ushort GetCell2 = 0x0029;
	static const ushort SIP = 0x0030; // Special Interest Group == Qun( in Chinese )
	static const ushort Test = 0x0031;
	static const ushort GroupNames = 0x003C;
	static const ushort UploadGroups = 0x003D;
	static const ushort Memo = 0x003E;
	static const ushort DownloadGroups = 0x0058;
	static const ushort GetLevel = 0x005C;
	static const ushort RequestLoginToken = 0x0062;
	static const ushort ExtraInfo = 0x0065;
	static const ushort Signature = 0x0067;
	static const ushort ReceiveSysMsg = 0x0080;
	static const ushort ContactStausChanged = 0x0081;

	// Options
	static const uchar NormalLogin = 0x0A;
	static const uchar InvisibleLogin = 0x28;
	static const uchar ContactListSorted = 0x01;
	static const uchar ContactListUnsorted = 0x00;
	static const uchar ContactListBegin = 0x00;
	static const uchar ContactListEnd = 0xff;
	static const uchar UploadGroupNames = 0x2;
	static const uchar DownloadGroupNames = 0x1;
	static const uchar TransferKey = 0x03; // file agent key in eva

	// reply
	static const uchar LoginTokenOK = 0x00;
	static const uchar LoginOK = 0x00;
	static const uchar LoginRedirect = 0x01;
	static const uchar LoginWrongPassword = 0x05;
	static const uchar LoginMiscError = 0x06;
	static const uchar ChangeStatusOK= 0x30;
	static const uchar RequestKeyOK = 0x00;

	// status
	static const uchar Online = 10;
	static const uchar Offline = 20;
	static const uchar Away = 30;
	static const uchar Invisible = 40;


	// IM operation( sending )
	static const ushort IMText = 0x000b;
	static const ushort IMNotifyIP = 0x003b;

	// IM reply types
	static const uchar NormalReply = 0x01;
	static const uchar AutoReply = 0x02;
	static const uchar ImageReply = 0x05;

	// Encoding
	static const ushort GBEncoding = 0x8602;

	// IM command ( receiving )
	static const ushort RcvFromBuddy = 0x0009;

	// Lengths
	static const uint MaxPacketLength = 65535;
	static const uint HeaderLength = 13;
	static const uint KeyLength = 16;
	static const uint LoginLength = 416;

	// POD storage
	struct ContactInfo {
		int id;
		short face;
		char age;
		char gender;
		std::string nick;
	};

	// Contact-Group-Trio
	struct CGT { 
		int qqId;
		char type;
		char groupId;

		CGT( int q, char t, char g ) : qqId(q), type (t), groupId(g) {};
	};


	// Customized max to get rid of stl dependence
	template<class T> T max( T a, T b) { return (a>b) ? a : b; }
	template<class T> T min( T a, T b) { return (a<b) ? a : b; }


    class ByteArray
    {
    public:
        ByteArray( int capacity=0 ) : m_itsOwn(capacity>0), m_capacity(capacity), 
                                       m_size(0), m_data((char*) malloc(capacity))
		{ }
        ByteArray( char* p, int size) : m_itsOwn(p!=NULL), m_capacity(size), 
                                       m_size(size), m_data(p)
		{ }
        
        ~ByteArray() { if( m_itsOwn ) free(m_data); }
        
        ByteArray( const ByteArray& r ) : m_itsOwn(r.m_itsOwn), m_capacity(r.capacity()), 
										  m_size(r.size()), m_data(r.release()) 
		{ }

        ByteArray& operator= ( const ByteArray& r )
        {
            if( &r != this )
            {
                if( m_data != r.m_data )
                {
                    if( m_itsOwn )
                        free(m_data);
                    m_itsOwn = r.m_itsOwn;
                }
                else
                    if( r.m_itsOwn )
                        m_itsOwn = true;
                m_data = r.release();
                m_size = r.size();
                m_capacity = r.capacity();
            }
            return *this;
        }

        char* release() const
        {
            (const_cast<ByteArray* >(this))->m_itsOwn = false;
            return m_data;
        }

        void copyAt( int index, const char* buf, int length )
        {
            // FIXME: use CONFIG to throw exception if possible
			if( index+length > m_capacity )
				return;
			memcpy( m_data+index, buf, length );
			m_size = max( index+length, m_size );
        }

        template<class T> void copyAt(int index, T d)
        {
            copyAt( index, (const char*)&d, sizeof(d) );
        }

        template<class T> void operator+= (T d)
        {
            copyAt<T>(m_size, d );
        }

        void operator+=(const ByteArray& d)
        {
            copyAt(m_size, d.data(), d.size());
        }

		void append( const char* d, int s )
		{
			copyAt( m_size, d, s );
		}

		static ByteArray duplicate( const char* d, int s )
		{
			ByteArray x(s);
			x.append( d, s );
			return x;
		}

        int size() const { return m_size; }
		void setSize( int size ) { if( size<= m_capacity ) m_size = size; }
        int capacity() const { return m_capacity; }
        char* data() const { return m_data; }

    private:
        bool m_itsOwn;
        int m_capacity;
        int m_size;
        char* m_data;

    };

    inline ByteArray operator+ ( ByteArray l, ByteArray r )
    {  
        ByteArray sum( l.size() + r.size() );
        sum += l;
        sum += r;
        return sum;
    }
   
	template<class T> inline T type_cast(char* buffer)
	{
		return (* ((T*) buffer) );
	}
	
	struct ContactStatus
	{
		int qqId;
		int ip;
		short port;
		char status;

		ContactStatus( char* data ) :
			qqId( ntohl( type_cast<int> (data )) ), 
			ip( ntohl( type_cast<int> (data+5 )) ), 
			port( ntohs( type_cast<short> (data+9 )) ), 
			status( type_cast<char> (data+12 ))  {};
	};

	struct MessageEnvelop
	{
		int sender;
		int receiver;
		int sequence;
		int ip;
		short port;
		short type;

		MessageEnvelop( const ByteArray& text ) :
			sender( ntohl( type_cast<int>(text.data())) ),
			receiver( ntohl( type_cast<int>(text.data() + 4 )) ),
			sequence( ntohl( type_cast<int>( text.data() + 8 )) ),
			ip( ntohl( type_cast<int>( text.data() + 12 )) ),
			port( ntohs( type_cast<short>( text.data() + 16 )) ),
			type( ntohs( type_cast<short>( text.data() + 18 )) )  {};
	};

	struct MessageHeader
	{
	// pack me !
		short version;
		int sender;
		int receiver;
		ByteArray transferKey;
		short type;
		short sequence;
		int timestamp;
		short avatar;

		MessageHeader( const ByteArray& text ) :
			version( ntohs( type_cast<short>(text.data())) ),
			sender( ntohl( type_cast<int>(text.data()+2)) ),
			receiver( ntohl( type_cast<int>(text.data() + 6 )) ),
			transferKey( text.data()+10, 16),
			type( ntohs( type_cast<short>( text.data() + 26)) ),
			sequence( ntohs( type_cast<short>( text.data() + 28)) ),
			timestamp( ntohl( type_cast<int>( text.data() + 30)) ),
			avatar( ntohs( type_cast<short>( text.data() + 34)) ) {};
	};

	/** 
	 * normalized QQ packet
	 */
	class Packet
	{
	public:
		Packet( const char* buffer, int size )
		{
			// FIXME: TCP packet.
			// FIXME: Add error handling
			int pos = 3;
			m_version = ntohs( *((short*)(buffer+pos)) );
			pos += 2;

			m_command = ntohs( *((short*)(buffer+pos)) );
			pos += 2;

			m_sequence = ntohs( *((short*)(buffer+pos)) );
			pos += 2;

			int len = size - pos - 1; // 1 is tail
			m_body = ByteArray::duplicate( buffer+pos, len );
		}

		short version() const { return m_version; }
		short command() const { return m_command; }
		short sequence() const { return m_sequence; }
		ByteArray& body() { return m_body; }

		// FIXME: Add const to data.
		static inline char replyCode( ByteArray& data ) { return data.data()[0]; }

		static inline int redirectedIP( ByteArray& data ) 
		{ return ntohl( type_cast<int> (data.data()+5) ); }

		static inline int redirectedPort( ByteArray& data ) 
		{ return ntohs( type_cast<short> (data.data()+9) ); }

		static inline ByteArray sessionKey( ByteArray& data ) 
		{ return ByteArray::duplicate( data.data()+1, KeyLength ); }

		static inline ByteArray transferKey( ByteArray& data ) 
		{ return ByteArray::duplicate( data.data()+2, KeyLength ); }

		static inline ByteArray replyKey( ByteArray& data ) 
		{ return ByteArray::duplicate( data.data(), KeyLength ); }

		static inline ByteArray transferToken( ByteArray& data ) 
		{ return ByteArray::duplicate( data.data()+2+KeyLength+13, (unsigned)(data.data()[2+KeyLength+12]) ); }

		static inline int remoteIP( ByteArray& data ) 
		{ return ntohl( type_cast<int> (data.data()+27) ); }

		static inline int remotePort( ByteArray& data ) 
		{ return ntohs( type_cast<short> (data.data()+31) ); }

		static inline int localIP( ByteArray& data ) 
		{ return ntohl( type_cast<int> (data.data()+21) ); }

		static inline int localPort( ByteArray& data ) 
		{ return ntohs( type_cast<short> (data.data()+25) ); }

		static inline int loginTime( ByteArray& data ) 
		{ return ntohl( type_cast<int> (data.data()+33) ); }

		static inline int lastLoginFrom( ByteArray& data ) 
		{ return ntohl( type_cast<int> (data.data()+123) ); }

		static inline int lastLoginTime( ByteArray& data ) 
		{ return ntohl( type_cast<int> (data.data()+127) ); }

		static inline int nextGroupId( const ByteArray& data ) 
		{ return ntohl( type_cast<int> (data.data()+6) ); }

		static std::list< std::string > groupNames(const ByteArray& text );
		// FIXME: use list as others
		ContactInfo contactInfo( char* buffer, int& len );
		static std::list< CGT > cgts( const ByteArray& text );
		static std::list< ContactStatus > onlineContacts( const ByteArray& text, char& pos );

	private:
		short m_version;
		short m_command;
		short m_sequence;
		ByteArray m_body;
	};

	
	// Packet operation
	ByteArray requestLoginToken( int id, short const sequence );
	ByteArray login( int id, short const sequence, const ByteArray& key, 
			const ByteArray& token, char const loginMode );
	ByteArray changeStatus( int id, short const sequence, ByteArray& key, char status );
	ByteArray userInfo( int id, short const sequence, ByteArray& key, int qqId );
	ByteArray requestTransferKey( int id, short const sequence, const ByteArray& key );
	ByteArray contactList( int id, short const sequence, const ByteArray& key, short pos = 0);
	ByteArray getGroupNames( int id, short const sequence, ByteArray& key );
	ByteArray downloadGroups( int id, short const sequence, ByteArray& key, int pos );
	ByteArray textMessage( int id, short const sequence, ByteArray& key, int toId, const ByteArray& transferKey, ByteArray& message );
	ByteArray messageReply(int id, short const sequence, ByteArray& key, const ByteArray& text );
	ByteArray heartbeat(int id, short const sequence, ByteArray& key );
	ByteArray onlineContacts(int id, short const sequence, const ByteArray& key, char pos );

	// Misc.
	ByteArray loginToken( const ByteArray& buffer );
	ByteArray QQHash( const ByteArray& text );
	ByteArray decrypt( const ByteArray& code, const ByteArray& key );
	const char* getInitKey();
	ByteArray buildPacket( int id, short const command, short const sequence, const ByteArray& key, const ByteArray& text );


};
#endif
