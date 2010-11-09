/*
 * Jingle call example
 * Copyright 2004--2005, Google Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// LinphoneMediaEngine is a Linphone implementation of MediaEngine
extern "C" {
#include <mediastreamer2/mediastream.h>
#include <mediastreamer2/mssndcard.h>
//#ifdef HAVE_ILBC
//#include "talk/third_party/mediastreamer/msilbcdec.h"
//#endif
#ifdef HAVE_SPEEX
#include <mediastreamer2/msfilter.h>
#endif
}

#include <ortp/ortp.h>
#include <ortp/telephonyevents.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "talk/base/logging.h"
#include "talk/base/thread.h"
#include "talk/session/phone/codec.h"
#include "talk/session/phone/linphonemediaengine.h"

using namespace cricket;

void LinphoneMediaChannel::OnIncomingData(talk_base::AsyncSocket *s)
{
  char *buf[2048];
  int len;
  len = s->Recv(buf, sizeof(buf));
  if (network_interface_ && !mute_)
    network_interface_->SendPacket(buf, len);
}

LinphoneMediaChannel::LinphoneMediaChannel(LinphoneMediaEngine*eng) :
  pt_(-1),
  audio_stream_(0),
  engine_(eng), 
  ring_stream_(0)
 {
  
  talk_base::Thread *thread = talk_base::ThreadManager::CurrentThread();
  talk_base::SocketServer *ss = thread->socketserver();
  socket_.reset(ss->CreateAsyncSocket(SOCK_DGRAM));
  
  socket_->Bind(talk_base::SocketAddress("localhost",3000));
  socket_->SignalReadEvent.connect(this, &LinphoneMediaChannel::OnIncomingData);
}

LinphoneMediaChannel::~LinphoneMediaChannel() {


  //bjd
  fflush(stdout);
  StopRing();

  if (audio_stream_)
      audio_stream_stop(audio_stream_);
}

void LinphoneMediaChannel::StartRing(bool bIncomingCall)
{
  MSSndCard *sndcard = NULL;
  sndcard=ms_snd_card_manager_get_default_card(ms_snd_card_manager_get());
  if (sndcard)
  {
    if (bIncomingCall)
    {
      if (engine_->GetRingWav().size() > 0)
      {
		 LOG(LS_VERBOSE) << "incoming ring. sound file: " << engine_->GetRingWav().c_str() << "\n";
	     ring_stream_ = ring_start (engine_->GetRingWav().c_str(), 1, sndcard);
      }
    }
    else
    {
      if (engine_->GetCallWav().size() > 0)
      {
		LOG(LS_VERBOSE) << "outgoing ring. sound file: " << engine_->GetCallWav().c_str() << "\n";  
	    ring_stream_ = ring_start (engine_->GetCallWav().c_str(), 1, sndcard);
      }
    }
  }
}

void LinphoneMediaChannel::StopRing()
{
	if (ring_stream_) { 
		ring_stop(ring_stream_);
		ring_stream_ = 0;
	}
}


void LinphoneMediaChannel::SetCodecs(const std::vector<Codec> &codecs) {
 bool first = true;
 std::vector<Codec>::const_iterator i;

  ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL);

  for (i = codecs.begin(); i < codecs.end(); i++) {

    if (!engine_->FindCodec(*i))
      continue;
#ifdef HAVE_ILBC	
    if (i->name == payload_type_ilbc.mime_type) {
      rtp_profile_set_payload(&av_profile, i->id, &payload_type_ilbc);
    } 
#endif
#ifdef HAVE_SPEEX
    if (i->name == payload_type_speex_wb.mime_type && i->clockrate == payload_type_speex_wb.clock_rate) {
      rtp_profile_set_payload(&av_profile, i->id, &payload_type_speex_wb);
    } else if (i->name == payload_type_speex_nb.mime_type && i->clockrate == payload_type_speex_nb.clock_rate) {
      rtp_profile_set_payload(&av_profile, i->id, &payload_type_speex_nb);
    }
#endif

    if (i->id == 0)
      rtp_profile_set_payload(&av_profile, 0, &payload_type_pcmu8000);

    if (i->name == payload_type_telephone_event.mime_type) {
      rtp_profile_set_payload(&av_profile, i->id, &payload_type_telephone_event);
    }
    
    if (first) {
      StopRing();
      LOG(LS_INFO) << "Using " << i->name << "/" << i->clockrate;
      pt_ = i->id;
      audio_stream_ = audio_stream_start(&av_profile, 2000, "127.0.0.1", 3000, i->id, 250, 0);
      first = false;
    }
  }
  
  if (first) {
    StopRing();
    // We're being asked to set an empty list of codecs. This will only happen when
    // working with a buggy client; let's try PCMU.
    LOG(LS_WARNING) << "Received empty list of codces; using PCMU/8000";
    audio_stream_ = audio_stream_start(&av_profile, 2000, "127.0.0.1", 3000, 0, 250, 0);
  }
 
}

bool LinphoneMediaEngine::FindCodec(const Codec &c) {
  if (c.id == 0)
    return true;
  if (c.name == payload_type_telephone_event.mime_type)
    return true;
#ifdef HAVE_SPEEX
  if (c.name == payload_type_speex_wb.mime_type && c.clockrate == payload_type_speex_wb.clock_rate)
    return true;
  if (c.name == payload_type_speex_nb.mime_type && c.clockrate == payload_type_speex_nb.clock_rate)
    return true;
#endif
#ifdef HAVE_ILBC
  if (c.name == payload_type_ilbc.mime_type)
    return true;
#endif
return false;
}

void LinphoneMediaChannel::OnPacketReceived(const void *data, int len) {
  uint8 buf[2048];
  memcpy(buf, data, len);
  
  /* We may receive packets with payload type 13: comfort noise. Linphone can't
   * handle them, so let's ignore those packets.
   */
  int payloadtype = buf[1] & 0x7f;
  if (play_ && payloadtype != 13)
    socket_->SendTo(buf, len, talk_base::SocketAddress("localhost",2000));
}

