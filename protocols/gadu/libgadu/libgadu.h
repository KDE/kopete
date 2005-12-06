/* $Id$ */

/*
 *  (C) Copyright 2001-2003 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Robert J. Woźny <speedy@ziew.org>
 *                          Arkadiusz Miśkiewicz <arekm@pld-linux.org>
 *                          Tomasz Chiliński <chilek@chilan.com>
 *                          Piotr Wysocki <wysek@linux.bydg.org>
 *                          Dawid Jarosz <dawjar@poczta.onet.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License Version
 *  2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#ifndef __GG_LIBGADU_H
#define __GG_LIBGADU_H

#ifdef __cplusplus
#ifdef _WIN32
#pragma pack(push, 1)
#endif
extern "C" {
#endif

#include <libgadu-config.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __GG_LIBGADU_HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

/*
 * typedef uin_t
 *
 * typ reprezentujący numer osoby.
 */
typedef uint32_t uin_t;

/*
 * ogólna struktura opisująca różne sesje. przydatna w klientach.
 */
#define gg_common_head(x) \
	int fd;			/* podglądany deskryptor */ \
	int check;		/* sprawdzamy zapis czy odczyt */ \
	int state;		/* aktualny stan maszynki */ \
	int error;		/* kod błędu dla GG_STATE_ERROR */ \
	int type;		/* rodzaj sesji */ \
	int id;			/* identyfikator */ \
	int timeout;		/* sugerowany timeout w sekundach */ \
	int (*callback)(x*); 	/* callback przy zmianach */ \
	void (*destroy)(x*); 	/* funkcja niszczenia */

struct gg_common {
	gg_common_head(struct gg_common)
};

struct gg_image_queue;

/*
 * struct gg_session
 *
 * struktura opisująca daną sesję. tworzona przez gg_login(), zwalniana
 * przez gg_free_session().
 */
struct gg_session {
	gg_common_head(struct gg_session)

	int async;      	/* czy połączenie jest asynchroniczne */
	int pid;        	/* pid procesu resolvera */
	int port;       	/* port, z którym się łączymy */
	int seq;        	/* numer sekwencyjny ostatniej wiadomości */
	int last_pong;  	/* czas otrzymania ostatniego ping/pong */
	int last_event;		/* czas otrzymania ostatniego pakietu */

	struct gg_event *event;	/* zdarzenie po ->callback() */

	uint32_t proxy_addr;	/* adres proxy, keszowany */
	uint16_t proxy_port;	/* port proxy */

	uint32_t hub_addr;	/* adres huba po resolvnięciu */
	uint32_t server_addr;	/* adres serwera, od huba */

	uint32_t client_addr;	/* adres klienta */
	uint16_t client_port;	/* port, na którym klient słucha */

	uint32_t external_addr;	/* adres zewnetrzny klienta */
	uint16_t external_port;	/* port zewnetrzny klienta */
	
	uin_t uin;		/* numerek klienta */
	char *password;		/* i jego hasło. zwalniane automagicznie */
        
	int initial_status;	/* początkowy stan klienta */
	int status;		/* aktualny stan klienta */

	char *recv_buf;		/* bufor na otrzymywane pakiety */
	int recv_done;		/* ile już wczytano do bufora */
	int recv_left;		/* i ile jeszcze trzeba wczytać */

	int protocol_version;	/* wersja używanego protokołu */
	char *client_version;	/* wersja używanego klienta */
	int last_sysmsg;	/* ostatnia wiadomość systemowa */

	char *initial_descr;	/* początkowy opis stanu klienta */

	void *resolver;		/* wskaźnik na informacje resolvera */

	char *header_buf;	/* bufor na początek nagłówka */
	unsigned int header_done;/* ile już mamy */

#ifdef __GG_LIBGADU_HAVE_OPENSSL
	SSL *ssl;		/* sesja TLS */
	SSL_CTX *ssl_ctx;	/* kontekst sesji? */
#else
	void *ssl;		/* zachowujemy ABI */
	void *ssl_ctx;
#endif

	int image_size;		/* maksymalny rozmiar obrazków w KiB */

	char *userlist_reply;	/* fragment odpowiedzi listy kontaktów */

	int userlist_blocks;	/* na ile kawałków podzielono listę kontaktów */

	struct gg_image_queue *images;	/* aktualnie wczytywane obrazki */
};

/*
 * struct gg_http
 * 
 * ogólna struktura opisująca stan wszystkich operacji HTTP. tworzona
 * przez gg_http_connect(), zwalniana przez gg_http_free().
 */
struct gg_http {
	gg_common_head(struct gg_http)

	int async;              /* czy połączenie asynchroniczne */
	int pid;                /* pid procesu resolvera */
	int port;               /* port, z którym się łączymy */

	char *query;            /* bufor zapytania http */
	char *header;           /* bufor nagłówka */
	int header_size;        /* rozmiar wczytanego nagłówka */
	char *body;             /* bufor otrzymanych informacji */
	unsigned int body_size; /* oczekiwana ilość informacji */

	void *data;             /* dane danej operacji http */

	char *user_data;	/* dane użytkownika, nie są zwalniane przez gg_http_free() */

	void *resolver;		/* wskaźnik na informacje resolvera */

	unsigned int body_done;	/* ile już treści odebrano? */
};

#ifdef __GNUC__
#define GG_PACKED __attribute__ ((packed))
#else
#define GG_PACKED
#endif

#define GG_MAX_PATH 276

/*
 * struct gg_file_info
 * 
 * odpowiednik windowsowej struktury WIN32_FIND_DATA niezbędnej przy
 * wysyłaniu plików.
 */
struct gg_file_info {
	uint32_t mode;			/* dwFileAttributes */
	uint32_t ctime[2];		/* ftCreationTime */
	uint32_t atime[2];		/* ftLastAccessTime */
	uint32_t mtime[2];		/* ftLastWriteTime */
	uint32_t size_hi;		/* nFileSizeHigh */
	uint32_t size;			/* nFileSizeLow */
	uint32_t reserved0;		/* dwReserved0 */
	uint32_t reserved1;		/* dwReserved1 */
	unsigned char filename[GG_MAX_PATH - 14];	/* cFileName */
	unsigned char short_filename[14];		/* cAlternateFileName */
} GG_PACKED;

/*
 * struct gg_dcc
 * 
 * struktura opisująca nasłuchujące gniazdo połączeń między klientami.
 * tworzona przez gg_dcc_socket_create(), zwalniana przez gg_dcc_free().
 */
struct gg_dcc {
	gg_common_head(struct gg_dcc)

	struct gg_event *event;	/* opis zdarzenia */

