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

#include "challengeresponsev10.h"
#include "challengeresponsev10tables.h"
#include "challengeresponsev10authfn.h"

#define A( x ) (( x ) & 0xFF )
#define B( x ) (( x ) >> 8 & 0xFF )
#define C( x ) (( x ) >> 16 & 0xFF )
#define D( x ) (( x ) >> 24 & 0xFF )

int ChallengeResponseV10::XFRM( int table, int depth, int seed )
{
   struct yahoo_fn *xfrm;
   int i, j, z;
   unsigned int n = seed;
   unsigned char *arg;

   for( i = 0; i < depth; i++ )
   {
      xfrm = &yahoo_fntable[table][n % 96];
      switch( xfrm->type )
      {
      case IDENT:
         return seed;
      case XOR:
         seed ^= xfrm->arg1;
         break;
      case MULADD:
         seed = seed * xfrm->arg1 + xfrm->arg2;
         break;
      case LOOKUP:
         arg = (unsigned char *)xfrm->arg1;
         seed = arg[A( seed )] | arg[B( seed )] << 8 | arg[C( seed )] << 16
            | arg[D( seed )] << 24;
         break;
      case BITFLD:
         arg = (unsigned char *)xfrm->arg1;
         for( j = 0, z = 0; j < 32; j++ )
            z = ((( seed >> j ) & 1 ) << arg[j] ) | ( ~( 1 << arg[j] ) & z );
         seed = z;
         break;
      }
      if( depth - i == 1 )
         return seed;
      z = (((((( A( seed ) * 0x9E3779B1 ) ^ B( seed )) * 0x9E3779B1 )
         ^ C( seed )) * 0x9E3779B1 ) ^ D( seed )) * 0x9E3779B1;
      n = (((( z ^ ( z >> 8 )) >> 16 ) ^ z ) ^ ( z >> 8 )) & 0xFF;
      seed *= 0x00010DCD;
   }
   return seed;
}

unsigned int ChallengeResponseV10::authFibonacci(unsigned int challenge, int divisor, int outer_loop, int inner_loop)
{
	unsigned int	hash = (challenge & 0xff) * 0x9e3779b1;

	hash ^= (challenge & 0xff00) >> 0x8;
	hash *= 0x9e3779b1;
	hash ^= (challenge & 0xff0000) >> 0x10;
	hash *= 0x9e3779b1;
	hash ^= (challenge & 0xff000000) >> 0x18;
	hash *= 0x9e3779b1;
		
	if (outer_loop > 1) {
		auth_function_t		*ft;

		int					remainder;
				
		hash = ((((hash ^ (hash >> 0x8)) >> 0x10) ^ hash) ^ (hash >> 0x8)) & 0xff;

		remainder = hash % divisor;

		outer_loop--;
		challenge *= 0x10dcd;

		ft = &main_function_list[inner_loop][remainder];

		if (ft) {

			switch (ft->type) {

				case 0:
					return challenge;
				case 1:
					return authType1(challenge, divisor, outer_loop, inner_loop, ft->var1);

				case 2:
					return authType2(challenge, divisor, outer_loop, inner_loop, ft->var1, ft->var2);

				case 3:
					return authType3(challenge, divisor, outer_loop, inner_loop, ft->var1);

				case 4:
				case 5:
					return authType45(challenge, divisor, outer_loop, inner_loop, ft->var1);

				default:
					break;
			}
		}
	}

	return challenge;
}

unsigned char ChallengeResponseV10::authRead45(unsigned int buffer, int offset)
{
	int i;

	if (offset > 32)
		return 0;

	for (i = 0; i < NUM_TYPE_FOURS; i++) {
		if (type_four_list[i].buffer_start == buffer)
			return type_four_list[i].buffer[offset] ^ (buffer & 0xff);
	}

	for (i = 0; i < NUM_TYPE_FIVES; i++) {
		if (type_five_list[i].buffer_start == buffer)
			return type_five_list[i].buffer[offset] ^ (buffer & 0xff);
	}

	return 0;
}

unsigned char ChallengeResponseV10::authRead3(unsigned int buffer, int offset)
{
	int		i;

	if (offset > 256)
		return 0;

	for (i = 0; i < NUM_TYPE_THREES; i++) {
		if (type_three_list[i].buffer_start == buffer)
			return type_three_list[i].buffer[offset] ^ (buffer & 0xff);
	}

	return 0;
}

unsigned int ChallengeResponseV10::authType45(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int initial)
{
	unsigned int	final_value = 0;

	int				i;

	/* Run through each bit.
	 */

	for (i = 0; i < 32; i++)
	{
		unsigned char	buffer = authRead45(initial, i);		/* Find the location in the challenge to put the 1/0 bit */
		int				mask = ~(1 << buffer);						/* so that we can do a replace of our current value. */
		int				new_value = (challenge >> i) & 1;			/* Is this bit 1 or 0? */

		final_value = (final_value & mask) | (new_value << buffer);
	}

	return authFibonacci(final_value, divisor, outer_loop, inner_loop);
}

unsigned int ChallengeResponseV10::authType3(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int offset)
{
	int new_challenge = authRead3(offset, (challenge & 0xff000000) >> 0x18) << 0x18;

	new_challenge |= authRead3(offset, (challenge & 0x00ff0000) >> 0x10) << 0x10;
	new_challenge |= authRead3(offset, (challenge & 0x0000ff00) >> 0x8) << 0x8;
	new_challenge |= authRead3(offset, (challenge & 0x000000ff));
	
	return authFibonacci(new_challenge, divisor, outer_loop, inner_loop);
}

unsigned int ChallengeResponseV10::authType2(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int type_two_variable, int type_two_variable2)
{
	return authFibonacci((challenge * type_two_variable) + type_two_variable2, divisor, outer_loop, inner_loop);
}

unsigned int ChallengeResponseV10::authType1(unsigned int challenge, int divisor, int outer_loop, int inner_loop, int type_one_variable)
{
	return authFibonacci(challenge ^ type_one_variable, divisor, outer_loop, inner_loop);
}

int ChallengeResponseV10::authFinalCountdown(unsigned int challenge, int divisor, int inner_loop, int outer_loop)
{
	auth_function_t		*ft;
	
	int remainder = challenge % divisor;

	ft = &main_function_list[inner_loop][remainder];

	if (ft) {

		switch(ft->type) {

			case 0:
				break;

			case 1:
				challenge = authType1(challenge, divisor, outer_loop, inner_loop, ft->var1);
				break;

			case 2:
				challenge = authType2(challenge, divisor, outer_loop, inner_loop, ft->var1, ft->var2);
				break;

			case 3:
				challenge = authType3(challenge, divisor, outer_loop, inner_loop, ft->var1);
				break;

			case 4:
			case 5:
				challenge = authType45(challenge, divisor, outer_loop, inner_loop, ft->var1);
				break;
		}
	}

	return challenge;
}

