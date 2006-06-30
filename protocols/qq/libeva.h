#ifndef __LIB_EVA_H__
#define __LIB_EVA_H__

#include <stdlib.h>
#include <iostream>

using namespace std;

namespace Eva {
	short const Version = 0x0F15;
	char const Head = 0x02;
	char const Tail = 0x03;
	short const RequestLoginToken = 0x0062;

	int const MaxPacketSize = 65536;

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
			{
				m_data = NULL;
				m_capacity = 0;
			}
			// FIXME: throw the exception if possible.
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
				//FIXME: throw Exception();
            m_data[m_size] = c;
            m_size++;
        }

        template<class T>void operator += (T d) 
		{
			char* p = (char*) &d;
			int len = sizeof(d);
            if( m_size+len >= m_capacity )
				return; // FIXME: throw later
			memcpy( m_data+m_size, p, len );
			m_size += len;
		}
                
        int capacity() { return m_capacity; }
        int size() { return m_size; }
        char* data() { return m_data; }

	private:
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