	int active;		/* czy to my się łączymy? */
	int port;		/* port, na którym siedzi */
	uin_t uin;		/* uin klienta */
	uin_t peer_uin;		/* uin drugiej strony */
	int file_fd;		/* deskryptor pliku */
	unsigned int offset;	/* offset w pliku */
	unsigned int chunk_size;/* rozmiar kawałka */
	unsigned int chunk_offset;/* offset w aktualnym kawałku */
	struct gg_file_info file_info;
				/* informacje o pliku */
	int established;	/* połączenie ustanowione */
	char *voice_buf;	/* bufor na pakiet połączenia głosowego */
	int incoming;		/* połączenie przychodzące */
	char *chunk_buf;	/* bufor na kawałek danych */
	uint32_t remote_addr;	/* adres drugiej strony */
	uint16_t remote_port;	/* port drugiej strony */
};

/*
 * enum gg_session_t
 *
 * rodzaje sesji.
 */
enum gg_session_t {
	GG_SESSION_GG = 1,	/* połączenie z serwerem gg */
	GG_SESSION_HTTP,	/* ogólna sesja http */
	GG_SESSION_SEARCH,	/* szukanie */
	GG_SESSION_REGISTER,	/* rejestrowanie */
	GG_SESSION_REMIND,	/* przypominanie hasła */
	GG_SESSION_PASSWD,	/* zmiana hasła */
	GG_SESSION_CHANGE,	/* zmiana informacji o sobie */
	GG_SESSION_DCC,		/* ogólne połączenie DCC */
	GG_SESSION_DCC_SOCKET,	/* nasłuchujący socket */
	GG_SESSION_DCC_SEND,	/* wysyłanie pliku */
	GG_SESSION_DCC_GET,	/* odbieranie pliku */
	GG_SESSION_DCC_VOICE,	/* rozmowa głosowa */
	GG_SESSION_USERLIST_GET,	/* pobieranie userlisty */
	GG_SESSION_USERLIST_PUT,	/* wysyłanie userlisty */
	GG_SESSION_UNREGISTER,	/* usuwanie konta */
	GG_SESSION_USERLIST_REMOVE,	/* usuwanie userlisty */
	GG_SESSION_TOKEN,	/* pobieranie tokenu */
	
	GG_SESSION_USER0 = 256,	/* zdefiniowana dla użytkownika */
	GG_SESSION_USER1,	/* j.w. */
	GG_SESSION_USER2,	/* j.w. */
	GG_SESSION_USER3,	/* j.w. */
	GG_SESSION_USER4,	/* j.w. */
	GG_SESSION_USER5,	/* j.w. */
	GG_SESSION_USER6,	/* j.w. */
	GG_SESSION_USER7	/* j.w. */
};

/*
 * enum gg_state_t
 *
 * opisuje stan asynchronicznej maszyny.
 */
enum gg_state_t {
		/* wspólne */
	GG_STATE_IDLE = 0,		/* nie powinno wystąpić. */
	GG_STATE_RESOLVING,             /* wywołał gethostbyname() */
	GG_STATE_CONNECTING,            /* wywołał connect() */
	GG_STATE_READING_DATA,		/* czeka na dane http */
	GG_STATE_ERROR,			/* wystąpił błąd. kod w x->error */

		/* gg_session */
	GG_STATE_CONNECTING_HUB,	/* wywołał connect() na huba */
	GG_STATE_CONNECTING_GG,         /* wywołał connect() na serwer */
	GG_STATE_READING_KEY,           /* czeka na klucz */
	GG_STATE_READING_REPLY,         /* czeka na odpowiedź */
	GG_STATE_CONNECTED,             /* połączył się */

		/* gg_http */
	GG_STATE_SENDING_QUERY,		/* wysyła zapytanie http */
	GG_STATE_READING_HEADER,	/* czeka na nagłówek http */
	GG_STATE_PARSING,               /* przetwarza dane */
	GG_STATE_DONE,                  /* skończył */

		/* gg_dcc */
	GG_STATE_LISTENING,		/* czeka na połączenia */
	GG_STATE_READING_UIN_1,		/* czeka na uin peera */
	GG_STATE_READING_UIN_2,		/* czeka na swój uin */
	GG_STATE_SENDING_ACK,		/* wysyła potwierdzenie dcc */
	GG_STATE_READING_ACK,		/* czeka na potwierdzenie dcc */
	GG_STATE_READING_REQUEST,	/* czeka na komendę */
	GG_STATE_SENDING_REQUEST,	/* wysyła komendę */
	GG_STATE_SENDING_FILE_INFO,	/* wysyła informacje o pliku */
	GG_STATE_READING_PRE_FILE_INFO,	/* czeka na pakiet przed file_info */
	GG_STATE_READING_FILE_INFO,	/* czeka na informacje o pliku */
	GG_STATE_SENDING_FILE_ACK,	/* wysyła potwierdzenie pliku */
	GG_STATE_READING_FILE_ACK,	/* czeka na potwierdzenie pliku */
	GG_STATE_SENDING_FILE_HEADER,	/* wysyła nagłówek pliku */
	GG_STATE_READING_FILE_HEADER,	/* czeka na nagłówek */
	GG_STATE_GETTING_FILE,		/* odbiera plik */
	GG_STATE_SENDING_FILE,		/* wysyła plik */
	GG_STATE_READING_VOICE_ACK,	/* czeka na potwierdzenie voip */
	GG_STATE_READING_VOICE_HEADER,	/* czeka na rodzaj bloku voip */
	GG_STATE_READING_VOICE_SIZE,	/* czeka na rozmiar bloku voip */
	GG_STATE_READING_VOICE_DATA,	/* czeka na dane voip */
	GG_STATE_SENDING_VOICE_ACK,	/* wysyła potwierdzenie voip */
	GG_STATE_SENDING_VOICE_REQUEST,	/* wysyła żądanie voip */
	GG_STATE_READING_TYPE,		/* czeka na typ połączenia */

	/* nowe. bez sensu jest to API. */
	GG_STATE_TLS_NEGOTIATION	/* negocjuje połączenie TLS */
};

/*
 * enum gg_check_t
 *
 * informuje, co proces klienta powinien sprawdzić na deskryptorze danego
 * połączenia.
 */
enum gg_check_t {
	GG_CHECK_NONE = 0,		/* nic. nie powinno wystąpić */
	GG_CHECK_WRITE = 1,		/* sprawdzamy możliwość zapisu */
	GG_CHECK_READ = 2		/* sprawdzamy możliwość odczytu */
};

/*
 * struct gg_login_params
 *
 * parametry gg_login(). przeniesiono do struktury, żeby uniknąć problemów
 * z ciągłymi zmianami API, gdy dodano coś nowego do protokołu.
 */
struct gg_login_params {
	uin_t uin;			/* numerek */
	char *password;			/* hasło */
	int async;			/* asynchroniczne sockety? */
	int status;			/* początkowy status klienta */
	char *status_descr;		/* opis statusu */
	uint32_t server_addr;		/* adres serwera gg */
	uint16_t server_port;		/* port serwera gg */
	uint32_t client_addr;		/* adres dcc klienta */
	uint16_t client_port;		/* port dcc klienta */
	int protocol_version;		/* wersja protokołu */
	char *client_version;		/* wersja klienta */
	int has_audio;			/* czy ma dźwięk? */
	int last_sysmsg;		/* ostatnia wiadomość systemowa */
	uint32_t external_addr;		/* adres widziany na zewnatrz */
	uint16_t external_port;		/* port widziany na zewnatrz */
	int tls;			/* czy łączymy po TLS? */
	int image_size;			/* maksymalny rozmiar obrazka w KiB */
	int era_omnix;			/* czy udawać klienta era omnix? */

