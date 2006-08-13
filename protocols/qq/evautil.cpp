#include "libeva.h"
#include "md5.h"
#include "crypt.h"

namespace Eva {

	int rand(void)
	{
		return 0xdead;
	}

	ByteArray doMd5( const ByteArray& text )
	{
		ByteArray code( Md5KeyLength );
		md5_state_t ctx;
		md5_init( &ctx );
		md5_append( &ctx, (md5_byte_t*) text.data(), text.size() );
		md5_finish( &ctx, (md5_byte_t*)code.data() );
		code.setSize( Md5KeyLength ); 
		return code;
	}

	ByteArray QQHash( const ByteArray& text )
	{
		return doMd5( doMd5( text ) );
	}

	inline void encrypt64( unsigned char* plain, unsigned char* plain_pre, 
			unsigned char* key, unsigned char* crypted, unsigned char* crypted_pre, 
			bool& isHeader )
	{
		int i;

		for( i = 0; i< 8; i++ )
			plain[i] ^= isHeader ? plain_pre[i] : crypted_pre[i];

		TEA::encipher( (unsigned int*) plain, (unsigned int*) key, 
				(unsigned int*) crypted );

		for( i = 0; i< 8; i++ )
			crypted[i] ^= plain_pre[i];

		memcpy( plain_pre, plain, 8 );
		memcpy( crypted_pre, crypted, 8 );
			
		isHeader = false;
	}

	inline void decrypt64( unsigned char* crypt, unsigned char* crypt_pre, 
			unsigned char* key, unsigned char* decrypted)
	{
		for( int i = 0; i< 8; i++ )
			decrypted[i] ^= crypt[i];

		TEA::decipher( (unsigned int*) decrypted,
				(unsigned int*) key, (unsigned int*) decrypted );
		/*
		fprintf( stderr, "decrypt64 : " );
		for( int i = 0; i< 8; i++ )
			fprintf( stderr, "%x ", decrypted[i] );
		fprintf( stderr, "\n" );
		*/
	}
	ByteArray encrypt( const ByteArray& text, const ByteArray& key )
	{

		unsigned char 
			plain[8],         /* plain text buffer*/
			plain_pre[8],   /* plain text buffer, previous 8 bytes*/
			crypted[8],        /* crypted text*/
			crypted_pre[8];  /* crypted test, previous 8 bytes*/
	
		int pos, len, i;
		bool isHeader = true;      /* header is one byte*/
		ByteArray encoded( text.size() + 32 );
		
		pos = ( text.size() + 10 ) % 8;
		if( pos )
			pos = 8 - pos;

		// Prepare the first 8 bytes:
		plain[0] = ( rand() & 0xf8 ) | pos;
		memset( plain_pre, 0, 8 );
		memset( crypted_pre, 0, 8 );
		memset( plain+1, rand()& 0xff, pos++ );

		// pad 2 bytes
		for( i = 0; i< 2; i++ )
		{
			if( pos < 8 )
				plain[pos++] = rand() & 0xff;

			if( pos == 8 )
			{
				encrypt64( plain, plain_pre, (unsigned char*)key.data(), crypted, crypted_pre, isHeader );
				pos = 0;
				encoded.append( (char*)crypted, 8 );
			}
		}

		for( i = 0; i< text.size(); i++ )
		{
			if( pos < 8 )
				plain[pos++] = text.data()[i];

			if( pos == 8 )
			{
				encrypt64( plain, plain_pre, (unsigned char*)key.data(), crypted, crypted_pre, isHeader );
				pos = 0;
				encoded.append( (char*)crypted, 8 );
			}
		}

		for( i = 0; i< 7; i++ )
		{
			if( pos < 8 )
				plain[pos++] = 0;

			if( pos == 8 )
			{
				encrypt64( plain, plain_pre, (unsigned char*)key.data(), crypted, crypted_pre, isHeader );
				encoded.append( (char*)crypted, 8 );
				break;
			}
		}

		return encoded;
	}

