/*
    Kopete Yahoo Protocol
    Copyright (c) 2004  Duncan Mac-Vicar Prett <duncan@kde.org>
    
    Based on code from: 
    
    YMSG Java API - http://jymsg9.sourceforge.net
    Copyright (c) S.E.Morris (FISH) 2003-04
    
    libyahoo2 - http://libyahoo2.sf.net
    Copyright (c) libyahoo2 Developers 
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHALLENGE_RESPONSE_V10
#define CHALLENGE_RESPONSE_V10

class ChallengeResponseV10
{
public:
	int XFRM( int table, int depth, int seed );
	unsigned int authFibonacci(unsigned int challenge, int divisor, int outer_loop, int inner_loop);
	unsigned char authRead45(unsigned int buffer, int offset);
	unsigned char authRead3(unsigned int buffer, int offset);
	unsigned int authType45(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int initial);
	unsigned int authType3(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int offset);
	unsigned int authType2(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int type_two_variable, int type_two_variable2);
	unsigned int authType1(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int type_one_variable);
	int authFinalCountdown(unsigned int challenge, int divisor, int inner_loop, int outer_loop);
};

#endif