	char dummy[6 * sizeof(int)];	/* miejsce na kolejnych 6 zmiennych,
					 * żeby z dodaniem parametru nie 
					 * zmieniał się rozmiar struktury */
};

struct gg_session *gg_login(const struct gg_login_params *p);
void gg_free_session(struct gg_session *sess);
void gg_logoff(struct gg_session *sess);
int gg_change_status(struct gg_session *sess, int status);
int gg_change_status_descr(struct gg_session *sess, int status, const char *descr);
int gg_change_status_descr_time(struct gg_session *sess, int status, const char *descr, int time);
int gg_send_message(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message);
int gg_send_message_richtext(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message, const unsigned char *format, int formatlen);
int gg_send_message_confer(struct gg_session *sess, int msgclass, int recipients_count, uin_t *recipients, const unsigned char *message);
int gg_send_message_confer_richtext(struct gg_session *sess, int msgclass, int recipients_count, uin_t *recipients, const unsigned char *message, const unsigned char *format, int formatlen);
int gg_send_message_ctcp(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message, int message_len);
int gg_ping(struct gg_session *sess);
int gg_userlist_request(struct gg_session *sess, char type, const char *request);
int gg_image_request(struct gg_session *sess, uin_t recipient, int size, uint32_t crc32);
int gg_image_reply(struct gg_session *sess, uin_t recipient, const char *filename, const char *image, int size);

uint32_t gg_crc32(uint32_t crc, const unsigned char *buf, int len);

struct gg_image_queue {
	uin_t sender;			/* nadawca obrazka */
	uint32_t size;			/* rozmiar */
	uint32_t crc32;			/* suma kontrolna */
	char *filename;			/* nazwa pliku */
	char *image;			/* bufor z obrazem */
	uint32_t done;			/* ile już wczytano */

	struct gg_image_queue *next;	/* następny na liście */
};

/*
 * enum gg_event_t
 *
 * rodzaje zdarzeń.
 */
enum gg_event_t {
	GG_EVENT_NONE = 0,		/* nic się nie wydarzyło */
	GG_EVENT_MSG,			/* otrzymano wiadomość */
	GG_EVENT_NOTIFY,		/* ktoś się pojawił */
	GG_EVENT_NOTIFY_DESCR,		/* ktoś się pojawił z opisem */
	GG_EVENT_STATUS,		/* ktoś zmienił stan */
	GG_EVENT_ACK,			/* potwierdzenie wysłania wiadomości */
	GG_EVENT_PONG,			/* pakiet pong */
	GG_EVENT_CONN_FAILED,		/* połączenie się nie udało */
	GG_EVENT_CONN_SUCCESS,		/* połączenie się powiodło */
	GG_EVENT_DISCONNECT,		/* serwer zrywa połączenie */

	GG_EVENT_DCC_NEW,		/* nowe połączenie między klientami */
	GG_EVENT_DCC_ERROR,		/* błąd połączenia między klientami */
	GG_EVENT_DCC_DONE,		/* zakończono połączenie */
	GG_EVENT_DCC_CLIENT_ACCEPT,	/* moment akceptacji klienta */
	GG_EVENT_DCC_CALLBACK,		/* klient się połączył na żądanie */
	GG_EVENT_DCC_NEED_FILE_INFO,	/* należy wypełnić file_info */
	GG_EVENT_DCC_NEED_FILE_ACK,	/* czeka na potwierdzenie pliku */
	GG_EVENT_DCC_NEED_VOICE_ACK,	/* czeka na potwierdzenie rozmowy */
	GG_EVENT_DCC_VOICE_DATA, 	/* ramka danych rozmowy głosowej */

	GG_EVENT_PUBDIR50_SEARCH_REPLY,	/* odpowiedz wyszukiwania */
	GG_EVENT_PUBDIR50_READ,		/* odczytano własne dane z katalogu */
	GG_EVENT_PUBDIR50_WRITE,	/* wpisano własne dane do katalogu */

	GG_EVENT_STATUS60,		/* ktoś zmienił stan w GG 6.0 */
	GG_EVENT_NOTIFY60,		/* ktoś się pojawił w GG 6.0 */
	GG_EVENT_USERLIST,		/* odpowiedź listy kontaktów w GG 6.0 */
	GG_EVENT_IMAGE_REQUEST,		/* prośba o wysłanie obrazka GG 6.0 */
	GG_EVENT_IMAGE_REPLY,		/* podesłany obrazek GG 6.0 */
	GG_EVENT_DCC_ACK		/* potwierdzenie transmisji */
};

#define GG_EVENT_SEARCH50_REPLY GG_EVENT_PUBDIR50_SEARCH_REPLY

/*
 * enum gg_failure_t
 *
 * określa powód nieudanego połączenia.
 */
enum gg_failure_t {
	GG_FAILURE_RESOLVING = 1,	/* nie znaleziono serwera */
	GG_FAILURE_CONNECTING,		/* nie można się połączyć */
	GG_FAILURE_INVALID,		/* serwer zwrócił nieprawidłowe dane */
	GG_FAILURE_READING,		/* zerwano połączenie podczas odczytu */
	GG_FAILURE_WRITING,		/* zerwano połączenie podczas zapisu */
	GG_FAILURE_PASSWORD,		/* nieprawidłowe hasło */
	GG_FAILURE_404, 		/* XXX nieużywane */
	GG_FAILURE_TLS,			/* błąd negocjacji TLS */
	GG_FAILURE_NEED_EMAIL 		/* serwer rozłączył nas z prośbą o zmianę emaila */
};

/*
 * enum gg_error_t
 *
 * określa rodzaj błędu wywołanego przez daną operację. nie zawiera
 * przesadnie szczegółowych informacji o powodzie błędu, by nie komplikować
 * obsługi błędów. jeśli wymagana jest większa dokładność, należy sprawdzić
 * zawartość zmiennej errno.
 */
enum gg_error_t {
	GG_ERROR_RESOLVING = 1,		/* błąd znajdowania hosta */
	GG_ERROR_CONNECTING,		/* błąd łaczenia się */
	GG_ERROR_READING,		/* błąd odczytu */
	GG_ERROR_WRITING,		/* błąd wysyłania */

	GG_ERROR_DCC_HANDSHAKE,		/* błąd negocjacji */
	GG_ERROR_DCC_FILE,		/* błąd odczytu/zapisu pliku */
	GG_ERROR_DCC_EOF,		/* plik się skończył? */
	GG_ERROR_DCC_NET,		/* błąd wysyłania/odbierania */
	GG_ERROR_DCC_REFUSED 		/* połączenie odrzucone przez usera */
};

