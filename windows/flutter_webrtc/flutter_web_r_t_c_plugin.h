#ifndef PLUGINS_FLUTTER_WEBRTC_PLUGIN_CPP_H_
#define PLUGINS_FLUTTER_WEBRTC_PLUGIN_CPP_H_

#include <flutter_plugin_registrar.h>

#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FLUTTER_PLUGIN_EXPORT __declspec(dllimport)
#endif

namespace flutter_webrtc_plugin {
class FlutterWebRTC;
class LocalVideoProcessor;
}  // namespace flutter_webrtc_plugin

#if defined(__cplusplus)
extern "C" {
#endif

FLUTTER_PLUGIN_EXPORT void FlutterWebRTCPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

FLUTTER_PLUGIN_EXPORT flutter_webrtc_plugin::FlutterWebRTC* FlutterWebRTCPluginSharedInstance();

// Installs [processor] on the local camera track [track_id]; null detaches.
// Exported as a C function because FlutterWebRTCBase's non-virtual members are
// not exported across the DLL boundary. Returns false for an unknown track or
// one that is not a camera. Call on the platform thread; on return no frame is
// inside the previous processor, so it may be destroyed. See
// local_video_processor.h.
FLUTTER_PLUGIN_EXPORT bool FlutterWebRTCPluginSetLocalVideoProcessor(
    const char* track_id,
    flutter_webrtc_plugin::LocalVideoProcessor* processor);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // PLUGINS_FLUTTER_WEBRTC_PLUGIN_CPP_H_
