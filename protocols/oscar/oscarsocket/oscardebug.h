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

/*
 * Define this if you want to debug encoding/decoding of strings
 * Handy if you wonder why your cyrillic only consists of questionmarks ;)
 */
//#define CHARSET_DEBUG 1

/*
 * just spits out parsed capabilities from userinfo blocks
 * Even end users trying to debug things don't want this one :)
 */
// #define OSCAR_CAP_DEBUG 1

//#define OSCAR_SSI_DEBUG 1
#endif