	ByteArray decrypt( const ByteArray& code, const ByteArray& key )
	{
		unsigned char
			decrypted[8], m[8],
			*crypt_pre, *crypt;
		char* outp;
		
		int pos, len, i;

		if( code.size() < 16 || code.size() % 8 )
			return ByteArray(0);

		TEA::decipher( (unsigned int*) code.data(), 
				(unsigned int*) key.data(), (unsigned int*) decrypted );
		pos = decrypted[0] & 0x7;
		len = code.size() - pos - 10;
		if( len < 0 )
			return ByteArray(0);

		ByteArray text(len);
		memset( m, 0, 8 );
		crypt = (unsigned char*)code.data() + 8;
		crypt_pre = m;
		pos ++;

		for( i = 0; i< 2; )
		{
			if( pos < 8 )
			{
				pos ++;
				i++;
			}
			if( pos == 8 )
			{
				crypt_pre = (unsigned char*) code.data();
				decrypt64( crypt, crypt_pre, (unsigned char*) key.data(), 
						decrypted ); 
				crypt += 8;
				pos = 0;
			}
		}
		for( i = 0; i< len;  )
		{
			if( pos < 8 )
			{
				text += (char) (crypt_pre[pos] ^ decrypted[pos]);
				pos ++;
				i ++;
			}
			if( pos == 8 )
			{
				crypt_pre = crypt - 8;
				decrypt64( crypt, crypt_pre, (unsigned char*) key.data(), decrypted );
				crypt += 8;
				pos = 0;
			}
		}
		
		for( i = 0; i< 7; i++ )
		{
			if( pos < 8 )
			{
				if( crypt_pre[pos] ^ decrypted[pos] )
					return ByteArray(0);
				pos ++;
			}
			if( pos == 8 )
			{
				crypt_pre = crypt;
				decrypt64( crypt, crypt_pre, (unsigned char*) key.data(), decrypted );
				break;
			}
				
		}

		return text;
	}
}

/* FIXME: using regular expression here ? */
std::string textToStream(const std::string &text, bool& hasImage)
{
	std::string converted = "";
	int offset=0;
	char smileyTag = 0x14;
	char customTag = 0x15;
	bool isFirst32 = true;
	std::string code32FileTag = "";
	char seperator32_1 = 0x13;
	char seperator32_2 = 0x4c;
	for(uint i=0; i< text.length(); i++){
		if(text[i] == '/'){
			offset = i;
			while(text[offset] != 0x00 && text[++offset]!=' ');
			if((offset - i)<8){
				// TODO: add the filter!
				char code = 5;// textToSmiley(text.substr(i,offset-i));
				if(code){
					converted += smileyTag;
					converted += code;
					i=offset;
					continue;
				}
			}
			converted+=text[i];
			continue;
		}else{
			if(text[i] == '['){
				std::string zdyTag = text.substr(i, 5);
				if(zdyTag == "[ZDY]"){
					offset = text.find("[/ZDY]", i);
					std::string zdyType = text.substr(i+6, 2);
					zdyTag = text.substr(i+5+4, offset-i-14);
					std::string sendFormat;
					sendFormat += customTag;
					if(zdyType == "32"){
						if(isFirst32){
							code32FileTag = zdyTag.substr(0, zdyTag.length() - 7);
							code32FileTag += seperator32_1;
							code32FileTag += seperator32_2;
							isFirst32 = false;
						}
						sendFormat += "2"; // 0x32
						sendFormat += zdyTag.substr(zdyTag.length() - 2, 2);
						sendFormat += "999999";
					} else	if(zdyType == "36"){
						sendFormat += "6"; // note: at the moment, we only use type 6
						int len = zdyTag.length() + 5; // the len includes tag(1) and type(1) and itself(3)
						char *strLen = new char[4];
						sprintf(strLen, "%3d", len);
						sendFormat += strLen;
						delete strLen;
						sendFormat += zdyTag;
					}
					converted += sendFormat;
					i += (offset - i + 5);
					continue;
				}
			}
		}
		converted+=text[i];
	}
	if(!isFirst32){
		converted = code32FileTag + converted; 
		hasImage = true;
	} else
		hasImage = false;
	return converted;
}


