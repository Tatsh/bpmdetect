# BPM Detect

[![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black)](https://www.gentoo.org/)
[![macOS](https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=F0F0F0)](https://www.apple.com/macos)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white)](https://www.microsoft.com/en-us/windows)
[![CMake](https://img.shields.io/badge/cmake-black.svg?logo=cmake&logoColor=064F8C)](https://cmake.org/)
[![ffmpeg](https://img.shields.io/badge/ffmpeg-black.svg?logo=ffmpeg&logoColor=007808)](https://ffmpeg.org/)
[![Qt 6.7+ supported](https://img.shields.io/badge/qt-6.7+-black.svg?logo=qt&logoColor=00fa6f)](https://doc.qt.io/)
[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/bpmdetect)](https://github.com/Tatsh/bpmdetect/tags)
[![License](https://img.shields.io/github/license/Tatsh/bpmdetect)](https://github.com/Tatsh/bpmdetect/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/bpmdetect/v0.8.2/master)](https://github.com/Tatsh/bpmdetect/compare/v0.8.2...master)
[![CodeQL](https://github.com/Tatsh/bpmdetect/actions/workflows/codeql.yml/badge.svg)](https://github.com/Tatsh/bpmdetect/actions/workflows/codeql.yml)
[![QA](https://github.com/Tatsh/bpmdetect/actions/workflows/qa.yml/badge.svg)](https://github.com/Tatsh/bpmdetect/actions/workflows/qa.yml)
[![Tests](https://github.com/Tatsh/bpmdetect/actions/workflows/tests.yml/badge.svg)](https://github.com/Tatsh/bpmdetect/actions/workflows/tests.yml)
[![Coverage Status](https://coveralls.io/repos/github/Tatsh/bpmdetect/badge.svg?branch=master)](https://coveralls.io/github/Tatsh/bpmdetect?branch=master)
[![GitHub Pages](https://github.com/Tatsh/bpmdetect/actions/workflows/pages.yml/badge.svg)](https://tatsh.github.io/bpmdetect/)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/bpmdetect?logo=github&style=flat)](https://github.com/Tatsh/bpmdetect/stargazers)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor%3Ddid%3Aplc%3Auq42idtvuccnmtl57nsucz72%26query%3D%24.followersCount%26style%3Dsocial%26logo%3Dbluesky%26label%3DFollow%2520%40Tatsh&query=%24.followersCount&style=social&logo=bluesky&label=Follow%20%40Tatsh)](https://bsky.app/profile/Tatsh.bsky.social)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)

![Screenshot](https://raw.githubusercontent.com/Tatsh/bpmdetect/master/screenshot.png)

BPM Detect is an automatic BPM (beats per minute) detection utility.

It uses SoundTouch's BPM library for detecting BPMs, ffmpeg for reading and writing BPMs to tags
and Qt for the GUI. It supports detection with any audio format that ffmpeg can read. However, for
files containing multiple audio tracks, only the first seen will be used for detection.

On Windows, this application requires
[Media Feature Pack](https://support.microsoft.com/en-us/topic/media-feature-pack-list-for-windows-n-editions-c1c6fffa-d052-8338-7a79-a4bb980a700a)
to be installed.

[![Packaging status](https://repology.org/badge/vertical-allrepos/bpmdetect.svg)](https://repology.org/project/bpmdetect/versions)

[Original project](https://sourceforge.net/projects/bpmdetect/)

## Building

Required dependencies:

- CMake at build time
- [ECM](https://invent.kde.org/frameworks/extra-cmake-modules) at build time
- Qt 6.7 or later with Gui and Multimedia modules
- SoundTouch 2.3.2 or later
- ffmpeg 6 or later

In the cloned project:

```shell
mkdir build
cmake ..
make
```

To build tests, add `-DBUILD_TESTS=ON`. Add `-DCOVERAGE=ON` to enable coverage (Clang and GCC only).

Translation support has been added but there are currently no translations. This can be enabled with
`-DI18N=ON`.