/*
 * struktury dotyczące wyszukiwania w GG 5.0. NIE NALEŻY SIĘ DO NICH
 * ODWOŁYWAĆ BEZPOŚREDNIO! do dostępu do nich służą funkcje gg_pubdir50_*()
 */
struct gg_pubdir50_entry {
	int num;
	char *field;
	char *value;
};

struct gg_pubdir50_s {
	int count;
	uin_t next;
	int type;
	uint32_t seq;
	struct gg_pubdir50_entry *entries;
	int entries_count;
};

/*
 * typedef gg_pubdir_50_t
 *
 * typ opisujący zapytanie lub wynik zapytania katalogu publicznego
 * z protokołu GG 5.0. nie należy się odwoływać bezpośrednio do jego
 * pól -- służą do tego funkcje gg_pubdir50_*()
 */
typedef struct gg_pubdir50_s *gg_pubdir50_t;

/*
 * struct gg_event
 *
 * struktura opisująca rodzaj zdarzenia. wychodzi z gg_watch_fd() lub
 * z gg_dcc_watch_fd()
 */
struct gg_event {
	int type;	/* rodzaj zdarzenia -- gg_event_t */
	union {		/* @event */
		struct gg_notify_reply *notify;	/* informacje o liście kontaktów -- GG_EVENT_NOTIFY */

		enum gg_failure_t failure;	/* błąd połączenia -- GG_EVENT_FAILURE */

		struct gg_dcc *dcc_new;		/* nowe połączenie bezpośrednie -- GG_EVENT_DCC_NEW */
		
		int dcc_error;			/* błąd połączenia bezpośredniego -- GG_EVENT_DCC_ERROR */

		gg_pubdir50_t pubdir50;		/* wynik operacji związanej z katalogiem publicznym -- GG_EVENT_PUBDIR50_* */
	
		struct {			/* @msg odebrano wiadomość -- GG_EVENT_MSG */
			uin_t sender;		/* numer nadawcy */
			int msgclass;		/* klasa wiadomości */
			time_t time;		/* czas nadania */
			unsigned char *message;	/* treść wiadomości */

			int recipients_count;	/* ilość odbiorców konferencji */
			uin_t *recipients;	/* odbiorcy konferencji */
			
			int formats_length;	/* długość informacji o formatowaniu tekstu */
			void *formats;		/* informacje o formatowaniu tekstu */
		} msg;
		
		struct {			/* @notify_descr informacje o liście kontaktów z opisami stanu -- GG_EVENT_NOTIFY_DESCR */
			struct gg_notify_reply *notify;	/* informacje o liście kontaktów */
			char *descr;		/* opis stanu */
		} notify_descr;
		
		struct {			/* @status zmiana stanu -- GG_EVENT_STATUS */
			uin_t uin;		/* numer */
			uint32_t status;	/* nowy stan */
			char *descr;		/* opis stanu */
		} status;

		struct {			/* @status60 zmiana stanu -- GG_EVENT_STATUS60 */
			uin_t uin;		/* numer */
			int status;	/* nowy stan */
			uint32_t remote_ip;	/* adres ip */
			uint16_t remote_port;	/* port */
			int version;	/* wersja klienta */
			int image_size;	/* maksymalny rozmiar grafiki w KiB */
			char *descr;		/* opis stanu */
			time_t time;		/* czas powrotu */
		} status60;

		struct {			/* @notify60 informacja o liście kontaktów -- GG_EVENT_NOTIFY60 */
			uin_t uin;		/* numer */
			int status;	/* stan */
			uint32_t remote_ip;	/* adres ip */
			uint16_t remote_port;	/* port */
			int version;	/* wersja klienta */
			int image_size;	/* maksymalny rozmiar grafiki w KiB */
			char *descr;		/* opis stanu */
			time_t time;		/* czas powrotu */
		} *notify60;
		
		struct {			/* @ack potwierdzenie wiadomości -- GG_EVENT_ACK */
			uin_t recipient;	/* numer odbiorcy */
			int status;		/* stan doręczenia wiadomości */
			int seq;		/* numer sekwencyjny wiadomości */
		} ack;

		struct {			/* @dcc_voice_data otrzymano dane dźwiękowe -- GG_EVENT_DCC_VOICE_DATA */
			uint8_t *data;		/* dane dźwiękowe */
			int length;		/* ilość danych dźwiękowych */
		} dcc_voice_data;

		struct {			/* @userlist odpowiedź listy kontaktów serwera */
			char type;		/* rodzaj odpowiedzi */
			char *reply;		/* treść odpowiedzi */
		} userlist;

		struct {			/* @image_request prośba o obrazek */
			uin_t sender;		/* nadawca prośby */
			uint32_t size;		/* rozmiar obrazka */
			uint32_t crc32;		/* suma kontrolna */
		} image_request;

		struct {			/* @image_reply odpowiedź z obrazkiem */
			uin_t sender;		/* nadawca odpowiedzi */
			uint32_t size;		/* rozmiar obrazka */
			uint32_t crc32;		/* suma kontrolna */
			char *filename;		/* nazwa pliku */
			char *image;		/* bufor z obrazkiem */
		} image_reply;
	} event;
};

struct gg_event *gg_watch_fd(struct gg_session *sess);
void gg_event_free(struct gg_event *e);
#define gg_free_event gg_event_free

/*
 * funkcje obsługi listy kontaktów.
 */
int gg_notify_ex(struct gg_session *sess, uin_t *userlist, char *types, int count);
int gg_notify(struct gg_session *sess, uin_t *userlist, int count);
int gg_add_notify_ex(struct gg_session *sess, uin_t uin, char type);
int gg_add_notify(struct gg_session *sess, uin_t uin);
int gg_remove_notify_ex(struct gg_session *sess, uin_t uin, char type);
int gg_remove_notify(struct gg_session *sess, uin_t uin);

/*
 * funkcje obsługi http.
 */
struct gg_http *gg_http_connect(const char *hostname, int port, int async, const char *method, const char *path, const char *header);
int gg_http_watch_fd(struct gg_http *h);
void gg_http_stop(struct gg_http *h);
void gg_http_free(struct gg_http *h);
void gg_http_free_fields(struct gg_http *h);
#define gg_free_http gg_http_free

/*
 * struktury opisująca kryteria wyszukiwania dla gg_search(). nieaktualne,
 * zastąpione przez gg_pubdir50_t. pozostawiono je dla zachowania ABI.
 */
struct gg_search_request {
	int active;
	unsigned int start;
	char *nickname;
	char *first_name;
	char *last_name;
	char *city;
	int gender;
	int min_birth;
	int max_birth;
	char *email;
	char *phone;
	uin_t uin;
};

struct gg_search {
	int count;
	struct gg_search_result *results;
};

struct gg_search_result {
	uin_t uin;
	char *first_name;
	char *last_name;
	char *nickname;
	int born;
	int gender;
	char *city;
	int active;
};

