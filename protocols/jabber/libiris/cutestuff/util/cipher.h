/*
 * cipher.h - Simple wrapper to 3DES,AES128/256 CBC ciphers
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

#ifndef CS_CIPHER_H
#define CS_CIPHER_H

#include<qstring.h>
#include<qcstring.h>

namespace Cipher
{
	enum Type { None, TripleDES, AES_128, AES_256 };

	class Key
	{
	public:
		Key() { v_type = None; }

		bool isValid() const { return (v_type == None ? false: true); }
		void setType(Type x) { v_type = x; }
		Type type() const { return v_type; }
		void setData(const QByteArray &d) { v_data = d; }
		const QByteArray & data() const { return v_data; }

	private:
		Type v_type;
		QByteArray v_data;
	};

	Key generateKey(Type);
	QByteArray generateIV(Type);
	int ivSize(Type);
	QByteArray encrypt(const QByteArray &, const Key &, const QByteArray &iv, bool pad, bool *ok=0);
	QByteArray decrypt(const QByteArray &, const Key &, const QByteArray &iv, bool pad, bool *ok=0);
}

class RSAKey
{
public:
	RSAKey();
	RSAKey(const RSAKey &);
	RSAKey & operator=(const RSAKey &);
	~RSAKey();

	bool isNull() const;
	void *data() const;
	void setData(void *);

private:
	class Private;
	Private *d;

	void free();
};

RSAKey generateRSAKey();
QByteArray encryptRSA(const QByteArray &buf, const RSAKey &key, bool *ok=0);
QByteArray decryptRSA(const QByteArray &buf, const RSAKey &key, bool *ok=0);
QByteArray encryptRSA2(const QByteArray &buf, const RSAKey &key, bool *ok=0);
QByteArray decryptRSA2(const QByteArray &buf, const RSAKey &key, bool *ok=0);

#endif
