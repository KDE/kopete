/*
 * libjingle
 * Copyright 2010, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// LinphoneMediaEngine is a Linphone implementation of MediaEngine

#ifndef TALK_SESSION_PHONE_LINPHONEMEDIAENGINE_H_
#define TALK_SESSION_PHONE_LINPHONEMEDIAENGINE_H_

#ifdef HAVE_LINPHONE

#include <string>
#include <vector>

extern "C" {
#include <mediastreamer2/mediastream.h>
}

#include "talk/base/scoped_ptr.h"
#include "talk/session/phone/codec.h"
#include "talk/session/phone/mediachannel.h"
#include "talk/session/phone/mediaengine.h"

namespace talk_base {
class StreamInterface;
}

namespace cricket {

class LinphoneMediaEngine : public MediaEngineInterface {
 public:
  LinphoneMediaEngine(const std::string& ringWav,  const std::string& callWav);
  virtual ~LinphoneMediaEngine() {}

  // Implement pure virtual methods of MediaEngine.
  virtual bool Init() { return true; }
  virtual void Terminate();
  virtual int GetCapabilities();
  virtual VoiceMediaChannel* CreateChannel();
  virtual VideoMediaChannel* CreateVideoChannel(VoiceMediaChannel* voice_ch);
  virtual SoundclipMedia* CreateSoundclip() { return NULL; }
  virtual bool SetAudioOptions(int options) { return true; }
  virtual bool SetVideoOptions(int options) { return true; }
  virtual bool SetDefaultVideoEncoderConfig(const VideoEncoderConfig& config) {
    return true;
  }
  virtual bool SetSoundDevices(const Device* in_dev, const Device* out_dev) {
    return true;
  }
  virtual bool SetVideoCaptureDevice(const Device* cam_device) { return true; }
  virtual bool SetOutputVolume(int level) { return true; }
  virtual int GetInputLevel() { return 0; }
  virtual bool SetLocalMonitor(bool enable) { return true; }
  virtual bool SetLocalRenderer(VideoRenderer* renderer) { return true; }
  // TODO: control channel send?
  virtual CaptureResult SetVideoCapture(bool capture) { return CR_SUCCESS; }
  virtual const std::vector<AudioCodec>& audio_codecs() { return voice_codecs_; }
  virtual const std::vector<VideoCodec>& video_codecs() { return video_codecs_; }
  virtual bool FindAudioCodec(const AudioCodec& codec);
  virtual bool FindVideoCodec(const VideoCodec& codec) { return true; }
  virtual void SetVoiceLogging(int min_sev, const char* filter) {}
  virtual void SetVideoLogging(int min_sev, const char* filter) {}
  virtual bool SetVideoCapturer(cricket::VideoCapturer*, uint32) { return true; }
  virtual bool GetOutputVolume(int*) { return true; }
  virtual bool RegisterVideoProcessor(cricket::VideoProcessor*) { return true; }
  virtual bool UnregisterVideoProcessor(cricket::VideoProcessor*) { return true; }
  virtual bool RegisterVoiceProcessor(uint32, cricket::VoiceProcessor*, cricket::MediaProcessorDirection) { return true; }
  virtual bool UnregisterVoiceProcessor(uint32, cricket::VoiceProcessor*, cricket::MediaProcessorDirection) { return true; }

  std::string GetRingWav(){return ring_wav_;}
  std::string GetCallWav(){return call_wav_;}

  bool have_ilbc;
  bool have_speex;
  bool have_gsm;

 private:
  std::vector<AudioCodec> voice_codecs_;
  std::vector<VideoCodec> video_codecs_;

  std::string ring_wav_;
  std::string call_wav_;

  DISALLOW_COPY_AND_ASSIGN(LinphoneMediaEngine);
};

class LinphoneVoiceChannel : public VoiceMediaChannel {
 public:
  LinphoneVoiceChannel(LinphoneMediaEngine *eng);
  virtual ~LinphoneVoiceChannel();

  // Implement pure virtual methods of VoiceMediaChannel.
  virtual bool SetRecvCodecs(const std::vector<AudioCodec>& codecs);
  virtual bool SetSendCodecs(const std::vector<AudioCodec>& codecs);
  virtual bool SetPlayout(bool playout);
  virtual bool SetSend(SendFlags flag);
  virtual bool GetActiveStreams(AudioInfo::StreamList* actives) { return true; }
  virtual int GetOutputLevel() { return 0; }
  virtual bool SetOutputScaling(uint32 ssrc, double left, double right) {
    return false;
  }
  virtual bool GetOutputScaling(uint32 ssrc, double* left, double* right) {
    return false;
  }
  virtual bool SetRingbackTone(const char* buf, int len) { return true; }
  virtual bool PlayRingbackTone(bool play, bool loop) { return true; }
  virtual bool PressDTMF(int event, bool playout) { return true; }
  virtual bool GetStats(VoiceMediaInfo* info) { return true; }

  // Implement pure virtual methods of MediaChannel.
  virtual void OnPacketReceived(talk_base::Buffer* packet);
  virtual void OnRtcpReceived(talk_base::Buffer* packet);
  virtual bool Mute(bool on);
  virtual bool SetSendBandwidth(bool autobw, int bps) { return true; }
  virtual bool SetOptions(int options) { return true; }
  virtual bool SetRecvRtpHeaderExtensions(
      const std::vector<RtpHeaderExtension>& extensions) { return true; }
  virtual bool SetSendRtpHeaderExtensions(
      const std::vector<RtpHeaderExtension>& extensions) { return true; }
  virtual bool AddSendStream(const cricket::StreamParams&);
  virtual bool RemoveSendStream(uint32) { return true; }
  virtual bool AddRecvStream(const cricket::StreamParams&) { return true; }
  virtual bool RemoveRecvStream(uint32) { return true; }
  virtual int GetOptions() const { return 0; }
  virtual bool PlayRingbackTone(uint32, bool, bool) { return true; }

  virtual void StartRing(bool bIncomingCall);
  virtual void StopRing();

 private:
  int pt_;
  bool profile_;
  bool mute_;
  bool play_;
  AudioStream *audio_stream_;
  LinphoneMediaEngine *engine_;
  RingStream* ring_stream_;
  talk_base::scoped_ptr<talk_base::AsyncSocket> socket_;
  talk_base::scoped_ptr<talk_base::AsyncSocket> socketRtcp_;
  void OnIncomingData(talk_base::AsyncSocket *s);
  void OnIncomingRtcp(talk_base::AsyncSocket *s);
  bool StartCall();

  int captport; // local port for audio_stream
  int playport; // local port for rtp
  int playport2; // local port for rtcp

  DISALLOW_COPY_AND_ASSIGN(LinphoneVoiceChannel);
};

}  // namespace cricket

#endif // HAVE_LINPHONE

#endif  // TALK_SESSION_PHONE_LINPHONEMEDIAENGINE_H_