#define GG_GENDER_NONE 0
#define GG_GENDER_FEMALE 1
#define GG_GENDER_MALE 2

/*
 * funkcje wyszukiwania.
 */
struct gg_http *gg_search(const struct gg_search_request *r, int async);
int gg_search_watch_fd(struct gg_http *f);
void gg_free_search(struct gg_http *f);
#define gg_search_free gg_free_search

const struct gg_search_request *gg_search_request_mode_0(char *nickname, char *first_name, char *last_name, char *city, int gender, int min_birth, int max_birth, int active, int start);
const struct gg_search_request *gg_search_request_mode_1(char *email, int active, int start);
const struct gg_search_request *gg_search_request_mode_2(char *phone, int active, int start);
const struct gg_search_request *gg_search_request_mode_3(uin_t uin, int active, int start);
void gg_search_request_free(struct gg_search_request *r);

/*
 * funkcje obsługi katalogu publicznego zgodne z GG 5.0. tym razem funkcje
 * zachowują pewien poziom abstrakcji, żeby uniknąć zmian ABI przy zmianach
 * w protokole.
 *
 * NIE NALEŻY SIĘ ODWOŁYWAĆ DO PÓL gg_pubdir50_t BEZPOŚREDNIO!
 */
uint32_t gg_pubdir50(struct gg_session *sess, gg_pubdir50_t req);
gg_pubdir50_t gg_pubdir50_new(int type);
int gg_pubdir50_add(gg_pubdir50_t req, const char *field, const char *value);
int gg_pubdir50_seq_set(gg_pubdir50_t req, uint32_t seq);
const char *gg_pubdir50_get(gg_pubdir50_t res, int num, const char *field);
int gg_pubdir50_type(gg_pubdir50_t res);
int gg_pubdir50_count(gg_pubdir50_t res);
uin_t gg_pubdir50_next(gg_pubdir50_t res);
uint32_t gg_pubdir50_seq(gg_pubdir50_t res);
void gg_pubdir50_free(gg_pubdir50_t res);

#define GG_PUBDIR50_UIN "FmNumber"
#define GG_PUBDIR50_STATUS "FmStatus"
#define GG_PUBDIR50_FIRSTNAME "firstname"
#define GG_PUBDIR50_LASTNAME "lastname"
#define GG_PUBDIR50_NICKNAME "nickname"
#define GG_PUBDIR50_BIRTHYEAR "birthyear"
#define GG_PUBDIR50_CITY "city"
#define GG_PUBDIR50_GENDER "gender"
#define GG_PUBDIR50_GENDER_FEMALE "1"
#define GG_PUBDIR50_GENDER_MALE "2"
#define GG_PUBDIR50_GENDER_SET_FEMALE "2"
#define GG_PUBDIR50_GENDER_SET_MALE "1"
#define GG_PUBDIR50_ACTIVE "ActiveOnly"
#define GG_PUBDIR50_ACTIVE_TRUE "1"
#define GG_PUBDIR50_START "fmstart"
#define GG_PUBDIR50_FAMILYNAME "familyname"
#define GG_PUBDIR50_FAMILYCITY "familycity"

int gg_pubdir50_handle_reply(struct gg_event *e, const char *packet, int length);

/*
 * struct gg_pubdir
 *
 * operacje na katalogu publicznym.
 */
struct gg_pubdir {
	int success;		/* czy się udało */
	uin_t uin;		/* otrzymany numerek. 0 jeśli błąd */
};

/* ogólne funkcje, nie powinny być używane */
int gg_pubdir_watch_fd(struct gg_http *f);
void gg_pubdir_free(struct gg_http *f);
#define gg_free_pubdir gg_pubdir_free

struct gg_token {
	int width;		/* szerokość obrazka */
	int height;		/* wysokość obrazka */
	int length;		/* ilość znaków w tokenie */
	char *tokenid;		/* id tokenu */
};

/* funkcje dotyczące tokenów */
struct gg_http *gg_token(int async);
int gg_token_watch_fd(struct gg_http *h);
void gg_token_free(struct gg_http *h);

/* rejestracja nowego numerka */
struct gg_http *gg_register(const char *email, const char *password, int async);
struct gg_http *gg_register2(const char *email, const char *password, const char *qa, int async);
struct gg_http *gg_register3(const char *email, const char *password, const char *tokenid, const char *tokenval, int async);
#define gg_register_watch_fd gg_pubdir_watch_fd
#define gg_register_free gg_pubdir_free
#define gg_free_register gg_pubdir_free

struct gg_http *gg_unregister(uin_t uin, const char *password, const char *email, int async);
struct gg_http *gg_unregister2(uin_t uin, const char *password, const char *qa, int async);
struct gg_http *gg_unregister3(uin_t uin, const char *password, const char *tokenid, const char *tokenval, int async);
#define gg_unregister_watch_fd gg_pubdir_watch_fd
#define gg_unregister_free gg_pubdir_free

/* przypomnienie hasła e-mailem */
struct gg_http *gg_remind_passwd(uin_t uin, int async);
struct gg_http *gg_remind_passwd2(uin_t uin, const char *tokenid, const char *tokenval, int async);
struct gg_http *gg_remind_passwd3(uin_t uin, const char *email, const char *tokenid, const char *tokenval, int async);
#define gg_remind_passwd_watch_fd gg_pubdir_watch_fd
#define gg_remind_passwd_free gg_pubdir_free
#define gg_free_remind_passwd gg_pubdir_free

/* zmiana hasła */
struct gg_http *gg_change_passwd(uin_t uin, const char *passwd, const char *newpasswd, const char *newemail, int async);
struct gg_http *gg_change_passwd2(uin_t uin, const char *passwd, const char *newpasswd, const char *email, const char *newemail, int async);
struct gg_http *gg_change_passwd3(uin_t uin, const char *passwd, const char *newpasswd, const char *qa, int async);
struct gg_http *gg_change_passwd4(uin_t uin, const char *email, const char *passwd, const char *newpasswd, const char *tokenid, const char *tokenval, int async);
#define gg_change_passwd_free gg_pubdir_free
#define gg_free_change_passwd gg_pubdir_free

/*
 * struct gg_change_info_request
 * 
 * opis żądania zmiany informacji w katalogu publicznym.
 */
struct gg_change_info_request {
	char *first_name;	/* imię */
	char *last_name;	/* nazwisko */
	char *nickname;		/* pseudonim */
	char *email;		/* email */
	int born;		/* rok urodzenia */
	int gender;		/* płeć */
	char *city;		/* miasto */
};

struct gg_change_info_request *gg_change_info_request_new(const char *first_name, const char *last_name, const char *nickname, const char *email, int born, int gender, const char *city);
void gg_change_info_request_free(struct gg_change_info_request *r);

struct gg_http *gg_change_info(uin_t uin, const char *passwd, const struct gg_change_info_request *request, int async);
#define gg_change_pubdir_watch_fd gg_pubdir_watch_fd
#define gg_change_pubdir_free gg_pubdir_free
#define gg_free_change_pubdir gg_pubdir_free

