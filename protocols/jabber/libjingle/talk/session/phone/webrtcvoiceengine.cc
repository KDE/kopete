/*
 * libjingle
 * Copyright 2004--2011, Google Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_WEBRTC_VOICE

#include "talk/session/phone/webrtcvoiceengine.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include "talk/base/base64.h"
#include "talk/base/byteorder.h"
#include "talk/base/common.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/stringencode.h"
#include "talk/base/stringutils.h"
#include "talk/session/phone/streamparams.h"
#include "talk/session/phone/voiceprocessor.h"
#include "talk/session/phone/webrtcvoe.h"

#ifdef WIN32
#include <objbase.h>  // NOLINT
#endif

namespace cricket {

// For Linux/Mac, using the default device is done by specifying index 0 for
// VoE 4.0 and not -1 (which was the case for VoE 3.5).
//
// On Windows Vista and newer, Microsoft introduced the concept of "Default
// Communications Device". This means that there are two types of default
// devices (old Wave Audio style default and Default Communications Device).
//
// On Windows systems which only support Wave Audio style default, uses either
// -1 or 0 to select the default device.
//
// On Windows systems which support both "Default Communication Device" and
// old Wave Audio style default, use -1 for Default Communications Device and
// -2 for Wave Audio style default, which is what we want to use for clips.
// It's not clear yet whether the -2 index is handled properly on other OSes.

#ifdef WIN32
static const int kDefaultAudioDeviceId = -1;
static const int kDefaultSoundclipDeviceId = -2;
#else
static const int kDefaultAudioDeviceId = 0;
#endif

// extension header for audio levels, as defined in
// http://tools.ietf.org/html/draft-ietf-avtext-client-to-mixer-audio-level-03
static const char kRtpAudioLevelHeaderExtension[] =
    "urn:ietf:params:rtp-hdrext:ssrc-audio-level";

static const char kIsacCodecName[] = "ISAC";
static const char kL16CodecName[] = "L16";

// Dumps an AudioCodec in RFC 2327-ish format.
static std::string ToString(const AudioCodec& codec) {
  std::stringstream ss;
  ss << codec.name << "/" << codec.clockrate << "/" << codec.channels
     << " (" << codec.id << ")";
  return ss.str();
}
static std::string ToString(const webrtc::CodecInst& codec) {
  std::stringstream ss;
  ss << codec.plname << "/" << codec.plfreq << "/" << codec.channels
     << " (" << codec.pltype << ")";
  return ss.str();
}

static void LogMultiline(talk_base::LoggingSeverity sev, char* text) {
  const char* delim = "\r\n";
  for (char* tok = strtok(text, delim); tok; tok = strtok(NULL, delim)) {
    LOG_V(sev) << tok;
  }
}

// WebRtcVoiceEngine
const WebRtcVoiceEngine::CodecPref WebRtcVoiceEngine::kCodecPrefs[] = {
  { "ISAC",   16000,  103 },
  { "ISAC",   32000,  104 },
  { "speex",  16000,  107 },
  { "G722",   16000,  9 },
  { "ILBC",   8000,   102 },
  { "speex",  8000,   108 },
  { "PCMU",   8000,   0 },
  { "PCMA",   8000,   8 },
  { "CN",     32000,  106 },
  { "CN",     16000,  105 },
  { "CN",     8000,   13 },
  { "red",    8000,   127 },
  { "telephone-event", 8000, 126 },
};

class WebRtcSoundclipMedia : public SoundclipMedia {
 public:
  explicit WebRtcSoundclipMedia(WebRtcVoiceEngine *engine)
      : engine_(engine), webrtc_channel_(-1) {
    engine_->RegisterSoundclip(this);
  }

  virtual ~WebRtcSoundclipMedia() {
    engine_->UnregisterSoundclip(this);
    if (webrtc_channel_ != -1) {
      // We shouldn't have to call Disable() here. DeleteChannel() should call
      // StopPlayout() while deleting the channel.  We should fix the bug
      // inside WebRTC and remove the Disable() call bellow.  This work is
      // tracked by bug http://b/issue?id=5382855.
      PlaySound(NULL, 0, 0);
      Disable();
      if (engine_->voe_sc()->base()->DeleteChannel(webrtc_channel_)
          == -1) {
        LOG_RTCERR1(DeleteChannel, webrtc_channel_);
      }
    }
  }

  bool Init() {
    webrtc_channel_ = engine_->voe_sc()->base()->CreateChannel();
    if (webrtc_channel_ == -1) {
      LOG_RTCERR0(CreateChannel);
      return false;
    }
    return true;
  }

  bool Enable() {
    if (engine_->voe_sc()->base()->StartPlayout(webrtc_channel_) == -1) {
      LOG_RTCERR1(StartPlayout, webrtc_channel_);
      return false;
    }
    return true;
  }

  bool Disable() {
    if (engine_->voe_sc()->base()->StopPlayout(webrtc_channel_) == -1) {
      LOG_RTCERR1(StopPlayout, webrtc_channel_);
      return false;
    }
    return true;
  }

  virtual bool PlaySound(const char *buf, int len, int flags) {
    // The voe file api is not available in chrome.
    if (!engine_->voe_sc()->file()) {
      return false;
    }
    // Must stop playing the current sound (if any), because we are about to
    // modify the stream.
    if (engine_->voe_sc()->file()->StopPlayingFileLocally(webrtc_channel_)
        == -1) {
      LOG_RTCERR1(StopPlayingFileLocally, webrtc_channel_);
      return false;
    }

    if (buf) {
      stream_.reset(new WebRtcSoundclipStream(buf, len));
      stream_->set_loop((flags & SF_LOOP) != 0);
      stream_->Rewind();

      // Play it.
      if (engine_->voe_sc()->file()->StartPlayingFileLocally(
          webrtc_channel_, stream_.get()) == -1) {
        LOG_RTCERR2(StartPlayingFileLocally, webrtc_channel_, stream_.get());
        LOG(LS_ERROR) << "Unable to start soundclip";
        return false;
      }
    } else {
      stream_.reset();
    }
    return true;
  }

  int GetLastEngineError() const { return engine_->voe_sc()->error(); }

 private:
  WebRtcVoiceEngine *engine_;
  int webrtc_channel_;
  talk_base::scoped_ptr<WebRtcSoundclipStream> stream_;
};

WebRtcVoiceEngine::WebRtcVoiceEngine()
    : voe_wrapper_(new VoEWrapper()),
      voe_wrapper_sc_(new VoEWrapper()),
      tracing_(new VoETraceWrapper()),
      adm_(NULL),
      adm_sc_(NULL),
      log_level_(kDefaultLogSeverity),
      is_dumping_aec_(false),
      desired_local_monitor_enable_(false) {
  Construct();
}

WebRtcVoiceEngine::WebRtcVoiceEngine(VoEWrapper* voe_wrapper,
                                     VoEWrapper* voe_wrapper_sc,
                                     VoETraceWrapper* tracing)
    : voe_wrapper_(voe_wrapper),
      voe_wrapper_sc_(voe_wrapper_sc),
      tracing_(tracing),
      adm_(NULL),
      adm_sc_(NULL),
      log_level_(kDefaultLogSeverity),
      is_dumping_aec_(false),
      desired_local_monitor_enable_(false) {
  Construct();
}

void WebRtcVoiceEngine::Construct() {
  initialized_ = false;
  LOG(LS_VERBOSE) << "WebRtcVoiceEngine::WebRtcVoiceEngine";
  ApplyLogging("");
  if (tracing_->SetTraceCallback(this) == -1) {
    LOG_RTCERR0(SetTraceCallback);
  }
  if (voe_wrapper_->base()->RegisterVoiceEngineObserver(*this) == -1) {
    LOG_RTCERR0(RegisterVoiceEngineObserver);
  }
  // Clear the default agc state.
  memset(&default_agc_config_, 0, sizeof(default_agc_config_));

  // Load our audio codec list.
  ConstructCodecs();
}

void WebRtcVoiceEngine::ConstructCodecs() {
  LOG(LS_INFO) << "WebRtc VoiceEngine codecs:";
  int ncodecs = voe_wrapper_->codec()->NumOfCodecs();
  for (int i = 0; i < ncodecs; ++i) {
    webrtc::CodecInst voe_codec;
    if (voe_wrapper_->codec()->GetCodec(i, voe_codec) != -1) {
      // Skip uncompressed formats.
      if (_stricmp(voe_codec.plname, kL16CodecName) == 0) {
        continue;
      }

      const CodecPref* pref = NULL;
      for (size_t j = 0; j < ARRAY_SIZE(kCodecPrefs); ++j) {
        if (_stricmp(kCodecPrefs[j].name, voe_codec.plname) == 0 &&
            kCodecPrefs[j].clockrate == voe_codec.plfreq) {
          pref = &kCodecPrefs[j];
          break;
        }
      }

      if (pref) {
        // Use the payload type that we've configured in our pref table;
        // use the offset in our pref table to determine the sort order.
        AudioCodec codec(pref->payload_type, voe_codec.plname, voe_codec.plfreq,
                         voe_codec.rate, voe_codec.channels,
                         ARRAY_SIZE(kCodecPrefs) - (pref - kCodecPrefs));
        LOG(LS_INFO) << ToString(codec);
        // For ISAC, use 0 to indicate auto bandwidth in our signaling.
        if (_stricmp(codec.name.c_str(), kIsacCodecName) == 0) {
          codec.bitrate = 0;
        }
        codecs_.push_back(codec);
      } else {
        LOG(LS_WARNING) << "Unexpected codec: " << ToString(voe_codec);
      }
    }
  }
  // Make sure they are in local preference order.
  std::sort(codecs_.begin(), codecs_.end(), &AudioCodec::Preferable);
}

WebRtcVoiceEngine::~WebRtcVoiceEngine() {
  LOG(LS_VERBOSE) << "WebRtcVoiceEngine::~WebRtcVoiceEngine";
  if (voe_wrapper_->base()->DeRegisterVoiceEngineObserver() == -1) {
    LOG_RTCERR0(DeRegisterVoiceEngineObserver);
  }
  if (adm_) {
    voe_wrapper_.reset();
    adm_->Release();
    adm_ = NULL;
  }
  if (adm_sc_) {
    voe_wrapper_sc_.reset();
    adm_sc_->Release();
    adm_sc_ = NULL;
  }

  // Test to see if the media processor was deregistered properly
  ASSERT(SignalRxMediaFrame.is_empty());
  ASSERT(SignalTxMediaFrame.is_empty());

  tracing_->SetTraceCallback(NULL);
}

bool WebRtcVoiceEngine::Init() {
  LOG(LS_INFO) << "WebRtcVoiceEngine::Init";
  bool res = InitInternal();
  if (res) {
    LOG(LS_INFO) << "WebRtcVoiceEngine::Init Done!";
  } else {
    LOG(LS_ERROR) << "WebRtcVoiceEngine::Init failed";
    Terminate();
  }
  return res;
}

bool WebRtcVoiceEngine::InitInternal() {
  // Temporarily turn logging level up for the Init call
  int old_level = log_level_;
  log_level_ = talk_base::_min(log_level_,
                               static_cast<int>(talk_base::LS_INFO));
  ApplyLogging("");

  // Init WebRtc VoiceEngine, enabling AEC logging if specified in SetLogging.
  if (voe_wrapper_->base()->Init(adm_) == -1) {
    LOG_RTCERR0_EX(Init, voe_wrapper_->error());
    return false;
  }

  // Restore the previous log level and apply the log filter.
  log_level_ = old_level;
  ApplyLogging(log_filter_);

  // Log the VoiceEngine version info
  char buffer[1024] = "";
  voe_wrapper_->base()->GetVersion(buffer);
  LOG(LS_INFO) << "WebRtc VoiceEngine Version:";
  LogMultiline(talk_base::LS_INFO, buffer);

  // Turn on AEC and AGC by default.
  if (!SetOptions(
      MediaEngineInterface::ECHO_CANCELLATION |
      MediaEngineInterface::AUTO_GAIN_CONTROL)) {
    return false;
  }

  // Save the default AGC configuration settings.
  if (voe_wrapper_->processing()->SetAgcConfig(default_agc_config_) == -1) {
    LOG_RTCERR0(GetAGCConfig);
    return false;
  }

  // Print our codec list again for the call diagnostic log
  LOG(LS_INFO) << "WebRtc VoiceEngine codecs:";
  for (std::vector<AudioCodec>::const_iterator it = codecs_.begin();
      it != codecs_.end(); ++it) {
    LOG(LS_INFO) << ToString(*it);
  }

#if defined(LINUX) && !defined(HAVE_LIBPULSE)
  voe_wrapper_sc_->hw()->SetAudioDeviceLayer(webrtc::kAudioLinuxAlsa);
#endif

  // Initialize the VoiceEngine instance that we'll use to play out sound clips.
  if (voe_wrapper_sc_->base()->Init(adm_sc_) == -1) {
    LOG_RTCERR0_EX(Init, voe_wrapper_sc_->error());
    return false;
  }

  // On Windows, tell it to use the default sound (not communication) devices.
  // First check whether there is a valid sound device for playback.
  // TODO: Clean this up when we support setting the soundclip device.
#ifdef WIN32
  // The SetPlayoutDevice may not be implemented in the case of external ADM.
  // TODO: We should only check the adm_sc_ here, but current
  // PeerConnection interface never set the adm_sc_, so need to check both
  // in order to determine if the external adm is used.
  if (!adm_ && !adm_sc_) {
    int num_of_devices = 0;
    if (voe_wrapper_sc_->hw()->GetNumOfPlayoutDevices(num_of_devices) != -1 &&
        num_of_devices > 0) {
      if (voe_wrapper_sc_->hw()->SetPlayoutDevice(kDefaultSoundclipDeviceId)
          == -1) {
        LOG_RTCERR1_EX(SetPlayoutDevice, kDefaultSoundclipDeviceId,
                       voe_wrapper_sc_->error());
        return false;
      }
    } else {
      LOG(LS_WARNING) << "No valid sound playout device found.";
    }
  }
#endif

  initialized_ = true;
  return true;
}

void WebRtcVoiceEngine::Terminate() {
  LOG(LS_INFO) << "WebRtcVoiceEngine::Terminate";
  initialized_ = false;

  if (is_dumping_aec_) {
    if (voe_wrapper_->processing()->StopDebugRecording() == -1) {
      LOG_RTCERR0(StopDebugRecording);
    }
    is_dumping_aec_ = false;
  }

  voe_wrapper_sc_->base()->Terminate();
  voe_wrapper_->base()->Terminate();
  desired_local_monitor_enable_ = false;
}

int WebRtcVoiceEngine::GetCapabilities() {
  return AUDIO_SEND | AUDIO_RECV;
}

VoiceMediaChannel *WebRtcVoiceEngine::CreateChannel() {
  WebRtcVoiceMediaChannel* ch = new WebRtcVoiceMediaChannel(this);
  if (!ch->valid()) {
    delete ch;
    ch = NULL;
  }
  return ch;
}

SoundclipMedia *WebRtcVoiceEngine::CreateSoundclip() {
  WebRtcSoundclipMedia *soundclip = new WebRtcSoundclipMedia(this);
  if (!soundclip->Init() || !soundclip->Enable()) {
    delete soundclip;
    return NULL;
  }
  return soundclip;
}

bool WebRtcVoiceEngine::SetOptions(int options) {
  // NS and typing detection are always on, if supported.
  bool aec = (options & MediaEngineInterface::ECHO_CANCELLATION) ? true : false;
  bool agc = (options & MediaEngineInterface::AUTO_GAIN_CONTROL) ? true : false;
#if !defined(IOS) && !defined(ANDROID)
  if (voe_wrapper_->processing()->SetEcStatus(aec) == -1) {
    LOG_RTCERR1(SetEcStatus, aec);
    return false;
  }
  if (aec) {
    if (voe_wrapper_->processing()->SetEcMetricsStatus(true) == -1) {
      LOG_RTCERR1(SetEcMetricsStatus, true);
      return false;
    }
  }

  if (voe_wrapper_->processing()->SetAgcStatus(agc) == -1) {
    LOG_RTCERR1(SetAgcStatus, agc);
    return false;
  }

  if (voe_wrapper_->processing()->SetNsStatus(true) == -1) {
    LOG_RTCERR1(SetNsStatus, true);
    return false;
  }

  if (voe_wrapper_->processing()->SetTypingDetectionStatus(true) == -1) {
    // In case of error, log the info and continue
    LOG_RTCERR1(SetTypingDetectionStatus, true);
  }
#else
  if (voe_wrapper_->processing()->SetEcStatus(aec, kEcAecm) == -1) {
    LOG_RTCERR2(SetEcStatus, aec, kEcAecm);
    return false;
  }

  if (aec) {
    // Use speakerphone mode with comfort noise generation for mobile.
    if (voe_wrapper_->processing()->SetAecmMode(kAecmSpeakerphone, true) != 0) {
      LOG_RTCERR2(SetAecmMode, kAecmSpeakerphone, true);
    }
  }

  // On mobile, GIPS recommends fixed AGC (not adaptive)
  if (voe_wrapper_->processing()->SetAgcStatus(agc, kAgcFixedDigital) == -1) {
    LOG_RTCERR2(SetAgcStatus, agc, kAgcFixedDigital);
    return false;
  }

  // On mobile, GIPS recommends moderate aggressiveness.
  if (voe_wrapper_->processing()->SetNsStatus(true,
      kNsModerateSuppression) == -1) {
    LOG_RTCERR2(SetNsStatus, ns, kNsModerateSuppression);
    return false;
  }

  // No typing detection support on iOS or Android.
#endif  // !IOS && !ANDROID

  return true;
}

struct ResumeEntry {
  ResumeEntry(WebRtcVoiceMediaChannel *c, bool p, SendFlags s)
      : channel(c),
        playout(p),
        send(s) {
  }

  WebRtcVoiceMediaChannel *channel;
  bool playout;
  SendFlags send;
};

// TODO: Refactor this so that the core logic can be used to set the
// soundclip device. At that time, reinstate the soundclip pause/resume code.
bool WebRtcVoiceEngine::SetDevices(const Device* in_device,
                                   const Device* out_device) {
#if !defined(IOS) && !defined(ANDROID)
  int in_id = in_device ? talk_base::FromString<int>(in_device->id) :
      kDefaultAudioDeviceId;
  int out_id = out_device ? talk_base::FromString<int>(out_device->id) :
      kDefaultAudioDeviceId;
  // The device manager uses -1 as the default device, which was the case for
  // VoE 3.5. VoE 4.0, however, uses 0 as the default in Linux and Mac.
#ifndef WIN32
  if (-1 == in_id) {
    in_id = kDefaultAudioDeviceId;
  }
  if (-1 == out_id) {
    out_id = kDefaultAudioDeviceId;
  }
#endif

  std::string in_name = (in_id != kDefaultAudioDeviceId) ?
      in_device->name : "Default device";
  std::string out_name = (out_id != kDefaultAudioDeviceId) ?
      out_device->name : "Default device";
  LOG(LS_INFO) << "Setting microphone to (id=" << in_id << ", name=" << in_name
            << ") and speaker to (id=" << out_id << ", name=" << out_name
            << ")";

  // If we're running the local monitor, we need to stop it first.
  bool ret = true;
  if (!PauseLocalMonitor()) {
    LOG(LS_WARNING) << "Failed to pause local monitor";
    ret = false;
  }

  // Must also pause all audio playback and capture.
  for (ChannelList::const_iterator i = channels_.begin();
       i != channels_.end(); ++i) {
    WebRtcVoiceMediaChannel *channel = *i;
    if (!channel->PausePlayout()) {
      LOG(LS_WARNING) << "Failed to pause playout";
      ret = false;
    }
    if (!channel->PauseSend()) {
      LOG(LS_WARNING) << "Failed to pause send";
      ret = false;
    }
  }

  // Find the recording device id in VoiceEngine and set recording device.
  if (!FindWebRtcAudioDeviceId(true, in_name, in_id, &in_id)) {
    ret = false;
  }
  if (ret) {
    if (voe_wrapper_->hw()->SetRecordingDevice(in_id) == -1) {
      LOG_RTCERR2(SetRecordingDevice, in_device->name, in_id);
      ret = false;
    }
  }

  // Find the playout device id in VoiceEngine and set playout device.
  if (!FindWebRtcAudioDeviceId(false, out_name, out_id, &out_id)) {
    LOG(LS_WARNING) << "Failed to find VoiceEngine device id for " << out_name;
    ret = false;
  }
  if (ret) {
    if (voe_wrapper_->hw()->SetPlayoutDevice(out_id) == -1) {
      LOG_RTCERR2(SetPlayoutDevice, out_device->name, out_id);
      ret = false;
    }
  }

  // Resume all audio playback and capture.
  for (ChannelList::const_iterator i = channels_.begin();
       i != channels_.end(); ++i) {
    WebRtcVoiceMediaChannel *channel = *i;
    if (!channel->ResumePlayout()) {
      LOG(LS_WARNING) << "Failed to resume playout";
      ret = false;
    }
    if (!channel->ResumeSend()) {
      LOG(LS_WARNING) << "Failed to resume send";
      ret = false;
    }
  }

  // Resume local monitor.
  if (!ResumeLocalMonitor()) {
    LOG(LS_WARNING) << "Failed to resume local monitor";
    ret = false;
  }

  if (ret) {
    LOG(LS_INFO) << "Set microphone to (id=" << in_id <<" name=" << in_name
                 << ") and speaker to (id="<< out_id << " name=" << out_name
                 << ")";
  }

  return ret;
#else
  return true;
#endif  // !IOS && !ANDROID
}

bool WebRtcVoiceEngine::FindWebRtcAudioDeviceId(
  bool is_input, const std::string& dev_name, int dev_id, int* rtc_id) {
  // In Linux, VoiceEngine uses the same device dev_id as the device manager.
#ifdef LINUX
  *rtc_id = dev_id;
  return true;
#else
  // In Windows and Mac, we need to find the VoiceEngine device id by name
  // unless the input dev_id is the default device id.
  if (kDefaultAudioDeviceId == dev_id) {
    *rtc_id = dev_id;
    return true;
  }

  // Get the number of VoiceEngine audio devices.
  int count = 0;
  if (is_input) {
    if (-1 == voe_wrapper_->hw()->GetNumOfRecordingDevices(count)) {
      LOG_RTCERR0(GetNumOfRecordingDevices);
      return false;
    }
  } else {
    if (-1 == voe_wrapper_->hw()->GetNumOfPlayoutDevices(count)) {
      LOG_RTCERR0(GetNumOfPlayoutDevices);
      return false;
    }
  }

  for (int i = 0; i < count; ++i) {
    char name[128];
    char guid[128];
    if (is_input) {
      voe_wrapper_->hw()->GetRecordingDeviceName(i, name, guid);
      LOG(LS_VERBOSE) << "VoiceEngine microphone " << i << ": " << name;
    } else {
      voe_wrapper_->hw()->GetPlayoutDeviceName(i, name, guid);
      LOG(LS_VERBOSE) << "VoiceEngine speaker " << i << ": " << name;
    }

    std::string webrtc_name(name);
    if (dev_name.compare(0, webrtc_name.size(), webrtc_name) == 0) {
      *rtc_id = i;
      return true;
    }
  }
  LOG(LS_WARNING) << "VoiceEngine cannot find device: " << dev_name;
  return false;
#endif
}

bool WebRtcVoiceEngine::GetOutputVolume(int* level) {
  unsigned int ulevel;
  if (voe_wrapper_->volume()->GetSpeakerVolume(ulevel) == -1) {
    LOG_RTCERR1(GetSpeakerVolume, level);
    return false;
  }
  *level = ulevel;
  return true;
}

bool WebRtcVoiceEngine::SetOutputVolume(int level) {
  ASSERT(level >= 0 && level <= 255);
  if (voe_wrapper_->volume()->SetSpeakerVolume(level) == -1) {
    LOG_RTCERR1(SetSpeakerVolume, level);
    return false;
  }
  return true;
}

int WebRtcVoiceEngine::GetInputLevel() {
  unsigned int ulevel;
  return (voe_wrapper_->volume()->GetSpeechInputLevel(ulevel) != -1) ?
      static_cast<int>(ulevel) : -1;
}

bool WebRtcVoiceEngine::SetLocalMonitor(bool enable) {
  desired_local_monitor_enable_ = enable;
  return ChangeLocalMonitor(desired_local_monitor_enable_);
}

bool WebRtcVoiceEngine::ChangeLocalMonitor(bool enable) {
  // The voe file api is not available in chrome.
  if (!voe_wrapper_->file()) {
    return false;
  }
  if (enable && !monitor_.get()) {
    monitor_.reset(new WebRtcMonitorStream);
    if (voe_wrapper_->file()->StartRecordingMicrophone(monitor_.get()) == -1) {
      LOG_RTCERR1(StartRecordingMicrophone, monitor_.get());
      // Must call Stop() because there are some cases where Start will report
      // failure but still change the state, and if we leave VE in the on state
      // then it could crash later when trying to invoke methods on our monitor.
      voe_wrapper_->file()->StopRecordingMicrophone();
      monitor_.reset();
      return false;
    }
  } else if (!enable && monitor_.get()) {
    voe_wrapper_->file()->StopRecordingMicrophone();
    monitor_.reset();
  }
  return true;
}

bool WebRtcVoiceEngine::PauseLocalMonitor() {
  return ChangeLocalMonitor(false);
}

bool WebRtcVoiceEngine::ResumeLocalMonitor() {
  return ChangeLocalMonitor(desired_local_monitor_enable_);
}

const std::vector<AudioCodec>& WebRtcVoiceEngine::codecs() {
  return codecs_;
}

bool WebRtcVoiceEngine::FindCodec(const AudioCodec& in) {
  return FindWebRtcCodec(in, NULL);
}

// Get the VoiceEngine codec that matches |in|, with the supplied settings.
bool WebRtcVoiceEngine::FindWebRtcCodec(const AudioCodec& in,
                                        webrtc::CodecInst* out) {
  int ncodecs = voe_wrapper_->codec()->NumOfCodecs();
  for (int i = 0; i < ncodecs; ++i) {
    webrtc::CodecInst voe_codec;
    if (voe_wrapper_->codec()->GetCodec(i, voe_codec) != -1) {
      AudioCodec codec(voe_codec.pltype, voe_codec.plname, voe_codec.plfreq,
                       voe_codec.rate, voe_codec.channels, 0);
      // Allow arbitrary rates for ISAC to be specified.
      if (_stricmp(codec.name.c_str(), kIsacCodecName) == 0) {
        codec.bitrate = 0;
      }
      if (codec.Matches(in)) {
        if (out) {
          // Fixup the payload type.
          voe_codec.pltype = in.id;
          // If ISAC is being used, and an explicit bitrate is not specified,
          // enable auto bandwidth adjustment.
          if (_stricmp(codec.name.c_str(), kIsacCodecName) == 0) {
            voe_codec.rate = (in.bitrate > 0) ? in.bitrate : -1;
          }
          *out = voe_codec;
        }
        return true;
      }
    }
  }
  return false;
}

void WebRtcVoiceEngine::SetLogging(int min_sev, const char* filter) {
  // if min_sev == -1, we keep the current log level.
  if (min_sev >= 0) {
    log_level_ = min_sev;
  }
  log_filter_ = filter;
  ApplyLogging(initialized_ ? log_filter_ : "");
}

int WebRtcVoiceEngine::GetLastEngineError() {
  return voe_wrapper_->error();
}

// We suppport three different logging settings for VoiceEngine:
// 1. Observer callback that goes into talk diagnostic logfile.
//    Use --logfile and --loglevel
//
// 2. Encrypted VoiceEngine log for debugging VoiceEngine.
//    Use --voice_loglevel --voice_logfilter "tracefile file_name"
//
// 3. EC log and dump for debugging QualityEngine.
//    Use --voice_loglevel --voice_logfilter "recordEC file_name"
//
// For more details see: "https://sites.google.com/a/google.com/wavelet/Home/
//    Magic-Flute--RTC-Engine-/Magic-Flute-Command-Line-Parameters"
void WebRtcVoiceEngine::ApplyLogging(const std::string& log_filter) {
  // Set log level.
  int filter = 0;
  switch (log_level_) {
    case talk_base::LS_VERBOSE:
      filter |= webrtc::kTraceAll;      // fall through
    case talk_base::LS_INFO:
      filter |= webrtc::kTraceStateInfo;  // fall through
    case talk_base::LS_WARNING:
      filter |= (webrtc::kTraceInfo | webrtc::kTraceWarning);  // fall through
    case talk_base::LS_ERROR:
      filter |= (webrtc::kTraceError | webrtc::kTraceCritical);
  }
  tracing_->SetTraceFilter(filter);

  // Set encrypted trace file.
  std::vector<std::string> opts;
  talk_base::tokenize(log_filter, ' ', '"', '"', &opts);
  std::vector<std::string>::iterator tracefile =
      std::find(opts.begin(), opts.end(), "tracefile");
  if (tracefile != opts.end() && ++tracefile != opts.end()) {
    // Write encrypted debug output (at same loglevel) to file
    // EncryptedTraceFile no longer supported.
    if (tracing_->SetTraceFile(tracefile->c_str()) == -1) {
      LOG_RTCERR1(SetTraceFile, *tracefile);
    }
  }

  // Set AEC dump file
  std::vector<std::string>::iterator recordEC =
      std::find(opts.begin(), opts.end(), "recordEC");
  if (recordEC != opts.end()) {
    ++recordEC;
    if (recordEC != opts.end() && !is_dumping_aec_) {
      // Start dumping AEC when we are not dumping and recordEC has a filename.
      if (voe_wrapper_->processing()->StartDebugRecording(
          recordEC->c_str()) == -1) {
        LOG_RTCERR0(StartDebugRecording);
      } else {
        is_dumping_aec_ = true;
      }
    } else if (recordEC == opts.end() && is_dumping_aec_) {
      // Stop dumping EC when we are dumping and recordEC has no filename.
      if (voe_wrapper_->processing()->StopDebugRecording() == -1) {
        LOG_RTCERR0(StopDebugRecording);
      }
      is_dumping_aec_ = false;
    }
  }
}

// Ignore spammy trace messages, mostly from the stats API when we haven't
// gotten RTCP info yet from the remote side.
bool WebRtcVoiceEngine::ShouldIgnoreTrace(const std::string& trace) {
  static const char* kTracesToIgnore[] = {
    "\tfailed to GetReportBlockInformation",
    "GetRecCodec() failed to get received codec",
    "GetReceivedRtcpStatistics: Could not get received RTP statistics",
    "GetRemoteRTCPData() failed to measure statistics due to lack of received RTP and/or RTCP packets",  // NOLINT
    "GetRemoteRTCPData() failed to retrieve sender info for remote side",
    "GetRTPStatistics() failed to measure RTT since no RTP packets have been received yet",  // NOLINT
    "GetRTPStatistics() failed to read RTP statistics from the RTP/RTCP module",
    "GetRTPStatistics() failed to retrieve RTT from the RTP/RTCP module",
    "webrtc::RTCPReceiver::SenderInfoReceived No received SR",
    "StatisticsRTP() no statisitics availble",
    NULL
  };
  for (const char* const* p = kTracesToIgnore; *p; ++p) {
    if (trace.find(*p) == 0) {
      return true;
    }
  }
  return false;
}

void WebRtcVoiceEngine::Print(const webrtc::TraceLevel level,
                              const char* trace, const int length) {
  talk_base::LoggingSeverity sev = talk_base::LS_VERBOSE;
  if (level == webrtc::kTraceError || level == webrtc::kTraceCritical)
    sev = talk_base::LS_ERROR;
  else if (level == webrtc::kTraceWarning)
    sev = talk_base::LS_WARNING;
  else if (level == webrtc::kTraceStateInfo || level == webrtc::kTraceInfo)
    sev = talk_base::LS_INFO;

  if (sev >= log_level_) {
    // Skip past boilerplate prefix text
    if (length < 72) {
      std::string msg(trace, length);
      LOG(LS_ERROR) << "Malformed webrtc log message: ";
      LOG_V(sev) << msg;
    } else {
      std::string msg(trace + 71, length - 72);
      if (!ShouldIgnoreTrace(msg)) {
        LOG_V(sev) << "WebRtc:" << msg;
      }
    }
  }
}

void WebRtcVoiceEngine::CallbackOnError(const int channel_num,
                                        const int err_code) {
  talk_base::CritScope lock(&channels_cs_);
  WebRtcVoiceMediaChannel* channel = NULL;
  uint32 ssrc = 0;
  LOG(LS_WARNING) << "VoiceEngine error " << err_code << " reported on channel "
                  << channel_num << ".";
  if (FindChannelAndSsrc(channel_num, &channel, &ssrc)) {
    ASSERT(channel != NULL);
    channel->OnError(ssrc, err_code);
  } else {
    LOG(LS_ERROR) << "VoiceEngine channel " << channel_num
                  << " could not be found in channel list when error reported.";
  }
}

bool WebRtcVoiceEngine::FindChannelAndSsrc(
    int channel_num, WebRtcVoiceMediaChannel** channel, uint32* ssrc) const {
  ASSERT(channel != NULL && ssrc != NULL);

  *channel = NULL;
  *ssrc = 0;
  // Find corresponding channel and ssrc
  for (ChannelList::const_iterator it = channels_.begin();
      it != channels_.end(); ++it) {
    ASSERT(*it != NULL);
    if ((*it)->FindSsrc(channel_num, ssrc)) {
      *channel = *it;
      return true;
    }
  }

  return false;
}

// This method will search through the WebRtcVoiceMediaChannels and
// obtain the voice engine's channel number.
bool WebRtcVoiceEngine::FindChannelNumFromSsrc(
    uint32 ssrc, MediaProcessorDirection direction, int* channel_num) {
  ASSERT(channel_num != NULL);
  ASSERT(direction == MPD_RX || direction == MPD_TX);

  *channel_num = -1;
  // Find corresponding channel for ssrc.
  for (ChannelList::const_iterator it = channels_.begin();
      it != channels_.end(); ++it) {
    ASSERT(*it != NULL);
    uint32 local_ssrc;
    if (direction == MPD_RX) {
      *channel_num = (*it)->GetChannelNum(ssrc);
    }
    if (*channel_num == -1 &&
        voe()->rtp()->GetLocalSSRC((*it)->voe_channel(), local_ssrc) != -1) {
      if (ssrc == local_ssrc) {
        *channel_num = (*it)->voe_channel();
      }
    }
    if (*channel_num != -1) {
      return true;
    }
  }
  LOG(LS_WARNING) << "FindChannelFromSsrc. No Channel Found for Ssrc: " << ssrc;
  return false;
}

void WebRtcVoiceEngine::RegisterChannel(WebRtcVoiceMediaChannel *channel) {
  talk_base::CritScope lock(&channels_cs_);
  channels_.push_back(channel);
}

void WebRtcVoiceEngine::UnregisterChannel(WebRtcVoiceMediaChannel *channel) {
  talk_base::CritScope lock(&channels_cs_);
  ChannelList::iterator i = std::find(channels_.begin(),
                                      channels_.end(),
                                      channel);
  if (i != channels_.end()) {
    channels_.erase(i);
  }
}

void WebRtcVoiceEngine::RegisterSoundclip(WebRtcSoundclipMedia *soundclip) {
  soundclips_.push_back(soundclip);
}

void WebRtcVoiceEngine::UnregisterSoundclip(WebRtcSoundclipMedia *soundclip) {
  SoundclipList::iterator i = std::find(soundclips_.begin(),
                                        soundclips_.end(),
                                        soundclip);
  if (i != soundclips_.end()) {
    soundclips_.erase(i);
  }
}

// Adjusts the default AGC target level by the specified delta.
// NB: If we start messing with other config fields, we'll want
// to save the current webrtc::AgcConfig as well.
bool WebRtcVoiceEngine::AdjustAgcLevel(int delta) {
  webrtc::AgcConfig config = default_agc_config_;
  config.targetLeveldBOv += delta;

  LOG(LS_INFO) << "Adjusting AGC level from default -"
               << default_agc_config_.targetLeveldBOv << "dB to -"
               << config.targetLeveldBOv << "dB";

  if (voe_wrapper_->processing()->SetAgcConfig(config) == -1) {
    LOG_RTCERR1(SetAgcConfig, config.targetLeveldBOv);
    return false;
  }
  return true;
}

// Configures echo cancellation and noise suppression modes according to
// whether or not we are in a multi-point conference.
bool WebRtcVoiceEngine::SetConferenceMode(bool enable) {
// Only use EC_AECM for mobile.
#if defined(IOS) || defined(ANDROID)
  return true;
#endif

  LOG(LS_INFO) << (enable ? "Enabling" : "Disabling")
               << " Conference Mode noise reduction";

  // We always configure noise suppression on, so just toggle the mode.
  const webrtc::NsModes ns_mode = enable ? webrtc::kNsConference
                                         : webrtc::kNsDefault;
  if (voe_wrapper_->processing()->SetNsStatus(true, ns_mode) == -1) {
    LOG_RTCERR2(SetNsStatus, true, ns_mode);
    return false;
  }

  // Echo-cancellation is a user-option, so preserve the enable state and
  // just toggle the mode.
  bool aec;
  webrtc::EcModes ec_mode;
  if (voe_wrapper_->processing()->GetEcStatus(aec, ec_mode) == -1) {
    LOG_RTCERR0(GetEcStatus);
    return false;
  }
  ec_mode = enable ? webrtc::kEcConference : webrtc::kEcDefault;
  if (voe_wrapper_->processing()->SetEcStatus(aec, ec_mode) == -1) {
    LOG_RTCERR2(SetEcStatus, aec, ec_mode);
    return false;
  }
  return true;
}

bool WebRtcVoiceEngine::SetAudioDeviceModule(webrtc::AudioDeviceModule* adm,
    webrtc::AudioDeviceModule* adm_sc) {
  if (initialized_) {
    LOG(LS_WARNING) << "SetAudioDeviceModule can not be called after Init.";
    return false;
  }
  if (adm_) {
    adm_->Release();
    adm_ = NULL;
  }
  if (adm) {
    adm_ = adm;
    adm_->AddRef();
  }

  if (adm_sc_) {
    adm_sc_->Release();
    adm_sc_ = NULL;
  }
  if (adm_sc) {
    adm_sc_ = adm_sc;
    adm_sc_->AddRef();
  }
  return true;
}

bool WebRtcVoiceEngine::RegisterProcessor(
    uint32 ssrc,
    VoiceProcessor* voice_processor,
    MediaProcessorDirection direction) {
  bool register_with_webrtc = false;
  int channel_id = -1;
  bool success = false;
  bool found_channel = FindChannelNumFromSsrc(ssrc, direction, &channel_id);
  if (voice_processor == NULL || !found_channel) {
    LOG(LS_WARNING) << "Media Processing Registration Failed. ssrc: " << ssrc
        << " foundChannel: " << found_channel;
    return false;
  }

  webrtc::ProcessingTypes processing_type;
  {
    talk_base::CritScope cs(&signal_media_critical_);
    if (direction == MPD_RX) {
      processing_type = webrtc::kPlaybackPerChannel;
      if (SignalRxMediaFrame.is_empty()) {
        register_with_webrtc = true;
      }
      SignalRxMediaFrame.connect(voice_processor,
                                 &VoiceProcessor::OnFrame);
    } else {
      processing_type = webrtc::kRecordingPerChannel;
      if (SignalTxMediaFrame.is_empty()) {
        register_with_webrtc = true;
      }
      SignalTxMediaFrame.connect(voice_processor,
                                 &VoiceProcessor::OnFrame);
    }
  }
  if (register_with_webrtc) {
    if (voe()->media() && voe()->media()->
        RegisterExternalMediaProcessing(channel_id,
                                        processing_type,
                                        *this) != -1) {
      LOG(LS_INFO) << "Media Processing Registration Succeeded. channel:"
                   << channel_id;
      success = true;
    } else {
      LOG_RTCERR2(RegisterExternalMediaProcessing,
                  channel_id,
                  processing_type);
      success = false;
    }
  } else {
    // If we don't have to register with the engine, we just needed to
    // connect a new processor, set success to true;
    success = true;
  }
  return success;
}

bool WebRtcVoiceEngine::UnregisterProcessorChannel(
    MediaProcessorDirection channel_direction,
    uint32 ssrc,
    VoiceProcessor* voice_processor,
    MediaProcessorDirection processor_direction) {
  bool success = true;
  FrameSignal* signal;
  webrtc::ProcessingTypes processing_type;
  if (channel_direction == MPD_RX) {
    signal = &SignalRxMediaFrame;
    processing_type = webrtc::kPlaybackPerChannel;
  } else {
    signal = &SignalTxMediaFrame;
    processing_type = webrtc::kRecordingPerChannel;
  }

  int deregister_id = -1;
  {
    talk_base::CritScope cs(&signal_media_critical_);
    if ((processor_direction & channel_direction) != 0 && !signal->is_empty()) {
      signal->disconnect(voice_processor);
      int channel_id = -1;
      bool found_channel = FindChannelNumFromSsrc(ssrc,
                                                  channel_direction,
                                                  &channel_id);
      if (signal->is_empty() && found_channel) {
        deregister_id = channel_id;
      }
    }
  }
  if (deregister_id != -1) {
    if (voe()->media() &&
        voe()->media()->DeRegisterExternalMediaProcessing(deregister_id,
        processing_type) != -1) {
      LOG(LS_INFO) << "Media Processing DeRegistration Succeeded. channel:"
                   << deregister_id;
    } else {
      LOG_RTCERR2(DeRegisterExternalMediaProcessing,
                  deregister_id,
                  processing_type);
      success = false;
    }
  }
  return success;
}

bool WebRtcVoiceEngine::UnregisterProcessor(
    uint32 ssrc,
    VoiceProcessor* voice_processor,
    MediaProcessorDirection direction) {
  bool success = true;
  if (voice_processor == NULL) {
    LOG(LS_WARNING) << "Media Processing Deregistration Failed. ssrc: "
                    << ssrc;
    return false;
  }
  if (!UnregisterProcessorChannel(MPD_RX, ssrc, voice_processor, direction)) {
    success = false;
  }
  if (!UnregisterProcessorChannel(MPD_TX, ssrc, voice_processor, direction)) {
    success = false;
  }
  return success;
}

// Implementing method from WebRtc VoEMediaProcess interface
void WebRtcVoiceEngine::Process(const int channel,
                                const webrtc::ProcessingTypes type,
                                WebRtc_Word16 audio10ms[],
                                const int length,
                                const int sampling_freq,
                                const bool is_stereo) {
  uint32 ssrc;
  WebRtcVoiceMediaChannel* media_channel;

  if (FindChannelAndSsrc(channel, &media_channel, &ssrc)) {
    talk_base::CritScope cs(&signal_media_critical_);
    AudioFrame frame(audio10ms, length, sampling_freq, is_stereo);
    if (type == webrtc::kPlaybackPerChannel) {
      SignalRxMediaFrame(ssrc, MPD_RX, &frame);
    } else if (type == webrtc::kRecordingPerChannel) {
      SignalTxMediaFrame(ssrc, MPD_TX, &frame);
    }
  } else {
    LOG(LS_WARNING) << "MediaProcess Callback invoked with unexpected channel: "
                    << channel;
  }
}

// WebRtcVoiceMediaChannel
WebRtcVoiceMediaChannel::WebRtcVoiceMediaChannel(WebRtcVoiceEngine *engine)
    : WebRtcMediaChannel<VoiceMediaChannel, WebRtcVoiceEngine>(
          engine,
          engine->voe()->base()->CreateChannel()),
      recv_codecs_set_(false),
      options_(0),
      agc_adjusted_(false),
      dtmf_allowed_(false),
      desired_playout_(false),
      playout_(false),
      desired_send_(SEND_NOTHING),
      send_(SEND_NOTHING),
      local_ssrc_(0) {
  engine->RegisterChannel(this);
  LOG(LS_VERBOSE) << "WebRtcVoiceMediaChannel::WebRtcVoiceMediaChannel "
                  << voe_channel();

  // Register external transport
  if (engine->voe()->network()->RegisterExternalTransport(
      voe_channel(), *static_cast<Transport*>(this)) == -1) {
    LOG_RTCERR2(RegisterExternalTransport, voe_channel(), this);
  }

  // Enable RTCP (for quality stats and feedback messages)
  EnableRtcp(voe_channel());

  // Reset all recv codecs; they will be enabled via SetRecvCodecs.
  ResetRecvCodecs(voe_channel());
}

WebRtcVoiceMediaChannel::~WebRtcVoiceMediaChannel() {
  LOG(LS_VERBOSE) << "WebRtcVoiceMediaChannel::~WebRtcVoiceMediaChannel "
                  << voe_channel();

  // DeRegister external transport
  if (engine()->voe()->network()->DeRegisterExternalTransport(
      voe_channel()) == -1) {
    LOG_RTCERR1(DeRegisterExternalTransport, voe_channel());
  }

  // Unregister ourselves from the engine.
  engine()->UnregisterChannel(this);
  // Remove any remaining streams.
  while (!mux_channels_.empty()) {
    RemoveRecvStream(mux_channels_.begin()->first);
  }

  // Delete the primary channel.
  if (engine()->voe()->base()->DeleteChannel(voe_channel()) == -1) {
    LOG_RTCERR1(DeleteChannel, voe_channel());
  }
}

bool WebRtcVoiceMediaChannel::SetOptions(int flags) {
  // Always accept flags that are unchanged.
  if (options_ == flags) {
    return true;
  }

  // Reject new options if we're already sending.
  if (send_ != SEND_NOTHING) {
    return false;
  }

  // Save the options, to be interpreted where appropriate.
  options_ = flags;
  return true;
}

bool WebRtcVoiceMediaChannel::SetRecvCodecs(
    const std::vector<AudioCodec>& codecs) {
  // Set the payload types to be used for incoming media.
  bool ret = true;
  LOG(LS_INFO) << "Setting receive voice codecs:";
  for (std::vector<AudioCodec>::const_iterator it = codecs.begin();
       it != codecs.end() && ret; ++it) {
    webrtc::CodecInst voe_codec;
    if (engine()->FindWebRtcCodec(*it, &voe_codec)) {
      LOG(LS_INFO) << ToString(*it);
      voe_codec.pltype = it->id;
      if (engine()->voe()->codec()->SetRecPayloadType(
          voe_channel(), voe_codec) == -1) {
        LOG_RTCERR2(SetRecPayloadType, voe_channel(), ToString(voe_codec));
        ret = false;
      }
      // Set the receive codecs on all receiving channels.
      for (ChannelMap::iterator it = mux_channels_.begin();
           it != mux_channels_.end() && ret; ++it) {
        if (engine()->voe()->codec()->SetRecPayloadType(
            it->second, voe_codec) == -1) {
          LOG_RTCERR2(SetRecPayloadType, it->second, ToString(voe_codec));
          ret = false;
        }
      }
    } else {
      LOG(LS_WARNING) << "Unknown codec " << ToString(*it);
      ret = false;
    }
  }
  recv_codecs_set_ = ret;
  return ret;
}

bool WebRtcVoiceMediaChannel::SetSendCodecs(
    const std::vector<AudioCodec>& codecs) {
  // Disable DTMF, VAD, and FEC unless we know the other side wants them.
  dtmf_allowed_ = false;
  engine()->voe()->codec()->SetVADStatus(voe_channel(), false);
  engine()->voe()->rtp()->SetFECStatus(voe_channel(), false);

  // Scan through the list to figure out the codec to use for sending, along
  // with the proper configuration for VAD and DTMF.
  bool first = true;
  webrtc::CodecInst send_codec;
  memset(&send_codec, 0, sizeof(send_codec));

  for (std::vector<AudioCodec>::const_iterator it = codecs.begin();
       it != codecs.end(); ++it) {
    // Ignore codecs we don't know about. The negotiation step should prevent
    // this, but double-check to be sure.
    webrtc::CodecInst voe_codec;
    if (!engine()->FindWebRtcCodec(*it, &voe_codec)) {
      LOG(LS_WARNING) << "Unknown codec " << ToString(voe_codec);
      continue;
    }

    // Find the DTMF telephone event "codec" and tell VoiceEngine about it.
    if (_stricmp(it->name.c_str(), "telephone-event") == 0 ||
        _stricmp(it->name.c_str(), "audio/telephone-event") == 0) {
      engine()->voe()->dtmf()->SetSendTelephoneEventPayloadType(
          voe_channel(), it->id);
      dtmf_allowed_ = true;
    }

    // Turn voice activity detection/comfort noise on if supported.
    // Set the wideband CN payload type appropriately.
    // (narrowband always uses the static payload type 13).
    if (_stricmp(it->name.c_str(), "CN") == 0) {
      webrtc::PayloadFrequencies cn_freq;
      switch (it->clockrate) {
        case 8000:
          cn_freq = webrtc::kFreq8000Hz;
          break;
        case 16000:
          cn_freq = webrtc::kFreq16000Hz;
          break;
        case 32000:
          cn_freq = webrtc::kFreq32000Hz;
          break;
        default:
          LOG(LS_WARNING) << "CN frequency " << it->clockrate
                          << " not supported.";
          continue;
      }
      engine()->voe()->codec()->SetVADStatus(voe_channel(), true);
      if (cn_freq != webrtc::kFreq8000Hz) {
        engine()->voe()->codec()->SetSendCNPayloadType(voe_channel(),
                                                       it->id, cn_freq);
      }
    }

    // We'll use the first codec in the list to actually send audio data.
    // Be sure to use the payload type requested by the remote side.
    // "red", for FEC audio, is a special case where the actual codec to be
    // used is specified in params.
    if (first) {
      if (_stricmp(it->name.c_str(), "red") == 0) {
        // Parse out the RED parameters. If we fail, just ignore RED;
        // we don't support all possible params/usage scenarios.
        if (!GetRedSendCodec(*it, codecs, &send_codec)) {
          continue;
        }

        // Enable redundant encoding of the specified codec. Treat any
        // failure as a fatal internal error.
        LOG(LS_INFO) << "Enabling FEC";
        if (engine()->voe()->rtp()->SetFECStatus(voe_channel(),
                                                 true, it->id) == -1) {
          LOG_RTCERR3(SetFECStatus, voe_channel(), true, it->id);
          return false;
        }
      } else {
        send_codec = voe_codec;
      }
      first = false;
    }
  }

  // If we're being asked to set an empty list of codecs, due to a buggy client,
  // choose the most common format: PCMU
  if (first) {
    LOG(LS_WARNING) << "Received empty list of codecs; using PCMU/8000";
    AudioCodec codec(0, "PCMU", 8000, 0, 1, 0);
    engine()->FindWebRtcCodec(codec, &send_codec);
  }

  // Set the codec.
  LOG(LS_INFO) << "Selected voice codec " << ToString(send_codec)
               << ", bitrate=" << send_codec.rate;
  if (engine()->voe()->codec()->SetSendCodec(voe_channel(),
                                             send_codec) == -1) {
    LOG_RTCERR2(SetSendCodec, voe_channel(), ToString(send_codec));
    return false;
  }

  send_codec_.reset(new webrtc::CodecInst(send_codec));
  return true;
}

bool WebRtcVoiceMediaChannel::SetRecvRtpHeaderExtensions(
    const std::vector<RtpHeaderExtension>& extensions) {
  // We don't support any incoming extensions headers right now.
  return true;
}

bool WebRtcVoiceMediaChannel::SetSendRtpHeaderExtensions(
    const std::vector<RtpHeaderExtension>& extensions) {
  // Enable the audio level extension header if requested.
  std::vector<RtpHeaderExtension>::const_iterator it;
  for (it = extensions.begin(); it != extensions.end(); ++it) {
    if (it->uri == kRtpAudioLevelHeaderExtension) {
      break;
    }
  }

  bool enable = (it != extensions.end());
  int id = 0;

  if (enable) {
    id = it->id;
    if (id < kMinRtpHeaderExtensionId ||
        id > kMaxRtpHeaderExtensionId) {
      LOG(LS_WARNING) << "Invalid RTP header extension id " << id;
      return false;
    }
  }

// This api call is not available in iOS version of VoiceEngine currently.
#if !defined(IOS) && !defined(ANDROID)
  if (engine()->voe()->rtp()->SetRTPAudioLevelIndicationStatus(
      voe_channel(), enable, id) == -1) {
    LOG_RTCERR3(SetRTPAudioLevelIndicationStatus, voe_channel(), enable, id);
    return false;
  }
#endif

  return true;
}

bool WebRtcVoiceMediaChannel::SetPlayout(bool playout) {
  desired_playout_ = playout;
  return ChangePlayout(desired_playout_);
}

bool WebRtcVoiceMediaChannel::PausePlayout() {
  return ChangePlayout(false);
}

bool WebRtcVoiceMediaChannel::ResumePlayout() {
  return ChangePlayout(desired_playout_);
}

bool WebRtcVoiceMediaChannel::ChangePlayout(bool playout) {
  if (playout_ == playout) {
    return true;
  }

  bool result = true;
  if (mux_channels_.empty()) {
    // Only toggle the default channel if we don't have any other channels.
    result = SetPlayout(voe_channel(), playout);
  }
  for (ChannelMap::iterator it = mux_channels_.begin();
       it != mux_channels_.end() && result; ++it) {
    if (!SetPlayout(it->second, playout)) {
      LOG(LS_ERROR) << "SetPlayout " << playout << " on channel " << it->second
                    << " failed";
      result = false;
    }
  }

  if (result) {
    playout_ = playout;
  }
  return result;
}

bool WebRtcVoiceMediaChannel::SetSend(SendFlags send) {
  desired_send_ = send;
  if (local_ssrc_ != 0)
    return ChangeSend(desired_send_);
  return true;
}

bool WebRtcVoiceMediaChannel::PauseSend() {
  return ChangeSend(SEND_NOTHING);
}

bool WebRtcVoiceMediaChannel::ResumeSend() {
  return ChangeSend(desired_send_);
}

bool WebRtcVoiceMediaChannel::ChangeSend(SendFlags send) {
  if (send_ == send) {
    return true;
  }

  if (send == SEND_MICROPHONE) {
#ifdef CHROMEOS
    // Conference mode doesn't work well on ChromeOS.
    if (!engine()->SetConferenceMode(false)) {
      LOG_RTCERR1(SetConferenceMode, voe_channel());
      return false;
    }
#else
    // Multi-point conferences use conference-mode noise filtering.
    if (!engine()->SetConferenceMode(
        0 != (options_ & OPT_CONFERENCE))) {
      LOG_RTCERR1(SetConferenceMode, voe_channel());
      return false;
    }
#endif  // CHROMEOS

    if ((options_ & OPT_AGC_MINUS_10DB) && !agc_adjusted_) {
      if (engine()->AdjustAgcLevel(kMinus10DbAdjustment)) {
        agc_adjusted_ = true;
      }
    }

    // VoiceEngine resets sequence number when StopSend is called. This
    // sometimes causes libSRTP to complain about packets being
    // replayed. To get around this we store the last sent sequence
    // number and initializes the channel with the next to continue on
    // the same sequence.
    if (sequence_number() != -1) {
      LOG(LS_INFO) << "WebRtcVoiceMediaChannel restores seqnum="
                   << sequence_number() + 1;
      if (engine()->voe()->sync()->SetInitSequenceNumber(
              voe_channel(), sequence_number() + 1) == -1) {
        LOG_RTCERR2(SetInitSequenceNumber, voe_channel(),
                    sequence_number() + 1);
      }
    }
    if (engine()->voe()->base()->StartSend(voe_channel()) == -1) {
      LOG_RTCERR1(StartSend, voe_channel());
      return false;
    }
    // It's OK not to have file() here, since we don't need to call Stop if
    // no file is playing.
    if (engine()->voe()->file() &&
        engine()->voe()->file()->StopPlayingFileAsMicrophone(
            voe_channel()) == -1) {
      LOG_RTCERR1(StopPlayingFileAsMicrophone, voe_channel());
      return false;
    }
  } else if (send == SEND_RINGBACKTONE) {
    ASSERT(ringback_tone_.get() != NULL);
    if (!ringback_tone_.get()) {
      return false;
    }
    if (engine()->voe()->file() &&
        engine()->voe()->file()->StartPlayingFileAsMicrophone(
        voe_channel(), ringback_tone_.get(), false) != -1) {
      LOG(LS_INFO) << "File StartPlayingFileAsMicrophone Succeeded. channel:"
                   << voe_channel();
    } else {
      LOG_RTCERR3(StartPlayingFileAsMicrophone, voe_channel(),
                  ringback_tone_.get(), false);
      return false;
    }
    // VoiceEngine resets sequence number when StopSend is called. This
    // sometimes causes libSRTP to complain about packets being
    // replayed. To get around this we store the last sent sequence
    // number and initializes the channel with the next to continue on
    // the same sequence.
    if (sequence_number() != -1) {
      LOG(LS_INFO) << "WebRtcVoiceMediaChannel restores seqnum="
                   << sequence_number() + 1;
      if (engine()->voe()->sync()->SetInitSequenceNumber(
              voe_channel(), sequence_number() + 1) == -1) {
        LOG_RTCERR2(SetInitSequenceNumber, voe_channel(),
                    sequence_number() + 1);
      }
    }
    if (engine()->voe()->base()->StartSend(voe_channel()) == -1) {
      LOG_RTCERR1(StartSend, voe_channel());
      return false;
    }
  } else {  // SEND_NOTHING
    if (engine()->voe()->base()->StopSend(voe_channel()) == -1) {
      LOG_RTCERR1(StopSend, voe_channel());
    }

    // Reset the AGC level, if it was set.
    if (agc_adjusted_) {
      if (engine()->AdjustAgcLevel(0)) {
        agc_adjusted_ = false;
      }
    }

    // Disable conference-mode noise filtering.
    if (!engine()->SetConferenceMode(false)) {
      LOG_RTCERR1(SetConferenceMode, voe_channel());
    }
  }
  send_ = send;
  return true;
}

bool WebRtcVoiceMediaChannel::AddSendStream(const StreamParams& sp) {
  if (local_ssrc_ != 0) {
    LOG(LS_ERROR) << "WebRtcVoiceMediaChannel supports one sending channel.";
    return false;
  }

  if (engine()->voe()->rtp()->SetLocalSSRC(voe_channel(), sp.first_ssrc())
        == -1) {
    LOG_RTCERR2(SetSendSSRC, voe_channel(), sp.first_ssrc());
    return false;
  }
  // Set the SSRC on the receive channels.
  // Receive channels have to have the same SSRC in order to send receiver
  // reports with this SSRC.
  for (ChannelMap::const_iterator it = mux_channels_.begin();
       it != mux_channels_.end(); ++it) {
    int channel_id = it->second;
    if (engine()->voe()->rtp()->SetLocalSSRC(channel_id,
                                             sp.first_ssrc()) != 0) {
      LOG_RTCERR1(SetLocalSSRC, it->first);
      return false;
    }
  }

  if (engine()->voe()->rtp()->SetRTCP_CNAME(voe_channel(),
                                            sp.cname.c_str()) == -1) {
     LOG_RTCERR2(SetRTCP_CNAME, voe_channel(), sp.cname);
     return false;
  }

  local_ssrc_ = sp.first_ssrc();
  if (desired_send_ != send_)
    return ChangeSend(desired_send_);
  return true;
}

bool WebRtcVoiceMediaChannel::RemoveSendStream(uint32 ssrc) {
  if (ssrc != local_ssrc_) {
    return false;
  }
  local_ssrc_ = 0;
  ChangeSend(SEND_NOTHING);
  return true;
}

bool WebRtcVoiceMediaChannel::AddRecvStream(const StreamParams& sp) {
  talk_base::CritScope lock(&mux_channels_cs_);

  // Reuse default channel for recv stream in 1:1 call.
  if ((options_ & OPT_CONFERENCE) == 0) {
    LOG(LS_INFO) << "Recv stream " << sp.first_ssrc()
                 << " reuse default channel";
    return true;
  }

  if (!VERIFY(sp.ssrcs.size() == 1))
    return false;
  uint32 ssrc = sp.first_ssrc();

  if (mux_channels_.find(ssrc) != mux_channels_.end()) {
    return false;
  }

  // Create a new channel for receiving audio data.
  int channel = engine()->voe()->base()->CreateChannel();
  if (channel == -1) {
    LOG_RTCERR0(CreateChannel);
    return false;
  }

  // Configure to use external transport, like our default channel.
  if (engine()->voe()->network()->RegisterExternalTransport(
          channel, *this) == -1) {
    LOG_RTCERR2(SetExternalTransport, channel, this);
    return false;
  }

  // Use the same SSRC as our default channel (so the RTCP reports are correct).
  unsigned int send_ssrc;
  webrtc::VoERTP_RTCP* rtp = engine()->voe()->rtp();
  if (rtp->GetLocalSSRC(voe_channel(), send_ssrc) == -1) {
    LOG_RTCERR2(GetSendSSRC, channel, send_ssrc);
    return false;
  }
  if (rtp->SetLocalSSRC(channel, send_ssrc) == -1) {
    LOG_RTCERR2(SetSendSSRC, channel, send_ssrc);
    return false;
  }

  // Use the same recv payload types as our default channel.
  ResetRecvCodecs(channel);
  if (recv_codecs_set_) {
    int ncodecs = engine()->voe()->codec()->NumOfCodecs();
    for (int i = 0; i < ncodecs; ++i) {
      webrtc::CodecInst voe_codec;
      if (engine()->voe()->codec()->GetCodec(i, voe_codec) != -1) {
        voe_codec.rate = 0;  // Needed to make GetRecPayloadType work for ISAC
        if (engine()->voe()->codec()->GetRecPayloadType(
            voe_channel(), voe_codec) != -1) {
          if (engine()->voe()->codec()->SetRecPayloadType(
              channel, voe_codec) == -1) {
            LOG_RTCERR2(SetRecPayloadType, channel, ToString(voe_codec));
            return false;
          }
        }
      }
    }
  }

  if (mux_channels_.empty() && playout_) {
    // This is the first stream in a multi user meeting. We can now
    // disable playback of the default stream. This since the default
    // stream will probably have received some initial packets before
    // the new stream was added. This will mean that the CN state from
    // the default channel will be mixed in with the other streams
    // throughout the whole meeting, which might be disturbing.
    LOG(LS_INFO) << "Disabling playback on the default voice channel";
    SetPlayout(voe_channel(), false);
  }

  mux_channels_[ssrc] = channel;

  // TODO: We should rollback the add if SetPlayout fails.
  LOG(LS_INFO) << "New audio stream " << ssrc
            << " registered to VoiceEngine channel #"
            << channel << ".";
  return SetPlayout(channel, playout_);
}

bool WebRtcVoiceMediaChannel::RemoveRecvStream(uint32 ssrc) {
  talk_base::CritScope lock(&mux_channels_cs_);
  ChannelMap::iterator it = mux_channels_.find(ssrc);

  if (it != mux_channels_.end()) {
    if (engine()->voe()->network()->DeRegisterExternalTransport(
        it->second) == -1) {
      LOG_RTCERR1(DeRegisterExternalTransport, it->second);
    }

    LOG(LS_INFO) << "Removing audio stream " << ssrc
              << " with VoiceEngine channel #"
              << it->second << ".";
    if (engine()->voe()->base()->DeleteChannel(it->second) == -1) {
      LOG_RTCERR1(DeleteChannel, voe_channel());
      return false;
    }

    mux_channels_.erase(it);
    if (mux_channels_.empty() && playout_) {
      // The last stream was removed. We can now enable the default
      // channel for new channels to be played out immediately without
      // waiting for AddStream messages.
      // TODO: Does the default channel still have it's CN state?
      LOG(LS_INFO) << "Enabling playback on the default voice channel";
      SetPlayout(voe_channel(), true);
    }
  }
  return true;
}

bool WebRtcVoiceMediaChannel::GetActiveStreams(
    AudioInfo::StreamList* actives) {
  actives->clear();
  for (ChannelMap::iterator it = mux_channels_.begin();
       it != mux_channels_.end(); ++it) {
    int level = GetOutputLevel(it->second);
    if (level > 0) {
      actives->push_back(std::make_pair(it->first, level));
    }
  }
  return true;
}

int WebRtcVoiceMediaChannel::GetOutputLevel() {
  // return the highest output level of all streams
  int highest = GetOutputLevel(voe_channel());
  for (ChannelMap::iterator it = mux_channels_.begin();
       it != mux_channels_.end(); ++it) {
    int level = GetOutputLevel(it->second);
    highest = talk_base::_max(level, highest);
  }
  return highest;
}

bool WebRtcVoiceMediaChannel::SetOutputScaling(
    uint32 ssrc, double left, double right) {
  talk_base::CritScope lock(&mux_channels_cs_);
  // Collect the channels to scale the output volume.
  std::vector<int> channels;
  if (0 == ssrc) {  // Collect all channels, including the default one.
    channels.push_back(voe_channel());
    for (ChannelMap::const_iterator it = mux_channels_.begin();
        it != mux_channels_.end(); ++it) {
      channels.push_back(it->second);
    }
  } else {  // Collect only the channel of the specified ssrc.
    int channel = GetChannelNum(ssrc);
    if (-1 == channel) {
      LOG(LS_WARNING) << "Cannot find channel for ssrc:" << ssrc;
      return false;
    }
    channels.push_back(channel);
  }

  // Scale the output volume for the collected channels. We first normalize to
  // scale the volume and then set the left and right pan.
  float scale = static_cast<float>(talk_base::_max(left, right));
  if (scale > 0.0001f) {
    left /= scale;
    right /= scale;
  }
  for (std::vector<int>::const_iterator it = channels.begin();
      it != channels.end(); ++it) {
    if (-1 == engine()->voe()->volume()->SetChannelOutputVolumeScaling(
        *it, scale)) {
      LOG_RTCERR2(SetChannelOutputVolumeScaling, *it, scale);
      return false;
    }
    if (-1 == engine()->voe()->volume()->SetOutputVolumePan(
        *it, static_cast<float>(left), static_cast<float>(right))) {
      LOG_RTCERR3(SetOutputVolumePan, *it, left, right);
      // Do not return if fails. SetOutputVolumePan is not available for all
      // pltforms.
    }
    LOG(LS_INFO) << "SetOutputScaling to left=" << left * scale
                 << " right=" << right * scale
                 << " for channel " << *it << " and ssrc " << ssrc;
  }
  return true;
}

bool WebRtcVoiceMediaChannel::GetOutputScaling(
    uint32 ssrc, double* left, double* right) {
  if (!left || !right) return false;

  talk_base::CritScope lock(&mux_channels_cs_);
  // Determine which channel based on ssrc.
  int channel = (0 == ssrc) ? voe_channel() : GetChannelNum(ssrc);
  if (channel == -1) {
    LOG(LS_WARNING) << "Cannot find channel for ssrc:" << ssrc;
    return false;
  }

  float scaling;
  if (-1 == engine()->voe()->volume()->GetChannelOutputVolumeScaling(
      channel, scaling)) {
    LOG_RTCERR2(GetChannelOutputVolumeScaling, channel, scaling);
    return false;
  }

  float left_pan;
  float right_pan;
  if (-1 == engine()->voe()->volume()->GetOutputVolumePan(
      channel, left_pan, right_pan)) {
    LOG_RTCERR3(GetOutputVolumePan, channel, left_pan, right_pan);
    // If GetOutputVolumePan fails, we use the default left and right pan.
    left_pan = 1.0f;
    right_pan = 1.0f;
  }

  *left = scaling * left_pan;
  *right = scaling * right_pan;
  return true;
}

bool WebRtcVoiceMediaChannel::SetRingbackTone(const char *buf, int len) {
  ringback_tone_.reset(new WebRtcSoundclipStream(buf, len));
  return true;
}

bool WebRtcVoiceMediaChannel::PlayRingbackTone(uint32 ssrc,
                                             bool play, bool loop) {
  if (!ringback_tone_.get()) {
    return false;
  }

  // The voe file api is not available in chrome.
  if (!engine()->voe()->file()) {
    return false;
  }

  // Determine which VoiceEngine channel to play on.
  int channel = (ssrc == 0) ? voe_channel() : GetChannelNum(ssrc);
  if (channel == -1) {
    return false;
  }

  // Make sure the ringtone is cued properly, and play it out.
  if (play) {
    ringback_tone_->set_loop(loop);
    ringback_tone_->Rewind();
    if (engine()->voe()->file()->StartPlayingFileLocally(channel,
        ringback_tone_.get()) == -1) {
      LOG_RTCERR2(StartPlayingFileLocally, channel, ringback_tone_.get());
      LOG(LS_ERROR) << "Unable to start ringback tone";
      return false;
    }
    ringback_channels_.insert(channel);
    LOG(LS_INFO) << "Started ringback on channel " << channel;
  } else {
    if (engine()->voe()->file()->IsPlayingFileLocally(channel) == 1 &&
        engine()->voe()->file()->StopPlayingFileLocally(channel) == -1) {
      LOG_RTCERR1(StopPlayingFileLocally, channel);
      return false;
    }
    LOG(LS_INFO) << "Stopped ringback on channel " << channel;
    ringback_channels_.erase(channel);
  }

  return true;
}

bool WebRtcVoiceMediaChannel::PressDTMF(int event, bool playout) {
  if (!dtmf_allowed_) {
    return false;
  }

  // Enable or disable DTMF playout of this tone as requested. This will linger
  // until the next call to this method, but that's OK.
  if (engine()->voe()->dtmf()->SetDtmfFeedbackStatus(playout) == -1) {
    LOG_RTCERR2(SendDTMF, voe_channel(), playout);
    return false;
  }

  // Send DTMF using out-of-band DTMF. ("true", as 3rd arg)
  if (engine()->voe()->dtmf()->SendTelephoneEvent(voe_channel(), event,
      true) == -1) {
    LOG_RTCERR3(SendDTMF, voe_channel(), event, true);
    return false;
  }

  return true;
}

void WebRtcVoiceMediaChannel::OnPacketReceived(talk_base::Buffer* packet) {
  // Pick which channel to send this packet to. If this packet doesn't match
  // any multiplexed streams, just send it to the default channel. Otherwise,
  // send it to the specific decoder instance for that stream.
  int which_channel = GetChannelNum(
      ParseSsrc(packet->data(), packet->length(), false));
  if (which_channel == -1) {
    which_channel = voe_channel();
  }

  // Stop any ringback that might be playing on the channel.
  // It's possible the ringback has already stopped, ih which case we'll just
  // use the opportunity to remove the channel from ringback_channels_.
  if (engine()->voe()->file()) {
    const std::set<int>::iterator it = ringback_channels_.find(which_channel);
    if (it != ringback_channels_.end()) {
      if (engine()->voe()->file()->IsPlayingFileLocally(
          which_channel) == 1) {
        engine()->voe()->file()->StopPlayingFileLocally(which_channel);
        LOG(LS_INFO) << "Stopped ringback on channel " << which_channel
                     << " due to incoming media";
      }
      ringback_channels_.erase(which_channel);
    }
  }

  // Pass it off to the decoder.
  engine()->voe()->network()->ReceivedRTPPacket(which_channel,
                                                   packet->data(),
                                                   packet->length());
}

void WebRtcVoiceMediaChannel::OnRtcpReceived(talk_base::Buffer* packet) {
  // See above.
  int which_channel = GetChannelNum(
      ParseSsrc(packet->data(), packet->length(), true));
  if (which_channel == -1) {
    which_channel = voe_channel();
  }

  engine()->voe()->network()->ReceivedRTCPPacket(which_channel,
                                                    packet->data(),
                                                    packet->length());
}

bool WebRtcVoiceMediaChannel::Mute(bool muted) {
  if (engine()->voe()->volume()->SetInputMute(voe_channel(),
      muted) == -1) {
    LOG_RTCERR2(SetInputMute, voe_channel(), muted);
    return false;
  }
  return true;
}

bool WebRtcVoiceMediaChannel::GetStats(VoiceMediaInfo* info) {
  // In VoiceEngine 3.5, GetRTCPStatistics will return 0 even when it fails,
  // causing the stats to contain garbage information. To prevent this, we
  // zero the stats structure before calling this API.
  // TODO: Remove this workaround.
  webrtc::CallStatistics cs;
  unsigned int ssrc;
  webrtc::CodecInst codec;
  unsigned int level;

  // Fill in the sender info, based on what we know, and what the
  // remote side told us it got from its RTCP report.
  VoiceSenderInfo sinfo;

  // Data we obtain locally.
  memset(&cs, 0, sizeof(cs));
  if (engine()->voe()->rtp()->GetRTCPStatistics(voe_channel(), cs) == -1 ||
      engine()->voe()->rtp()->GetLocalSSRC(voe_channel(), ssrc) == -1) {
    return false;
  }

  sinfo.ssrc = ssrc;
  sinfo.codec_name = send_codec_.get() ? send_codec_->plname : "";
  sinfo.bytes_sent = cs.bytesSent;
  sinfo.packets_sent = cs.packetsSent;
  // RTT isn't known until a RTCP report is received. Until then, VoiceEngine
  // returns 0 to indicate an error value.
  sinfo.rtt_ms = (cs.rttMs > 0) ? cs.rttMs : -1;

  // Data from the last remote RTCP report.
  unsigned int ntp_high, ntp_low, timestamp, ptimestamp, jitter;
  unsigned short loss;  // NOLINT
  if (engine()->voe()->rtp()->GetRemoteRTCPData(voe_channel(),
          ntp_high, ntp_low, timestamp, ptimestamp, &jitter, &loss) != -1 &&
      engine()->voe()->codec()->GetSendCodec(voe_channel(),
          codec) != -1) {
    // Convert Q8 to floating point.
    sinfo.fraction_lost = static_cast<float>(loss) / (1 << 8);
    // Convert samples to milliseconds.
    if (codec.plfreq / 1000 > 0) {
      sinfo.jitter_ms = jitter / (codec.plfreq / 1000);
    }
  } else {
    sinfo.fraction_lost = -1;
    sinfo.jitter_ms = -1;
  }
  // TODO: Figure out how to get remote packets_lost, ext_seqnum
  sinfo.packets_lost = -1;
  sinfo.ext_seqnum = -1;

  // Local speech level.
  sinfo.audio_level = (engine()->voe()->volume()->
      GetSpeechInputLevelFullRange(level) != -1) ? level : -1;

  bool echo_metrics_on = false;
  // These can take on valid negative values, so use the lowest possible level
  // as default rather than -1.
  sinfo.echo_return_loss = -100;
  sinfo.echo_return_loss_enhancement = -100;
  // These can also be negative, but in practice -1 is only used to signal
  // insufficient data, since the resolution is limited to multiples of 4 ms.
  sinfo.echo_delay_median_ms = -1;
  sinfo.echo_delay_std_ms = -1;
  if (engine()->voe()->processing()->GetEcMetricsStatus(echo_metrics_on) !=
      -1 && echo_metrics_on) {
    // TODO: we may want to use VoECallReport::GetEchoMetricsSummary
    // here, but it appears to be unsuitable currently. Revisit after this is
    // investigated: http://b/issue?id=5666755
    int erl, erle, rerl, anlp;
    if (engine()->voe()->processing()->GetEchoMetrics(erl, erle, rerl, anlp) !=
        -1) {
      sinfo.echo_return_loss = erl;
      sinfo.echo_return_loss_enhancement = erle;
    }

    int median, std;
    if (engine()->voe()->processing()->GetEcDelayMetrics(median, std) != -1) {
      sinfo.echo_delay_median_ms = median;
      sinfo.echo_delay_std_ms = std;
    }
  }

  info->senders.push_back(sinfo);

  // Build the list of receivers, one for each mux channel, or 1 in a 1:1 call.
  std::vector<int> channels;
  for (ChannelMap::const_iterator it = mux_channels_.begin();
       it != mux_channels_.end(); ++it) {
    channels.push_back(it->second);
  }
  if (channels.empty()) {
    channels.push_back(voe_channel());
  }

  // Get the SSRC and stats for each receiver, based on our own calculations.
  for (std::vector<int>::const_iterator it = channels.begin();
       it != channels.end(); ++it) {
    memset(&cs, 0, sizeof(cs));
    if (engine()->voe()->rtp()->GetRemoteSSRC(*it, ssrc) != -1 &&
        engine()->voe()->rtp()->GetRTCPStatistics(*it, cs) != -1 &&
        engine()->voe()->codec()->GetRecCodec(*it, codec) != -1) {
      VoiceReceiverInfo rinfo;
      rinfo.ssrc = ssrc;
      rinfo.bytes_rcvd = cs.bytesReceived;
      rinfo.packets_rcvd = cs.packetsReceived;
      // The next four fields are from the most recently sent RTCP report.
      // Convert Q8 to floating point.
      rinfo.fraction_lost = static_cast<float>(cs.fractionLost) / (1 << 8);
      rinfo.packets_lost = cs.cumulativeLost;
      rinfo.ext_seqnum = cs.extendedMax;
      // Convert samples to milliseconds.
      if (codec.plfreq / 1000 > 0) {
        rinfo.jitter_ms = cs.jitterSamples / (codec.plfreq / 1000);
      }

      // Get jitter buffer and total delay (alg + jitter + playout) stats.
      webrtc::NetworkStatistics ns;
      if (engine()->voe()->neteq() &&
          engine()->voe()->neteq()->GetNetworkStatistics(
              *it, ns) != -1) {
        rinfo.jitter_buffer_ms = ns.currentBufferSize;
        rinfo.jitter_buffer_preferred_ms = ns.preferredBufferSize;
      }
      if (engine()->voe()->sync()) {
        engine()->voe()->sync()->GetDelayEstimate(*it,
            rinfo.delay_estimate_ms);
      }

      // Get speech level.
      rinfo.audio_level = (engine()->voe()->volume()->
          GetSpeechOutputLevelFullRange(*it, level) != -1) ? level : -1;
      info->receivers.push_back(rinfo);
    }
  }

  return true;
}

void WebRtcVoiceMediaChannel::GetLastMediaError(
    uint32* ssrc, VoiceMediaChannel::Error* error) {
  ASSERT(ssrc != NULL);
  ASSERT(error != NULL);
  FindSsrc(voe_channel(), ssrc);
  *error = WebRtcErrorToChannelError(GetLastEngineError());
}

bool WebRtcVoiceMediaChannel::FindSsrc(int channel_num, uint32* ssrc) {
  talk_base::CritScope lock(&mux_channels_cs_);
  ASSERT(ssrc != NULL);
  if (channel_num == voe_channel()) {
    unsigned local_ssrc = 0;
    // This is a sending channel.
    if (engine()->voe()->rtp()->GetLocalSSRC(
        channel_num, local_ssrc) != -1) {
      *ssrc = local_ssrc;
    }
    return true;
  } else if (channel_num == -1 && send_ != SEND_NOTHING) {
    // Sometimes the VoiceEngine core will throw error with channel_num = -1.
    // This means the error is not limited to a specific channel.  Signal the
    // message using ssrc=0.  If the current channel is sending, use this
    // channel for sending the message.
    *ssrc = 0;
    return true;
  } else {
    // Check whether this is a receiving channel.
    for (ChannelMap::const_iterator it = mux_channels_.begin();
        it != mux_channels_.end(); ++it) {
      if (it->second == channel_num) {
        *ssrc = it->first;
        return true;
      }
    }
  }
  return false;
}

void WebRtcVoiceMediaChannel::OnError(uint32 ssrc, int error) {
  SignalMediaError(ssrc, WebRtcErrorToChannelError(error));
}

int WebRtcVoiceMediaChannel::GetOutputLevel(int channel) {
  unsigned int ulevel;
  int ret =
      engine()->voe()->volume()->GetSpeechOutputLevel(channel, ulevel);
  return (ret == 0) ? static_cast<int>(ulevel) : -1;
}

int WebRtcVoiceMediaChannel::GetChannelNum(uint32 ssrc) {
  ChannelMap::iterator it = mux_channels_.find(ssrc);
  return (it != mux_channels_.end()) ? it->second : -1;
}

bool WebRtcVoiceMediaChannel::GetRedSendCodec(const AudioCodec& red_codec,
    const std::vector<AudioCodec>& all_codecs, webrtc::CodecInst* send_codec) {
  // Get the RED encodings from the parameter with no name. This may
  // change based on what is discussed on the Jingle list.
  // The encoding parameter is of the form "a/b"; we only support where
  // a == b. Verify this and parse out the value into red_pt.
  // If the parameter value is absent (as it will be until we wire up the
  // signaling of this message), use the second codec specified (i.e. the
  // one after "red") as the encoding parameter.
  int red_pt = -1;
  std::string red_params;
  CodecParameterMap::const_iterator it = red_codec.params.find("");
  if (it != red_codec.params.end()) {
    red_params = it->second;
    std::vector<std::string> red_pts;
    if (talk_base::split(red_params, '/', &red_pts) != 2 ||
        red_pts[0] != red_pts[1] ||
        !talk_base::FromString(red_pts[0], &red_pt)) {
      LOG(LS_WARNING) << "RED params " << red_params << " not supported.";
      return false;
    }
  } else if (red_codec.params.empty()) {
    LOG(LS_WARNING) << "RED params not present, using defaults";
    if (all_codecs.size() > 1) {
      red_pt = all_codecs[1].id;
    }
  }

  // Try to find red_pt in |codecs|.
  std::vector<AudioCodec>::const_iterator codec;
  for (codec = all_codecs.begin(); codec != all_codecs.end(); ++codec) {
    if (codec->id == red_pt)
      break;
  }

  // If we find the right codec, that will be the codec we pass to
  // SetSendCodec, with the desired payload type.
  if (codec != all_codecs.end() &&
    engine()->FindWebRtcCodec(*codec, send_codec)) {
  } else {
    LOG(LS_WARNING) << "RED params " << red_params << " are invalid.";
    return false;
  }

  return true;
}

bool WebRtcVoiceMediaChannel::EnableRtcp(int channel) {
  if (engine()->voe()->rtp()->SetRTCPStatus(channel, true) == -1) {
    LOG_RTCERR2(SetRTCPStatus, voe_channel(), 1);
    return false;
  }
  // TODO: Enable VQMon and RTCP XR reports, once we know what
  // what we want to do with them.
  // engine()->voe().EnableVQMon(voe_channel(), true);
  // engine()->voe().EnableRTCP_XR(voe_channel(), true);
  return true;
}

bool WebRtcVoiceMediaChannel::ResetRecvCodecs(int channel) {
  int ncodecs = engine()->voe()->codec()->NumOfCodecs();
  for (int i = 0; i < ncodecs; ++i) {
    webrtc::CodecInst voe_codec;
    if (engine()->voe()->codec()->GetCodec(i, voe_codec) != -1) {
      voe_codec.pltype = -1;
      if (engine()->voe()->codec()->SetRecPayloadType(
          channel, voe_codec) == -1) {
        LOG_RTCERR2(SetRecPayloadType, channel, ToString(voe_codec));
        return false;
      }
    }
  }
  return true;
}

bool WebRtcVoiceMediaChannel::SetPlayout(int channel, bool playout) {
  if (playout) {
    LOG(LS_INFO) << "Starting playout for channel #" << channel;
    if (engine()->voe()->base()->StartPlayout(channel) == -1) {
      LOG_RTCERR1(StartPlayout, channel);
      return false;
    }
  } else {
    LOG(LS_INFO) << "Stopping playout for channel #" << channel;
    engine()->voe()->base()->StopPlayout(channel);
  }
  return true;
}

uint32 WebRtcVoiceMediaChannel::ParseSsrc(const void* data, size_t len,
                                        bool rtcp) {
  size_t ssrc_pos = (!rtcp) ? 8 : 4;
  uint32 ssrc = 0;
  if (len >= (ssrc_pos + sizeof(ssrc))) {
    ssrc = talk_base::GetBE32(static_cast<const char*>(data) + ssrc_pos);
  }
  return ssrc;
}

// Convert VoiceEngine error code into VoiceMediaChannel::Error enum.
VoiceMediaChannel::Error
    WebRtcVoiceMediaChannel::WebRtcErrorToChannelError(int err_code) {
  switch (err_code) {
    case 0:
      return ERROR_NONE;
    case VE_CANNOT_START_RECORDING:
    case VE_MIC_VOL_ERROR:
    case VE_GET_MIC_VOL_ERROR:
    case VE_CANNOT_ACCESS_MIC_VOL:
      return ERROR_REC_DEVICE_OPEN_FAILED;
    case VE_SATURATION_WARNING:
      return ERROR_REC_DEVICE_SATURATION;
    case VE_REC_DEVICE_REMOVED:
      return ERROR_REC_DEVICE_REMOVED;
    case VE_RUNTIME_REC_WARNING:
    case VE_RUNTIME_REC_ERROR:
      return ERROR_REC_RUNTIME_ERROR;
    case VE_CANNOT_START_PLAYOUT:
    case VE_SPEAKER_VOL_ERROR:
    case VE_GET_SPEAKER_VOL_ERROR:
    case VE_CANNOT_ACCESS_SPEAKER_VOL:
      return ERROR_PLAY_DEVICE_OPEN_FAILED;
    case VE_RUNTIME_PLAY_WARNING:
    case VE_RUNTIME_PLAY_ERROR:
      return ERROR_PLAY_RUNTIME_ERROR;
    case VE_TYPING_NOISE_WARNING:
      return ERROR_REC_TYPING_NOISE_DETECTED;
    default:
      return VoiceMediaChannel::ERROR_OTHER;
  }
}

int WebRtcSoundclipStream::Read(void *buf, int len) {
  size_t res = 0;
  mem_.Read(buf, len, &res, NULL);
  return res;
}

int WebRtcSoundclipStream::Rewind() {
  mem_.Rewind();
  // Return -1 to keep VoiceEngine from looping.
  return (loop_) ? 0 : -1;
}

}  // namespace cricket

#endif  // HAVE_WEBRTC_VOICE
