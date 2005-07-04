/*
   Kopete Oscar Protocol - Chat Navigation parsing tests
   Copyright (c) 2005 Matt Rogers <mattr@kde.org>

   Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef CHATNAVTESTS_H
#define CHATNAVTESTS_H

#include "tester.h"

class Buffer;

/**
@author Kopete Developers
*/
class ChatNavTests : public Tester
{
public:
	ChatNavTests();
	~ChatNavTests();

	void allTests();

// 	void limitsParsingTest();
	void exchangeParsingTest();
	void roominfoParsingTest();
// 	void extRoomInfoParsingTest();
// 	void memberListParsingTest();
// 	void searchInfoParsingTest();
// 	void createRoomParsingTest();

	void setupExchangeTestBuffer();
	void setupRoomInfoTestBuffer();

private:
	Buffer* m_buffer;
};

#endif
