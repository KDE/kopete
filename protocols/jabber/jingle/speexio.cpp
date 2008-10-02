
#include <speexio.h>

SpeexIO::SpeexIO(int samplingRate)
 : AbstractIO()
{
	//AlsaIO::Format format;

	kDebug() << "sampling rate =" << samplingRate;

	kDebug() << "Create SpeexIO";
	
	speex_bits_init(&encodeBits);
	speex_bits_init(&decodeBits);
	
	int encoder_frame_size;
	int encoder_sampling_rate;
	int encoder_bit_rate;
	int decoder_sampling_rate;
	int decoder_bit_rate;
	int quality = 10;
	
	speexEncoder = speex_encoder_init(&speex_nb_mode);
	speexDecoder = speex_decoder_init(&speex_nb_mode);
	
	// A frame is a bunch of samples
	// FRAME_SIZE is the number of sample per frame.
	// a sample is 20 ms
	int setSR = samplingRate;
	kDebug() << "setting sampling rate" << setSR; 
	if (speex_encoder_ctl(speexEncoder, SPEEX_SET_SAMPLING_RATE, &setSR) != 0)
	{
		kDebug() << "Error setting encoder sampling rate !";
	}
	if (0 != speex_encoder_ctl(speexEncoder, SPEEX_GET_FRAME_SIZE, &encoder_frame_size))
	{
		kDebug() << "Error getting encoder frame size !";
	}
	if (0 != speex_encoder_ctl(speexEncoder, SPEEX_GET_SAMPLING_RATE, &encoder_sampling_rate))
	{
		kDebug() << "Error getting encoder sampling rate !";
	}
	if (0 != speex_encoder_ctl(speexEncoder, SPEEX_GET_BITRATE, &encoder_bit_rate))
	{
		kDebug() << "Error getting encoder bit rate !";
	}
	//Trying different quality/bitrate
	/*for (int i = 0; i <= 10; i++)
	{
		quality = i;
		if (0 != speex_encoder_ctl(speexEncoder, SPEEX_SET_QUALITY, &quality))
		{
			kDebug() << "Error setting encoder quality !";
		}
		if (0 != speex_encoder_ctl(speexEncoder, SPEEX_GET_BITRATE, &encoder_bit_rate))
		{
			kDebug() << "Error getting encoder bit rate !";
		}
		kDebug() << "Quality =" << quality << ", bitrate =" << encoder_bit_rate;
	}*/
	
	setSR = samplingRate;
	kDebug() << "setting sampling rate" << setSR; 
	if (speex_decoder_ctl(speexDecoder, SPEEX_SET_SAMPLING_RATE, &setSR) != 0)
		kDebug() << "Error setting decoder sampling rate";
	if (speex_decoder_ctl(speexDecoder, SPEEX_GET_FRAME_SIZE, &decoderFrameSize) != 0)
		kDebug() << "Error getting decoder frame size";
	if (speex_decoder_ctl(speexDecoder, SPEEX_GET_SAMPLING_RATE, &decoder_sampling_rate) != 0)
		kDebug() << "Error getting decoder sampling rate";
	if (speex_decoder_ctl(speexDecoder, SPEEX_GET_BITRATE, &decoder_bit_rate) != 0)
		kDebug() << "Error getting decoder bit rate";
	
	kDebug() << "Speex encoder framesize =" << encoder_frame_size;
	kDebug() << "Speex encoder samplingrate =" << encoder_sampling_rate;
	kDebug() << "Speex encoder bitrate =" << encoder_bit_rate;
	
	kDebug() << "Speex decoder framesize =" << decoderFrameSize;
	kDebug() << "Speex decoder samplingrate =" << decoder_sampling_rate;
	kDebug() << "Speex decoder bitrate =" << decoder_bit_rate;

	m_alsaIn = new AlsaIO(AlsaIO::Capture, AlsaIO::Signed8);
	//m_alsaIn->setSamplingRate(samplingRate);
	//m_alsaIn->setFrameSize(encoder_frame_size);
	//m_alsaIn->setBitRate(encoder_bit_rate);
	//	-->FIXME:may have to do that
	m_alsaOut = new AlsaIO(AlsaIO::Playback, AlsaIO::Signed8);
	connect(m_alsaOut, SIGNAL(bytesWritten()), this, SLOT(slotBytesWritten()));
	
	connect(m_alsaIn, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

SpeexIO::~SpeexIO()
{
	speex_bits_destroy(&encodeBits); 
	speex_bits_destroy(&decodeBits); 

	speex_encoder_destroy(speexEncoder);
	speex_decoder_destroy(speexDecoder);
}

void SpeexIO::write(const QByteArray& data)
{
	//Decode speex data and give it to Alsa Out.
	rawData.clear();

	kDebug() << "Data size :" << data.size();
	
	speex_bits_read_from(&decodeBits, (char*) data.data(), data.size());
	
	kDebug() << "bits_nbytes =" << speex_bits_nbytes(&decodeBits);
	
	kDebug() << "alsa framesizebytes =" << m_alsaOut->frameSizeBytes() << "speex framesizesample =" << decoderFrameSize;
	
	rawData.resize(m_alsaOut->frameSizeBytes());
	
	kDebug() << "rawData.size() =" << rawData.size();

	//FIXME:A NULL value as the second argument indicates that we don't have the bits for the current frame. When a frame is lost,
	//	the Speex decoder will do its best to "guess" the correct signal.
	speex_decode_int(speexDecoder, &decodeBits, (short*) rawData.data());
	
	kDebug() << "rawData.size() =" << rawData.size();

	m_alsaOut->write(rawData);
}

int SpeexIO::start()
{
	m_alsaIn->start();
	m_alsaOut->start();
	return 0;
}

QByteArray SpeexIO::read()
{
	kDebug() << "called";
	return speexData;
}

void SpeexIO::slotReadyRead()
{
	speexData.clear();

	QByteArray data = m_alsaIn->data();
	
	kDebug() << "Read" << data.size() << "bytes";

	data.resize(data.size() - 10);
	
	speex_bits_reset(&encodeBits);
	speex_encode_int(speexEncoder, (short*) data.data(), &encodeBits);

	int speex_byte_number = speex_bits_nbytes(&encodeBits);
	speexData.resize(speex_byte_number);

	int nbBytes = speex_bits_write(&encodeBits, speexData.data(), speex_byte_number);

	speexData.resize(nbBytes);

	emit readyRead();
}

void SpeexIO::slotBytesWritten()
{
	//Data have been written, it is now ok to delete them.
	//kDebug() << "Delete data !, Not done, check memory leak !";
	//rawData.clear();
}

