## 1.0.1

* [CepheusLabs fork] Upgrade vendored WinSparkle 0.8.1 -> 0.9.3.
* [CepheusLabs fork] Add `setEdDSAPublicKey` method channel handler; the key is
  applied via `win_sparkle_set_eddsa_public_key()` immediately before
  `win_sparkle_init()` so Dart-side call order does not matter.

## 1.0.0

* First major release.

## 0.2.1

* chore(windows): Support before-quit-for-update event

## 0.2.0

* First release.
