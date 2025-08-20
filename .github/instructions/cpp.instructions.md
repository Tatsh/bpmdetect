---
applyTo: '**/*.cpp, **/*.h'
---

# C++/Qt guidelines

- `QT_NO_CAST_FROM_ASCII` and similar flags are always enabled, so use `QStringLiteral()` where
  necessary.
- To convert from `char *` to `QString`, use `QString::fromUtf8()`.
- Prefer to use `.constData()` instead of `.data()` if both methods are available.
- `QT_NO_SIGNALS_SLOTS_KEYWORDS` is always enabled, so do not use `signals:` or `slots:` keywords.
- All public signals should be under the `Q_SIGNALS:` macro.
