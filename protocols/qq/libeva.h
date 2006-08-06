#ifndef __LIB_EVA_H__
#define __LIB_EVA_H__

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <string>
#include <list>

namespace Eva {
	// magic number used in the packet
	short const Version = 0x0F15;
	char const Head = 0x02;
	char const Tail = 0x03;

	// command
	short const Logout = 0x0001;
	short const KeepAlive = 0x0002;
	short const UpdateInfo = 0x0004;
	short const Search = 0x0005;
	short const UserInfo = 0x0006;
	short const AddFriend = 0x0009;
	short const RemoveFriend = 0x000A;
	short const AuthInvite = 0x000B;
	short const ChangeStatus = 0x000D;
	short const AckSysMsg = 0x0012;
	short const SendMsg = 0x0016;
	short const ReceiveMsg = 0x0017;
	short const RemoveMe = 0x001C;
	short const RequestKey = 0x001D;
	short const GetCell = 0x0021;
	short const Login = 0x0022;
	short const ContactList = 0x0026;
	short const ContactsOnline = 0x0027;
	short const GetCell2 = 0x0029;
	short const SIP = 0x0030; // Special Interest Group == Qun( in Chinese )
	short const Test = 0x0031;
	short const GroupNames = 0x003C;
	short const UploadGroups = 0x003D;
	short const Memo = 0x003E;
	short const DownloadGroups = 0x0058;
	short const GetLevel = 0x005C;
	short const RequestLoginToken = 0x0062;
	short const ExtraInfo = 0x0065;
	short const Signature = 0x0067;
	short const ReceiveSysMsg = 0x0080;
	short const ContactStausChanged = 0x0081;

	// status
	char const Online = 10;
	char const Offline = 20;
	char const Away = 30;
	char const Invisible = 40;


	// reply
	char const LoginTokenOK = 0x00;
	char const LoginOK = 0x00;
	char const LoginRedirect = 0x01;
	char const LoginWrongPassword = 0x05;
	char const LoginMiscError = 0x06;
	char const ChangeStatusOK= 0x30;

	// Lengths
	int const MaxPacketLength = 65535;
	int const HeaderLength = 13;
	int const KeyLength = 16;
	int const Md5KeyLength = 16;
	int const LoginLength = 416;

	// Options
	const char NormalLogin = 0x0A;
	const char InvisibleLogin = 0x28;
	const char ContactListSorted = 0x01;
	const char ContactListUnsorted = 0x00;
	const char ContactListEnd = 0xff;
	const char UploadGroupNames = 0x2;
	const char DownloadGroupNames = 0x1;

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
	ByteArray contactList( int id, short const sequence, const ByteArray& key, short pos = 0);
	ByteArray getGroupNames( int id, short const sequence, ByteArray& key );
	ByteArray downloadGroups( int id, short const sequence, ByteArray& key, int pos );

	// Misc.
	ByteArray loginToken( const ByteArray& buffer );
	ByteArray QQHash( const ByteArray& text );
	ByteArray decrypt( const ByteArray& code, const ByteArray& key );
	const char* getInitKey();
	ByteArray buildPacket( int id, short const command, short const sequence, const ByteArray& key, const ByteArray& text );

	struct ContactStatus
	{
		int qqId;
		int ip;
		short port;
		char status;

		ContactStatus( const ByteArray& text ) :
			qqId( ntohl( type_cast<int> (text.data() )) ), 
			ip( ntohl( type_cast<int> (text.data()+5 )) ), 
			port( ntohs( type_cast<short> (text.data()+9 )) ), 
			status( type_cast<char> (text.data()+12 ))  {};
	};

};
#endif
