#include "WinSparkle-0.9.3/include/winsparkle.h"

#include <flutter/event_channel.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

namespace {
// Forward declarations for WinSparkle callbacks
void __onErrorCallback();
void __onShutdownRequestCallback();
void __onDidFindUpdateCallback();
void __onDidNotFindUpdateCallback();
void __onUpdateCancelledCallback();
void __onUpdateSkippedCallback();
void __onUpdatePostponedCallback();
void __onUpdateDismissedCallback();
void __onUserRunInstallerCallback();

class AutoUpdater {
 public:
  static AutoUpdater* GetInstance();

  AutoUpdater();

  virtual ~AutoUpdater();

  void AutoUpdater::SetFeedURL(std::string feedURL);
  void AutoUpdater::SetEdDSAPublicKey(std::string base64PublicKey);
  void AutoUpdater::CheckForUpdates();
  void AutoUpdater::CheckForUpdatesWithoutUI();
  void AutoUpdater::SetScheduledCheckInterval(int interval);

  void AutoUpdater::RegisterEventSink(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> ptr);
  void AutoUpdater::OnWinSparkleEvent(std::string eventName);

 private:
  static AutoUpdater* lazySingleton;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
  // EdDSA (ed25519) public key, base64-encoded, used to verify update
  // signatures. Cached here so it can be applied right before
  // win_sparkle_init() regardless of the order SetEdDSAPublicKey / SetFeedURL
  // are invoked from Dart. WinSparkle requires all configuration (including the
  // public key) to be set before win_sparkle_init().
  std::string eddsa_public_key_;
};

AutoUpdater* AutoUpdater::lazySingleton = nullptr;

AutoUpdater* AutoUpdater::GetInstance() {
  return lazySingleton;
}

AutoUpdater::AutoUpdater() {
  if (lazySingleton != nullptr) {
    throw std::invalid_argument("AutoUpdater has already been initialized");
  }

  lazySingleton = this;
}

AutoUpdater::~AutoUpdater() {}

void AutoUpdater::SetFeedURL(std::string feedURL) {
  win_sparkle_set_appcast_url(feedURL.c_str());

  // The EdDSA public key must be configured before win_sparkle_init(). Applying
  // the cached key here (rather than in SetEdDSAPublicKey) makes the Dart-side
  // call order between setEdDSAPublicKey() and setFeedURL() irrelevant.
  if (!eddsa_public_key_.empty()) {
    win_sparkle_set_eddsa_public_key(eddsa_public_key_.c_str());
  }

  win_sparkle_init();

  win_sparkle_set_error_callback(__onErrorCallback);
  win_sparkle_set_shutdown_request_callback(__onShutdownRequestCallback);
  win_sparkle_set_did_find_update_callback(__onDidFindUpdateCallback);
  win_sparkle_set_did_not_find_update_callback(__onDidNotFindUpdateCallback);
  win_sparkle_set_update_cancelled_callback(__onUpdateCancelledCallback);

  // TODO: These will be supported once we update WinSparkle to >0.8.0
  // win_sparkle_set_update_skipped_callback(__onUpdateSkippedCallback);
  // win_sparkle_set_update_postponed_callback(__onUpdatePostponedCallback);
  // win_sparkle_set_update_dismissed_callback(__onUpdateDismissedCallback);
  // win_sparkle_set_user_run_installer_callback(__onUserRunInstallerCallback);
}

void AutoUpdater::SetEdDSAPublicKey(std::string base64PublicKey) {
  // Cache only. The key is actually handed to WinSparkle in SetFeedURL(), right
  // before win_sparkle_init(), because win_sparkle_set_eddsa_public_key() must
  // be called before initialization to take effect.
  eddsa_public_key_ = base64PublicKey;
}

void AutoUpdater::CheckForUpdates() {
  win_sparkle_check_update_with_ui();
  OnWinSparkleEvent("checking-for-update");
}

void AutoUpdater::CheckForUpdatesWithoutUI() {
  win_sparkle_check_update_without_ui();
  OnWinSparkleEvent("checking-for-update");
}

void AutoUpdater::SetScheduledCheckInterval(int interval) {
  win_sparkle_set_update_check_interval(interval);
}

void AutoUpdater::RegisterEventSink(
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> ptr) {
  event_sink_ = std::move(ptr);
}

void AutoUpdater::OnWinSparkleEvent(std::string eventName) {
  if (event_sink_ == nullptr)
    return;
  flutter::EncodableMap args = flutter::EncodableMap();
  args[flutter::EncodableValue("type")] = eventName;
  if (event_sink_) {
    event_sink_->Success(flutter::EncodableValue(args));
  }
}

void __onErrorCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("error");
}

void __onShutdownRequestCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("before-quit-for-update");
}

void __onDidFindUpdateCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("update-available");
}

void __onDidNotFindUpdateCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("update-not-available");
}

void __onUpdateCancelledCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("updateCancelled");
}

void __onUpdateSkippedCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("updateSkipped");
}

void __onUpdatePostponedCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("updatePostponed");
}

void __onUpdateDismissedCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("updateDismissed");
}

void __onUserRunInstallerCallback() {
  AutoUpdater* autoUpdater = AutoUpdater::GetInstance();
  if (autoUpdater == nullptr)
    return;
  autoUpdater->OnWinSparkleEvent("userRunInstaller");
}
}  // namespace