/*
 * funkcje dotyczące listy kontaktów na serwerze.
 */
struct gg_http *gg_userlist_get(uin_t uin, const char *password, int async);
int gg_userlist_get_watch_fd(struct gg_http *f);
void gg_userlist_get_free(struct gg_http *f);

struct gg_http *gg_userlist_put(uin_t uin, const char *password, const char *contacts, int async);
int gg_userlist_put_watch_fd(struct gg_http *f);
void gg_userlist_put_free(struct gg_http *f);

struct gg_http *gg_userlist_remove(uin_t uin, const char *password, int async);
int gg_userlist_remove_watch_fd(struct gg_http *f);
void gg_userlist_remove_free(struct gg_http *f);



/*
 * funkcje dotyczące komunikacji między klientami.
 */
extern int gg_dcc_port;			/* port, na którym nasłuchuje klient */
extern unsigned long gg_dcc_ip;		/* adres, na którym nasłuchuje klient */

int gg_dcc_request(struct gg_session *sess, uin_t uin);

struct gg_dcc *gg_dcc_send_file(uint32_t ip, uint16_t port, uin_t my_uin, uin_t peer_uin);
struct gg_dcc *gg_dcc_get_file(uint32_t ip, uint16_t port, uin_t my_uin, uin_t peer_uin);
struct gg_dcc *gg_dcc_voice_chat(uint32_t ip, uint16_t port, uin_t my_uin, uin_t peer_uin);
void gg_dcc_set_type(struct gg_dcc *d, int type);
int gg_dcc_fill_file_info(struct gg_dcc *d, const char *filename);
int gg_dcc_fill_file_info2(struct gg_dcc *d, const char *filename, const char *local_filename);
int gg_dcc_voice_send(struct gg_dcc *d, char *buf, int length);

#define GG_DCC_VOICE_FRAME_LENGTH 195
#define GG_DCC_VOICE_FRAME_LENGTH_505 326

struct gg_dcc *gg_dcc_socket_create(uin_t uin, uint16_t port);
#define gg_dcc_socket_free gg_free_dcc
#define gg_dcc_socket_watch_fd gg_dcc_watch_fd

struct gg_event *gg_dcc_watch_fd(struct gg_dcc *d);

void gg_dcc_free(struct gg_dcc *c);
#define gg_free_dcc gg_dcc_free

/*
 * jeśli chcemy sobie podebugować, wystarczy ustawić `gg_debug_level'.
 * niestety w miarę przybywania wpisów `gg_debug(...)' nie chciało mi
 * się ustawiać odpowiednich leveli, więc większość szła do _MISC.
 */
extern int gg_debug_level;	/* poziom debugowania. mapa bitowa stałych GG_DEBUG_* */

/*
 * można podać wskaźnik do funkcji obsługującej wywołania gg_debug().
 * nieoficjalne, nieudokumentowane, może się zmienić. jeśli ktoś jest 
 * zainteresowany, niech da znać na ekg-devel.
 */
extern void (*gg_debug_handler)(int level, const char *format, va_list ap);

/*
 * można podać plik, do którego będą zapisywane teksty z gg_debug().
 */
extern FILE *gg_debug_file;

#define GG_DEBUG_NET 1
#define GG_DEBUG_TRAFFIC 2
#define GG_DEBUG_DUMP 4
#define GG_DEBUG_FUNCTION 8
#define GG_DEBUG_MISC 16

#ifdef GG_DEBUG_DISABLE
#define gg_debug(x, y...) do { } while(0)
#else
void gg_debug(int level, const char *format, ...);
#endif

const char *gg_libgadu_version(void);

/*
 * konfiguracja http proxy.
 */
extern int gg_proxy_enabled;		/* włącza obsługę proxy */
extern char *gg_proxy_host;		/* określa adres serwera proxy */
extern int gg_proxy_port;		/* określa port serwera proxy */
extern char *gg_proxy_username;		/* określa nazwę użytkownika przy autoryzacji serwera proxy */
extern char *gg_proxy_password;		/* określa hasło użytkownika przy autoryzacji serwera proxy */
extern int gg_proxy_http_only;		/* włącza obsługę proxy wyłącznie dla usług HTTP */


/* 
 * adres, z którego ślemy pakiety (np łączymy się z serwerem)
 * używany przy gg_connect()
 */
extern unsigned long gg_local_ip; 
/*
 * -------------------------------------------------------------------------
 * poniżej znajdują się wewnętrzne sprawy biblioteki. zwykły klient nie
 * powinien ich w ogóle ruszać, bo i nie ma po co. wszystko można załatwić
 * procedurami wyższego poziomu, których definicje znajdują się na początku
 * tego pliku.
 * -------------------------------------------------------------------------
 */

#ifdef __GG_LIBGADU_HAVE_PTHREAD
int gg_resolve_pthread(int *fd, void **resolver, const char *hostname);
#endif

#ifdef _WIN32
int gg_thread_socket(int thread_id, int socket);
#endif

int gg_resolve(int *fd, int *pid, const char *hostname);

#ifdef __GNUC__
char *gg_saprintf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
#else
char *gg_saprintf(const char *format, ...);
#endif

char *gg_vsaprintf(const char *format, va_list ap);

#define gg_alloc_sprintf gg_saprintf

char *gg_get_line(char **ptr);

int gg_connect(void *addr, int port, int async);
struct in_addr *gg_gethostbyname(const char *hostname);
char *gg_read_line(int sock, char *buf, int length);
void gg_chomp(char *line);
char *gg_urlencode(const char *str);
int gg_http_hash(const char *format, ...);
int gg_read(struct gg_session *sess, char *buf, int length);
int gg_write(struct gg_session *sess, const char *buf, int length);
void *gg_recv_packet(struct gg_session *sess);
int gg_send_packet(struct gg_session *sess, int type, ...);
unsigned int gg_login_hash(const unsigned char *password, unsigned int seed);
uint32_t gg_fix32(uint32_t x);
uint16_t gg_fix16(uint16_t x);
#define fix16 gg_fix16
#define fix32 gg_fix32
char *gg_proxy_auth(void);
char *gg_base64_encode(const char *buf);
char *gg_base64_decode(const char *buf);
int gg_image_queue_remove(struct gg_session *s, struct gg_image_queue *q, int freeq);

#define GG_APPMSG_HOST "appmsg.gadu-gadu.pl"
#define GG_APPMSG_PORT 80
#define GG_PUBDIR_HOST "pubdir.gadu-gadu.pl"
#define GG_PUBDIR_PORT 80
#define GG_REGISTER_HOST "register.gadu-gadu.pl"
#define GG_REGISTER_PORT 80
#define GG_REMIND_HOST "retr.gadu-gadu.pl"
#define GG_REMIND_PORT 80

#define GG_DEFAULT_PORT 8074
#define GG_HTTPS_PORT 443
#define GG_HTTP_USERAGENT "Mozilla/4.7 [en] (Win98; I)"

