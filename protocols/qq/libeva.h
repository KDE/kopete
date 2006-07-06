#ifndef __LIB_EVA_H__
#define __LIB_EVA_H__

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

namespace Eva {
	short const Version = 0x0F15;
	char const Head = 0x02;
	char const Tail = 0x03;

	short const RequestLoginToken = 0x0062;
	char const LoginTokenOK = 0x00;

	short const Login = 0x0022;
	char const LoginOK = 0x00;

	int const MaxPacketLength = 65535;
	int const HeaderLength = 13;
	int const KeyLength = 16;
	int const Md5KeyLength = 16;
	int const LoginLength = 416;

	const char NormalLogin = 0x0A;
	const char InvisibleLogin = 0x28;


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

		void duplicate( const char* d, int s )
		{
			if( m_itsOwn )
				free( m_data );
			m_data = (char*)malloc(s);
			m_size = m_capacity = s;
			m_itsOwn = true;
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
			m_body.duplicate( buffer+pos, len );
		}

		short version() const { return m_version; }
		short command() const { return m_command; }
		short sequence() const { return m_sequence; }
		ByteArray& body() { return m_body; }

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

	// Misc.
	ByteArray loginToken( const ByteArray& buffer );
	ByteArray QQHash( const ByteArray& text );


};


#endif
