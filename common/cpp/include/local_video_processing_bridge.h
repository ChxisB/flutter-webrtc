#ifndef FLUTTER_WEBRTC_LOCAL_VIDEO_PROCESSING_BRIDGE_HXX
#define FLUTTER_WEBRTC_LOCAL_VIDEO_PROCESSING_BRIDGE_HXX

#include <mutex>

#include "libwebrtc.h"
#include "local_video_processor.h"
#include "rtc_video_frame.h"
#include "rtc_video_renderer.h"
#include "rtc_video_source.h"
#include "rtc_video_track.h"

namespace flutter_webrtc_plugin {

// Sits between the camera and the track Dart publishes.
//
// `GetUserVideo` gives the camera its own hidden track and hands Dart a track
// built on a *custom* source instead. This renders the hidden track, runs the
// installed [LocalVideoProcessor] if there is one, and pushes the result into
// that custom source. With no processor installed the frame is passed straight
// through — same object, no copy — so an unused hook costs a virtual call.
//
// Lives as long as the published track: `FlutterWebRTCBase` owns it, keyed by
// that track's id.
class LocalVideoProcessingBridge
    : public libwebrtc::RTCVideoRenderer<
          libwebrtc::scoped_refptr<libwebrtc::RTCVideoFrame>> {
 public:
  LocalVideoProcessingBridge(
      libwebrtc::scoped_refptr<libwebrtc::RTCVideoTrack> camera_track,
      libwebrtc::scoped_refptr<libwebrtc::RTCVideoSource> output);

  ~LocalVideoProcessingBridge() override;

  // Null detaches. Returns once no frame is inside the previous processor, so a
  // caller may destroy it immediately afterwards.
  void SetProcessor(LocalVideoProcessor* processor);

  void OnFrame(
      libwebrtc::scoped_refptr<libwebrtc::RTCVideoFrame> frame) override;

 private:
  const libwebrtc::scoped_refptr<libwebrtc::RTCVideoTrack> camera_track_;
  const libwebrtc::scoped_refptr<libwebrtc::RTCVideoSource> output_;

  std::mutex mutex_;
  LocalVideoProcessor* processor_ = nullptr;
};

}  // namespace flutter_webrtc_plugin

#endif  // FLUTTER_WEBRTC_LOCAL_VIDEO_PROCESSING_BRIDGE_HXX
