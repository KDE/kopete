/*
 * cipher.cpp - Simple wrapper to 3DES,AES128/256 CBC ciphers
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include"cipher.h"

#include<openssl/evp.h>
#include<openssl/rsa.h>
#include"bytestream.h"
#include"qrandom.h"

static bool lib_encryptArray(const EVP_CIPHER *type, const QByteArray &buf, const QByteArray &key, const QByteArray &iv, bool pad, QByteArray *out)
{
	QByteArray result(buf.size()+type->block_size);
	int len;
	EVP_CIPHER_CTX c;

	unsigned char *ivp = NULL;
	if(!iv.isEmpty())
		ivp = (unsigned char *)iv.data();
	EVP_CIPHER_CTX_init(&c);
	//EVP_CIPHER_CTX_set_padding(&c, pad ? 1: 0);
	if(!EVP_EncryptInit_ex(&c, type, NULL, (unsigned char *)key.data(), ivp))
		return false;
	if(!EVP_EncryptUpdate(&c, (unsigned char *)result.data(), &len, (unsigned char *)buf.data(), buf.size()))
		return false;
	result.resize(len);
	if(pad) {
		QByteArray last(type->block_size);
		if(!EVP_EncryptFinal_ex(&c, (unsigned char *)last.data(), &len))
			return false;
		last.resize(len);
		ByteStream::appendArray(&result, last);
	}

	memset(&c, 0, sizeof(EVP_CIPHER_CTX));
	*out = result;
	return true;
}

static bool lib_decryptArray(const EVP_CIPHER *type, const QByteArray &buf, const QByteArray &key, const QByteArray &iv, bool pad, QByteArray *out)
{
	QByteArray result(buf.size()+type->block_size);
	int len;
	EVP_CIPHER_CTX c;

	unsigned char *ivp = NULL;
	if(!iv.isEmpty())
		ivp = (unsigned char *)iv.data();
	EVP_CIPHER_CTX_init(&c);
	//EVP_CIPHER_CTX_set_padding(&c, pad ? 1: 0);
	if(!EVP_DecryptInit_ex(&c, type, NULL, (unsigned char *)key.data(), ivp))
		return false;
	if(!pad) {
		if(!EVP_EncryptUpdate(&c, (unsigned char *)result.data(), &len, (unsigned char *)buf.data(), buf.size()))
			return false;
	}
	else {
		if(!EVP_DecryptUpdate(&c, (unsigned char *)result.data(), &len, (unsigned char *)buf.data(), buf.size()))
			return false;
	}
	result.resize(len);
	if(pad) {
		QByteArray last(type->block_size);
		if(!EVP_DecryptFinal_ex(&c, (unsigned char *)last.data(), &len))
			return false;
		last.resize(len);
		ByteStream::appendArray(&result, last);
	}

	memset(&c, 0, sizeof(EVP_CIPHER_CTX));
	*out = result;
	return true;
}

static bool lib_generateKeyIV(const EVP_CIPHER *type, const QByteArray &data, const QByteArray &salt, QByteArray *key, QByteArray *iv)
{
	QByteArray k, i;
	unsigned char *kp = 0;
	unsigned char *ip = 0;
	if(key) {
		k.resize(type->key_len);
		kp = (unsigned char *)k.data();
	}
	if(iv) {
		i.resize(type->iv_len);
		ip = (unsigned char *)i.data();
	}
	if(!EVP_BytesToKey(type, EVP_sha1(), (unsigned char *)salt.data(), (unsigned char *)data.data(), data.size(), 1, kp, ip))
		return false;
	if(key)
		*key = k;
	if(iv)
		*iv = i;
	return true;
}

static const EVP_CIPHER * typeToCIPHER(Cipher::Type t)
{
	if(t == Cipher::TripleDES)
		return EVP_des_ede3_cbc();
	else if(t == Cipher::AES_128)
		return EVP_aes_128_cbc();
	else if(t == Cipher::AES_256)
		return EVP_aes_256_cbc();
	else
		return 0;
}

Cipher::Key Cipher::generateKey(Type t)
{
	Key k;
	const EVP_CIPHER *type = typeToCIPHER(t);
	if(!type)
		return k;
	QByteArray out;
	if(!lib_generateKeyIV(type, QRandom::randomArray(128), QRandom::randomArray(2), &out, 0))
		return k;
	k.setType(t);
	k.setData(out);
	return k;
}

QByteArray Cipher::generateIV(Type t)
{
	const EVP_CIPHER *type = typeToCIPHER(t);
	if(!type)
		return QByteArray();
	QByteArray out;
	if(!lib_generateKeyIV(type, QCString("Get this man an iv!"), QByteArray(), 0, &out))
		return QByteArray();
	return out;
}

int Cipher::ivSize(Type t)
{
	const EVP_CIPHER *type = typeToCIPHER(t);
	if(!type)
		return -1;
	return type->iv_len;
}

QByteArray Cipher::encrypt(const QByteArray &buf, const Key &key, const QByteArray &iv, bool pad, bool *ok)
{
	if(ok)
		*ok = false;
	const EVP_CIPHER *type = typeToCIPHER(key.type());
	if(!type)
		return QByteArray();
	QByteArray out;
	if(!lib_encryptArray(type, buf, key.data(), iv, pad, &out))
		return QByteArray();

	if(ok)
		*ok = true;
	return out;
}

QByteArray Cipher::decrypt(const QByteArray &buf, const Key &key, const QByteArray &iv, bool pad, bool *ok)
{
	if(ok)
		*ok = false;
	const EVP_CIPHER *type = typeToCIPHER(key.type());
	if(!type)
		return QByteArray();
	QByteArray out;
	if(!lib_decryptArray(type, buf, key.data(), iv, pad, &out))
		return QByteArray();

	if(ok)
		*ok = true;
	return out;
}


class RSAKey::Private
{
public:
	Private() {}

	RSA *rsa;
	int ref;
};

RSAKey::RSAKey()
{
	d = 0;
}

RSAKey::RSAKey(const RSAKey &from)
{
	d = 0;
	*this = from;
}

RSAKey & RSAKey::operator=(const RSAKey &from)
{
	free();

	if(from.d) {
		d = from.d;
		++d->ref;
	}

	return *this;
}

RSAKey::~RSAKey()
{
	free();
}

bool RSAKey::isNull() const
{
	return d ? false: true;
}

void * RSAKey::data() const
{
	if(d)
		return (void *)d->rsa;
	else
		return 0;
}

void RSAKey::setData(void *p)
{
	free();

	if(p) {
		d = new Private;
		d->ref = 1;
		d->rsa = (RSA *)p;
	}
}

void RSAKey::free()
{
	if(!d)
		return;

	--d->ref;
	if(d->ref <= 0) {
		RSA_free(d->rsa);
		delete d;
	}
	d = 0;
}

RSAKey generateRSAKey()
{
	RSA *rsa = RSA_generate_key(1024, RSA_F4, NULL, NULL);
	RSAKey key;
	if(rsa)
		key.setData(rsa);
	return key;
}

QByteArray encryptRSA(const QByteArray &buf, const RSAKey &key, bool *ok)
{
	if(ok)
		*ok = false;

	int size = RSA_size((RSA *)key.data());
	int flen = buf.size();
	if(flen >= size - 11)
		flen = size - 11;
	QByteArray result(size);
	unsigned char *from = (unsigned char *)buf.data();
	unsigned char *to = (unsigned char *)result.data();
	int r = RSA_public_encrypt(flen, from, to, (RSA *)key.data(), RSA_PKCS1_PADDING);
	if(r == -1)
		return QByteArray();
	result.resize(r);

	if(ok)
		*ok = true;
	return result;
}

QByteArray decryptRSA(const QByteArray &buf, const RSAKey &key, bool *ok)
{
	if(ok)
		*ok = false;

	int size = RSA_size((RSA *)key.data());
	int flen = buf.size();
	QByteArray result(size);
	unsigned char *from = (unsigned char *)buf.data();
	unsigned char *to = (unsigned char *)result.data();
	int r = RSA_private_decrypt(flen, from, to, (RSA *)key.data(), RSA_PKCS1_PADDING);
	if(r == -1)
		return QByteArray();
	result.resize(r);

	if(ok)
		*ok = true;
	return result;
}

QByteArray encryptRSA2(const QByteArray &buf, const RSAKey &key, bool *ok)
{
	if(ok)
		*ok = false;

	int size = RSA_size((RSA *)key.data());
	int flen = buf.size();
	if(flen >= size - 41)
		flen = size - 41;
	QByteArray result(size);
	unsigned char *from = (unsigned char *)buf.data();
	unsigned char *to = (unsigned char *)result.data();
	int r = RSA_public_encrypt(flen, from, to, (RSA *)key.data(), RSA_PKCS1_OAEP_PADDING);
	if(r == -1)
		return QByteArray();
	result.resize(r);

	if(ok)
		*ok = true;
	return result;
}

QByteArray decryptRSA2(const QByteArray &buf, const RSAKey &key, bool *ok)
{
	if(ok)
		*ok = false;

	int size = RSA_size((RSA *)key.data());
	int flen = buf.size();
	QByteArray result(size);
	unsigned char *from = (unsigned char *)buf.data();
	unsigned char *to = (unsigned char *)result.data();
	int r = RSA_private_decrypt(flen, from, to, (RSA *)key.data(), RSA_PKCS1_OAEP_PADDING);
	if(r == -1)
		return QByteArray();
	result.resize(r);

	if(ok)
		*ok = true;
	return result;
}
