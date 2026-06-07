## 1.0.1

* [CepheusLabs fork] Handle `setEdDSAPublicKey` as a no-op (macOS reads its
  Ed25519 key from the `SUPublicEDKey` Info.plist entry), so cross-platform
  callers can invoke it unconditionally without a `MissingPluginException`.

## 1.0.0

* First major release.

## 0.2.0

* First release.
