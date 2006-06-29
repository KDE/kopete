#ifndef __LIB_EVA_H__
#define __LIB_EVA_H__

#include <stdlib.h>
#include <iostream>

using namespace std;

namespace Eva {
	short const Version = 0x0F15;

	char const Head = 0x02;
	char const Tail = 0x03;
	int const MaxPacketSize = 65536;

	short const RequestLoginToken = 0x0062;

	class Exception {};
	

	class ByteArray
	{
	public:
		ByteArray( int capacity )
		{
			m_capacity = capacity;
            m_size = 0;
            m_data = (char*) malloc(capacity);
			if(! m_data )
				m_data = NULL;
				//throw Exception();
		}

		void destory()
		{
			m_size = m_capacity = 0;
			if( !m_data )
				free( m_data );
		}
		
		void operator += (char c)
        {
            if( m_size >= m_capacity )
				return;
				//throw Exception();
            m_data[m_size] = c;
            m_size++;
        }

        template<class T>void operator += (T s) { this->append<T>(s); }
                
        int capacity() { return m_capacity; }
        int size() { return m_size; }
        char* data() { return m_data; }
		private:
        
        template< class T> void append( T d )
        {
			for( int i = 0; i< sizeof(d); i++ )
            {
				char c = d & 0xFF;
                (*this) += c;
                d >>= 8;
            }
        }

        int m_capacity;
        int m_size;
        char* m_data;
	};


	/** 
	 * Header section for QQ packet
	 */
	ByteArray header( short const command, short const sequence );
	ByteArray loginToken( int id, short const sequence );
};


#endif
