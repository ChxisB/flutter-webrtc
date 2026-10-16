#ifndef FLUTTER_WEBRTC_LOCAL_VIDEO_PROCESSOR_HXX
#define FLUTTER_WEBRTC_LOCAL_VIDEO_PROCESSOR_HXX

#include "libwebrtc.h"
#include "rtc_video_frame.h"

namespace flutter_webrtc_plugin {

// Sees every frame of a local camera track before it is published, and may
// replace it.
//
// This is the desktop counterpart of the hooks the other platforms already
// have: Android's `LocalVideoTrack.addProcessor` and iOS/macOS's
// `VideoProcessingAdapter.addProcessing`. It is implemented by a *dependent*
// plugin (background blur, beautification, an overlay) and installed with
// `FlutterWebRTCPluginSetLocalVideoProcessor`.
//
// [Process] runs on the capture thread, synchronously, in the path between the
// camera and the encoder — so it must be quick, and must not block on anything.
// It returns either its own argument or a frame obtained from
// `RTCVideoFrame::Create`/`Copy`; returning null drops the frame.
class LocalVideoProcessor {
 public:
  virtual ~LocalVideoProcessor() = default;

  virtual libwebrtc::scoped_refptr<libwebrtc::RTCVideoFrame> Process(
      libwebrtc::scoped_refptr<libwebrtc::RTCVideoFrame> frame) = 0;
};

}  // namespace flutter_webrtc_plugin

#endif  // FLUTTER_WEBRTC_LOCAL_VIDEO_PROCESSOR_HXX
