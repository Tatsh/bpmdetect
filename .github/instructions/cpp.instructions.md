---
applyTo: '**/*.cpp, **/*.h'
---

# C++/Qt guidelines

- `QT_NO_CAST_FROM_ASCII` and similar flags are always enabled, so use `QStringLiteral()` where
  necessary.
- To convert from `char *` to `QString`, use `QString::fromUtf8()`.
- Prefer to use `.constData()` instead of `.data()` if both methods are available.
