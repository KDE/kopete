#ifndef OSCARDEBUG_H
#define OSCARDEBUG_H

/*
 * Define this if you want to get tons of packets printed out
 */
//#define OSCAR_PACKETLOG 1

/* Define this if you want to get yoiur password in both plaintext and encrypted form
 * printed out
 * !!! security issue in case you don't remove your logfile afterwards !!!
 */
//#define OSCAR_PWDEBUG 1

/*
 * Define this if you want to debug the Buffer class
 * (It's the base of all incoming/outgoing packet handling)
 * Beware, this one REALLY REALLY splits out much-o-debug :)
 */
//#define BUFFER_DEBUG 1

#endif
