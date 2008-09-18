
#include <speexio.h>

SpeexIO::SpeexIO()
 : AbstractIO()
{
	setFormat(AlsaIO::Signed16Le);
	
	speex_bits_init(&bits);
	
	int frame_size;
	int quality = 10;
	
	speexEncoder = speex_encoder_init(&speex_nb_mode);
	
	speex_encoder_ctl(speexEncoder, SPEEX_SET_QUALITY, &quality);
	speex_encoder_ctl(speexEncoder, SPEEX_GET_FRAME_SIZE, &frame_size);
	
	qDebug() << "Speex framesize =" << frame_size;

	connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

SpeexIO::~SpeexIO()
{

}

void SpeexIO::write(const QByteArray& data)
{
	Q_UNUSED(data)
}

QByteArray SpeexIO::read()
{
	kDebug() << "called";
	return QByteArray();
}

void SpeexIO::slotReadyRead()
{
	QByteArray data = read();
	
	kDebug() << "Read" << data.size() << "bytes";
	
	char* byte_ptr;
	speex_bits_reset(&bits);
	speex_encode_int(speexEncoder, (short*) data.data(), &bits);

	int speex_byte_number = speex_bits_nbytes(&bits);
	byte_ptr = new char[speex_byte_number];

	int nbBytes = speex_bits_write(&bits, byte_ptr, speex_byte_number);
}