#define GG_DEFAULT_CLIENT_VERSION "6, 1, 0, 158"
#define GG_DEFAULT_PROTOCOL_VERSION 0x24
#define GG_DEFAULT_TIMEOUT 30
#define GG_HAS_AUDIO_MASK 0x40000000
#define GG_ERA_OMNIX_MASK 0x04000000
#define GG_LIBGADU_VERSION "CVS"

#define GG_DEFAULT_DCC_PORT 1550

struct gg_header {
	uint32_t type;			/* typ pakietu */
	uint32_t length;		/* długość reszty pakietu */
} GG_PACKED;

#define GG_WELCOME 0x0001
#define GG_NEED_EMAIL 0x0014

struct gg_welcome {
	uint32_t key;			/* klucz szyfrowania hasła */
} GG_PACKED;
	
#define GG_LOGIN 0x000c

struct gg_login {
	uint32_t uin;			/* mój numerek */
	uint32_t hash;			/* hash hasła */
	uint32_t status;		/* status na dzień dobry */
	uint32_t version;		/* moja wersja klienta */
	uint32_t local_ip;		/* mój adres ip */
	uint16_t local_port;		/* port, na którym słucham */
} GG_PACKED;

#define GG_LOGIN_EXT 0x0013

struct gg_login_ext {
	uint32_t uin;			/* mój numerek */
	uint32_t hash;			/* hash hasła */
	uint32_t status;		/* status na dzień dobry */
	uint32_t version;		/* moja wersja klienta */
	uint32_t local_ip;		/* mój adres ip */
	uint16_t local_port;		/* port, na którym słucham */
	uint32_t external_ip;		/* zewnętrzny adres ip */
	uint16_t external_port;		/* zewnętrzny port */
} GG_PACKED;

#define GG_LOGIN60 0x0015

struct gg_login60 {
	uint32_t uin;			/* mój numerek */
	uint32_t hash;			/* hash hasła */
	uint32_t status;		/* status na dzień dobry */
	uint32_t version;		/* moja wersja klienta */
	uint8_t dunno1;			/* 0x00 */
	uint32_t local_ip;		/* mój adres ip */
	uint16_t local_port;		/* port, na którym słucham */
	uint32_t external_ip;		/* zewnętrzny adres ip */
	uint16_t external_port;		/* zewnętrzny port */
	uint8_t image_size;		/* maksymalny rozmiar grafiki w KiB */
	uint8_t dunno2;			/* 0xbe */
} GG_PACKED;

#define GG_LOGIN_OK 0x0003

#define GG_LOGIN_FAILED 0x0009

#define GG_PUBDIR50_REQUEST 0x0014

#define GG_PUBDIR50_WRITE 0x01
#define GG_PUBDIR50_READ 0x02
#define GG_PUBDIR50_SEARCH 0x03
#define GG_PUBDIR50_SEARCH_REQUEST GG_PUBDIR50_SEARCH
#define GG_PUBDIR50_SEARCH_REPLY 0x05

struct gg_pubdir50_request {
	uint8_t type;			/* GG_PUBDIR50_* */
	uint32_t seq;			/* czas wysłania zapytania */
} GG_PACKED;

#define GG_PUBDIR50_REPLY 0x000e

struct gg_pubdir50_reply {
	uint8_t type;			/* GG_PUBDIR50_* */
	uint32_t seq;			/* czas wysłania zapytania */
} GG_PACKED;

#define GG_NEW_STATUS 0x0002

#define GG_STATUS_NOT_AVAIL 0x0001		/* niedostępny */
#define GG_STATUS_NOT_AVAIL_DESCR 0x0015	/* niedostępny z opisem (4.8) */
#define GG_STATUS_AVAIL 0x0002			/* dostępny */
#define GG_STATUS_AVAIL_DESCR 0x0004		/* dostępny z opisem (4.9) */
#define GG_STATUS_BUSY 0x0003			/* zajęty */
#define GG_STATUS_BUSY_DESCR 0x0005		/* zajęty z opisem (4.8) */
#define GG_STATUS_INVISIBLE 0x0014		/* niewidoczny (4.6) */
#define GG_STATUS_INVISIBLE_DESCR 0x0016	/* niewidoczny z opisem (4.9) */
#define GG_STATUS_BLOCKED 0x0006		/* zablokowany */

#define GG_STATUS_FRIENDS_MASK 0x8000		/* tylko dla znajomych (4.6) */

#define GG_STATUS_DESCR_MAXSIZE 70

/*
 * makra do łatwego i szybkiego sprawdzania stanu.
 */

/* GG_S_F() tryb tylko dla znajomych */
#define GG_S_F(x) (((x) & GG_STATUS_FRIENDS_MASK) != 0)

/* GG_S() stan bez uwzględnienia trybu tylko dla znajomych */
#define GG_S(x) ((x) & ~GG_STATUS_FRIENDS_MASK)

/* GG_S_A() dostępny */
#define GG_S_A(x) (GG_S(x) == GG_STATUS_AVAIL || GG_S(x) == GG_STATUS_AVAIL_DESCR)

/* GG_S_NA() niedostępny */
#define GG_S_NA(x) (GG_S(x) == GG_STATUS_NOT_AVAIL || GG_S(x) == GG_STATUS_NOT_AVAIL_DESCR)

/* GG_S_B() zajęty */
#define GG_S_B(x) (GG_S(x) == GG_STATUS_BUSY || GG_S(x) == GG_STATUS_BUSY_DESCR)

/* GG_S_I() niewidoczny */
#define GG_S_I(x) (GG_S(x) == GG_STATUS_INVISIBLE || GG_S(x) == GG_STATUS_INVISIBLE_DESCR)

/* GG_S_D() stan opisowy */
#define GG_S_D(x) (GG_S(x) == GG_STATUS_NOT_AVAIL_DESCR || GG_S(x) == GG_STATUS_AVAIL_DESCR || GG_S(x) == GG_STATUS_BUSY_DESCR || GG_S(x) == GG_STATUS_INVISIBLE_DESCR)

/* GG_S_BL() blokowany lub blokujący */
#define GG_S_BL(x) (GG_S(x) == GG_STATUS_BLOCKED)

struct gg_new_status {
	uint32_t status;			/* na jaki zmienić? */
} GG_PACKED;

#define GG_NOTIFY_FIRST 0x000f
#define GG_NOTIFY_LAST 0x0010

#define GG_NOTIFY 0x0010
	
struct gg_notify {
	uint32_t uin;				/* numerek danej osoby */
	uint8_t dunno1;				/* rodzaj wpisu w liście */
} GG_PACKED;

#define GG_USER_OFFLINE 0x01	/* będziemy niewidoczni dla użytkownika */
#define GG_USER_NORMAL 0x03	/* zwykły użytkownik */
#define GG_USER_BLOCKED 0x04	/* zablokowany użytkownik */

#define GG_LIST_EMPTY 0x0012
	
