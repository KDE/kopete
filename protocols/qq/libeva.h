#ifndef __LIB_EVA_H__
#define __LIB_EVA_H__

#include <string.h>
#include <stdio.h>

namespace Eva {
	short const Version = 0x0F15;
	char const Head = 0x02;
	char const Tail = 0x03;
	short const RequestLoginToken = 0x0062;
	char const LoginTokenOK = 0x00;

	int const MaxPacketSize = 65536;

	// Customized max to get rid of stl dependence
	template<class T> T max( T a, T b) { return (a>b) ? a : b; }


    class ByteArray
    {
    public:
        ByteArray( int capacity=0 ) : m_itsOwn(true), m_capacity(capacity), 
                                       m_size(0), m_data(new char(capacity))
		{
			fprintf( stderr, "m_itsOwn = %d, m_capacity = %d, m_size = %d, m_data = %x\n", 
					m_itsOwn, m_capacity, m_size, m_data );
		}
        
        ~ByteArray() { 
			fprintf( stderr, "~ m_itsOwn = %d, m_capacity = %d, m_size = %d, m_data = %x\n", 
					m_itsOwn, m_capacity, m_size, m_data );
			if( m_itsOwn ) 
				delete[] m_data; 
		}
        
        ByteArray( const ByteArray& r ) : m_itsOwn(r.m_itsOwn), m_capacity(r.capacity()), m_size(r.size()), m_data(r.release()) {
			fprintf( stderr, "m_itsOwn = %d, m_capacity = %d, m_size = %d, m_data = %x\n", 
					m_itsOwn, m_capacity, m_size, m_data );
		}
        ByteArray& operator= ( const ByteArray& r )
        {
            if( &r != this )
            {
                if( m_data != r.m_data )
                {
                    if( m_itsOwn )
                        delete []m_data;
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
			fprintf(stderr, "!1!" );
            copyAt<T>(m_size, d );
        }

        void append(const ByteArray& d)
        {
			fprintf(stderr, "!2!" );
            copyAt(m_size, d.data(), d.size());
        }

        int size() const { return m_size; }
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
	 * Header section for QQ packet
	 */
	ByteArray requestLoginToken( int id, short const sequence );
	ByteArray loginToken( char const* buffer );
	ByteArray header( int id, short const command, short const sequence );
};


#endif
