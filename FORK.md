# flutter_webrtc fork: local video processing hook (Windows, Linux)

Fork of [flutter-webrtc/flutter-webrtc](https://github.com/flutter-webrtc/flutter-webrtc)
(MIT — see `LICENSE`; upstream's copyright notice is retained, with one line added for
this patch) based on upstream tag `v1.6.0`, with one patch that upstream does not have yet.

| Branch | What |
|---|---|
| `windows-video-processor` | The local video processing hook for Windows/Linux, below, on top of upstream `v1.6.0`. |

## Patch: local video processing hook (Windows, Linux)

Android and iOS/macOS already let another plugin see — and replace — every frame of a
local camera track before it is encoded (`LocalVideoTrack.addProcessor`,
`VideoProcessingAdapter.addProcessing`). The desktop C++ layer had no equivalent, which
is why a camera-effects plugin (for example background blur) could not work on Windows.
This adds one.

**How.** `GetUserVideo` used to build the published track directly on the camera's
source. Now the camera gets its own *hidden* track, the published track is built on a
libwebrtc *custom* source, and a `LocalVideoProcessingBridge` renders the hidden track,
runs the installed processor if there is one, and pushes the result into the custom
source (`OnCapturedFrame`). Dart sees the same track id, and the sender, stop paths and
`video_capturers_` bookkeeping are unchanged.

**API for dependent plugins.**

```cpp
#include "local_video_processor.h"   // common/cpp/include

class MyProcessor : public flutter_webrtc_plugin::LocalVideoProcessor {
  scoped_refptr<RTCVideoFrame> Process(scoped_refptr<RTCVideoFrame> frame) override;
};

// windows/flutter_webrtc/flutter_web_r_t_c_plugin.h — exported from the plugin DLL.
FlutterWebRTCPluginSetLocalVideoProcessor(track_id, &processor);  // nullptr detaches
```

`Process` runs synchronously on the capture thread; return the frame itself or one from
`RTCVideoFrame::Create`/`Copy`; null drops it. Detaching returns only once no frame is
inside the old processor. A dependent plugin should resolve the export at runtime
(`GetProcAddress`) so it still loads against upstream flutter_webrtc.

Plugins vendor their own copy of `local_video_processor.h` rather than including this
one, so they should also check `FlutterWebRTCPluginLocalVideoProcessorAbiVersion()` —
currently **1** — and refuse to install on a mismatch, instead of calling through a
vtable the two sides disagree about. Bump it whenever `LocalVideoProcessor` changes
shape.

**Cost.** No processor installed: one extra renderer callback and `OnCapturedFrame` per
frame, no pixel copy. With one: whatever the processor does.

**Known limitations.**
- Pass-through frames keep their capture timestamp. A frame a processor builds with
  `RTCVideoFrame::Create` has none — the public frame type exposes no setter — so the
  custom source stamps it on arrival, a few milliseconds after capture.
- `enabled = false` on the published track stops sending, but the hidden track keeps
  delivering frames to the processor. Harmless, but a processor that is expensive should
  check.
- Linux gets the bridge (it is common code) but no exported setter; add one to
  `linux/flutter_webrtc_plugin.cc` if Linux ever needs effects.

**Verification status.** On macOS, the bridge and the patched `flutter_media_stream.cc` /
`flutter_webrtc_base.cc` pass `clang++ -fsyntax-only` against libwebrtc's wrapper headers
and Flutter's C++ client wrapper headers. The Windows plugin file is Windows-only and was
not compiled. Checked against libwebrtc's wrapper source as well:
`CreateCustomVideoSource` builds a pass-through capturer, so `OnCapturedFrame` forwards
the native buffer without copying; and `RemoveRenderer` takes the same mutex the sink
adapter holds while delivering a frame, so destroying a bridge waits for any frame still
inside it. **Nothing has been built or run on Windows yet.** Before relying on it:

```powershell
cd <your-flutter-app>
flutter build windows --debug     # x64, then again from an ARM64 host
# place a call: the camera must work exactly as before with no processor installed
```

## Using the fork

```yaml
# pubspec.yaml
dependency_overrides:
  flutter_webrtc:
    git:
      url: https://github.com/ChxisB/flutter-webrtc.git
      ref: windows-video-processor
```

It is upstream 1.6.0 on Android, iOS, macOS and web — the patch only touches the
Windows/Linux C++ — so the override changes nothing on the other platforms.

## Keeping it current

Rebase the patch branch onto a new upstream tag:

```bash
git fetch upstream --tags
git rebase --onto v<version> v1.6.0 windows-video-processor
```

## Upstreaming

The hook is desktop parity with the mobile processor APIs, which makes it a reasonable
upstream PR (an earlier, broader attempt — #2094 — was closed unmerged). Once merged
upstream, drop the override and this fork.