void LinphoneMediaChannel::SetPlayout(bool playout) {
  play_ = playout;
}

void LinphoneMediaChannel::SetSend(bool send) {
  mute_ = !send;
}

int LinphoneMediaChannel::GetOutputLevel() { return 0; }

LinphoneMediaEngine::LinphoneMediaEngine(const std::string& ringWav,  const std::string& callWav)
    : ring_wav_(ringWav),
      call_wav_(callWav)
{
}

LinphoneMediaEngine::~LinphoneMediaEngine() {}

bool LinphoneMediaEngine::Init() {
  ortp_init();
  ms_init();
 
#ifdef HAVE_SPEEX
  //ms_speex_codec_init();
  //ms_filter_register(MS_FILTER_INFO(&speex_info));

  codecs_.push_back(Codec(110, payload_type_speex_wb.mime_type, payload_type_speex_wb.clock_rate, 0, 1, 8));
  codecs_.push_back(Codec(111, payload_type_speex_nb.mime_type, payload_type_speex_nb.clock_rate, 0, 1, 7));
  
#endif

#ifdef HAVE_ILBC
  //ms_ilbc_codec_init();
  codecs_.push_back(Codec(102, payload_type_ilbc.mime_type, payload_type_ilbc.clock_rate, 0, 1, 4));
#endif

  codecs_.push_back(Codec(0, payload_type_pcmu8000.mime_type, payload_type_pcmu8000.clock_rate, 0, 1, 2));
  codecs_.push_back(Codec(101, payload_type_telephone_event.mime_type, payload_type_telephone_event.clock_rate, 0, 1, 1));
  return true;
}

void LinphoneMediaEngine::Terminate() {
  fflush(stdout);
}
  
MediaChannel *LinphoneMediaEngine::CreateChannel() {
  return new LinphoneMediaChannel(this);
}

int LinphoneMediaEngine::SetAudioOptions(int options) { return 0; }
int LinphoneMediaEngine::SetSoundDevices(int wave_in_device, int wave_out_device) { return 0; }

float LinphoneMediaEngine::GetCurrentQuality() { return 0; }
int LinphoneMediaEngine::GetInputLevel() { return 0; }
