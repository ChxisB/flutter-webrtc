#include "local_video_processing_bridge.h"

#include <utility>

namespace flutter_webrtc_plugin {

using namespace libwebrtc;

LocalVideoProcessingBridge::LocalVideoProcessingBridge(
    scoped_refptr<RTCVideoTrack> camera_track,
    scoped_refptr<RTCVideoSource> output)
    : camera_track_(std::move(camera_track)), output_(std::move(output)) {
  camera_track_->AddRenderer(this);
}

LocalVideoProcessingBridge::~LocalVideoProcessingBridge() {
  camera_track_->RemoveRenderer(this);
}

void LocalVideoProcessingBridge::SetProcessor(LocalVideoProcessor* processor) {
  std::lock_guard<std::mutex> lock(mutex_);
  processor_ = processor;
}

void LocalVideoProcessingBridge::OnFrame(scoped_refptr<RTCVideoFrame> frame) {
  // The lock spans the callback deliberately: it is what lets a dependent
  // plugin destroy its processor straight after detaching it, knowing no frame
  // is still inside. Android's processor list and iOS/macOS's
  // VideoProcessingAdapter hold their own locks across a frame for the same
  // reason.
  std::lock_guard<std::mutex> lock(mutex_);
  if (processor_ != nullptr) {
    frame = processor_->Process(std::move(frame));
    if (!frame.get()) {
      return;
    }
  }
  output_->OnCapturedFrame(std::move(frame));
}

}  // namespace flutter_webrtc_plugin