#define GG_NOTIFY_REPLY 0x000c	/* tak, to samo co GG_LOGIN */
	
struct gg_notify_reply {
	uint32_t uin;			/* numerek */
	uint32_t status;		/* status danej osoby */
	uint32_t remote_ip;		/* adres ip delikwenta */
	uint16_t remote_port;		/* port, na którym słucha klient */
	uint32_t version;		/* wersja klienta */
	uint16_t dunno2;		/* znowu port? */
} GG_PACKED;

#define GG_NOTIFY_REPLY60 0x0011
	
struct gg_notify_reply60 {
	uint32_t uin;			/* numerek plus flagi w MSB */
	uint8_t status;			/* status danej osoby */
	uint32_t remote_ip;		/* adres ip delikwenta */
	uint16_t remote_port;		/* port, na którym słucha klient */
	uint8_t version;		/* wersja klienta */
	uint8_t image_size;		/* maksymalny rozmiar grafiki w KiB */
	uint8_t dunno1;			/* 0x00 */
} GG_PACKED;

#define GG_STATUS60 0x000f
	
struct gg_status60 {
	uint32_t uin;			/* numerek plus flagi w MSB */
	uint8_t status;			/* status danej osoby */
	uint32_t remote_ip;		/* adres ip delikwenta */
	uint16_t remote_port;		/* port, na którym słucha klient */
	uint8_t version;		/* wersja klienta */
	uint8_t image_size;		/* maksymalny rozmiar grafiki w KiB */
	uint8_t dunno1;			/* 0x00 */
} GG_PACKED;

#define GG_ADD_NOTIFY 0x000d
#define GG_REMOVE_NOTIFY 0x000e
	
struct gg_add_remove {
	uint32_t uin;			/* numerek */
	uint8_t dunno1;			/* bitmapa */
} GG_PACKED;

#define GG_STATUS 0x0002

struct gg_status {
	uint32_t uin;			/* numerek */
	uint32_t status;		/* nowy stan */
} GG_PACKED;
	
#define GG_SEND_MSG 0x000b

#define GG_CLASS_QUEUED 0x0001
#define GG_CLASS_OFFLINE GG_CLASS_QUEUED
#define GG_CLASS_MSG 0x0004
#define GG_CLASS_CHAT 0x0008
#define GG_CLASS_CTCP 0x0010
#define GG_CLASS_ACK 0x0020
#define GG_CLASS_EXT GG_CLASS_ACK	/* kompatybilność wstecz */

#define GG_MSG_MAXSIZE 2000

struct gg_send_msg {
	uint32_t recipient;
	uint32_t seq;
	uint32_t msgclass;
} GG_PACKED;

struct gg_msg_richtext {
	uint8_t flag;		
	uint16_t length;	  
} GG_PACKED;

struct gg_msg_richtext_format {
	uint16_t position;
	uint8_t font;	  
} GG_PACKED;

struct gg_msg_richtext_image {
	uint16_t unknown1;
	uint32_t size;
	uint32_t crc32;
} GG_PACKED;

#define GG_FONT_BOLD 0x01
#define GG_FONT_ITALIC 0x02
#define GG_FONT_UNDERLINE 0x04
#define GG_FONT_COLOR 0x08
#define GG_FONT_IMAGE 0x80

struct gg_msg_richtext_color { 
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} GG_PACKED;

struct gg_msg_recipients {
	uint8_t flag;
	uint32_t count;
} GG_PACKED;

struct gg_msg_image_request {
	uint8_t flag;
	uint32_t size;
	uint32_t crc32;
} GG_PACKED;

struct gg_msg_image_reply {
	uint8_t flag;
	uint32_t size;
	uint32_t crc32;
	/* char filename[]; */
	/* char image[]; */
} GG_PACKED;

#define GG_SEND_MSG_ACK 0x0005

#define GG_ACK_BLOCKED 0x0001
#define GG_ACK_DELIVERED 0x0002
#define GG_ACK_QUEUED 0x0003
#define GG_ACK_MBOXFULL 0x0004
#define GG_ACK_NOT_DELIVERED 0x0006
	
struct gg_send_msg_ack {
	uint32_t status;
	uint32_t recipient;
	uint32_t seq;
} GG_PACKED;

#define GG_RECV_MSG 0x000a
	
struct gg_recv_msg {
	uint32_t sender;
	uint32_t seq;
	uint32_t time;
	uint32_t msgclass;
} GG_PACKED;

#define GG_PING 0x0008
	
#define GG_PONG 0x0007

#define GG_DISCONNECTING 0x000b

#define GG_USERLIST_REQUEST 0x0016

#define GG_USERLIST_PUT 0x00
#define GG_USERLIST_PUT_MORE 0x01
#define GG_USERLIST_GET 0x02

struct gg_userlist_request {
	uint8_t type;
} GG_PACKED;

#define GG_USERLIST_REPLY 0x0010

#define GG_USERLIST_PUT_REPLY 0x00
#define GG_USERLIST_PUT_MORE_REPLY 0x02
#define GG_USERLIST_GET_REPLY 0x06
#define GG_USERLIST_GET_MORE_REPLY 0x04

struct gg_userlist_reply {
	uint8_t type;
} GG_PACKED;

/*
 * pakiety, stałe, struktury dla DCC
 */

struct gg_dcc_tiny_packet {
	uint8_t type;		/* rodzaj pakietu */
} GG_PACKED;

struct gg_dcc_small_packet {
	uint32_t type;		/* rodzaj pakietu */
} GG_PACKED;

struct gg_dcc_big_packet {
	uint32_t type;		/* rodzaj pakietu */
	uint32_t dunno1;		/* niewiadoma */
	uint32_t dunno2;		/* niewiadoma */
} GG_PACKED;

/*
 * póki co, nie znamy dokładnie protokołu. nie wiemy, co czemu odpowiada.
 * nazwy są niepoważne i tymczasowe.
 */
#define GG_DCC_WANT_FILE 0x0003		/* peer chce plik */
#define GG_DCC_HAVE_FILE 0x0001		/* więc mu damy */
#define GG_DCC_HAVE_FILEINFO 0x0003	/* niech ma informacje o pliku */
#define GG_DCC_GIMME_FILE 0x0006	/* peer jest pewny */
#define GG_DCC_CATCH_FILE 0x0002	/* wysyłamy plik */

#define GG_DCC_FILEATTR_READONLY 0x0020

#define GG_DCC_TIMEOUT_SEND 1800	/* 30 minut */
#define GG_DCC_TIMEOUT_GET 1800		/* 30 minut */
#define GG_DCC_TIMEOUT_FILE_ACK 300	/* 5 minut */
#define GG_DCC_TIMEOUT_VOICE_ACK 300	/* 5 minut */

#ifdef __cplusplus
}
#ifdef _WIN32
#pragma pack(pop)
#endif
#endif

#endif /* __GG_LIBGADU_H */

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: notnil
 * End:
 *
 * vim: shiftwidth=8:
 */
