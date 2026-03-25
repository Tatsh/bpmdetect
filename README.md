# BPM Detect

[![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B)](https://isocpp.org)
[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/bpmdetect)](https://github.com/Tatsh/bpmdetect/tags)
[![License](https://img.shields.io/github/license/Tatsh/bpmdetect)](https://github.com/Tatsh/bpmdetect/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/bpmdetect/v0.8.9/master)](https://github.com/Tatsh/bpmdetect/compare/v0.8.9...master)
[![QA](https://github.com/Tatsh/bpmdetect/actions/workflows/qa.yml/badge.svg)](https://github.com/Tatsh/bpmdetect/actions/workflows/qa.yml)
[![Dependabot](https://img.shields.io/badge/Dependabot-enabled-blue?logo=dependabot)](https://github.com/dependabot)
[![GitHub Pages](https://github.com/Tatsh/bpmdetect/actions/workflows/pages.yml/badge.svg)](https://tatsh.github.io/bpmdetect/)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/bpmdetect?logo=github&style=flat)](https://github.com/Tatsh/bpmdetect/stargazers)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit)](https://pre-commit.com/)
[![CMake](https://img.shields.io/badge/CMake-6E6E6E?logo=cmake)](https://cmake.org/)
[![Prettier](https://img.shields.io/badge/Prettier-enabled-black?logo=prettier)](https://prettier.io/)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor=did%3Aplc%3Auq42idtvuccnmtl57nsucz72&query=%24.followersCount&label=Follow+%40Tatsh&logo=bluesky&style=social)](https://bsky.app/profile/Tatsh.bsky.social)
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Tatsh-black?logo=buymeacoffee)](https://buymeacoffee.com/Tatsh)
[![Libera.Chat](https://img.shields.io/badge/Libera.Chat-Tatsh-black?logo=liberadotchat)](irc://irc.libera.chat/Tatsh)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)
[![Patreon](https://img.shields.io/badge/Patreon-Tatsh2-F96854?logo=patreon)](https://www.patreon.com/Tatsh2)

BPM Detect is an automatic BPM (beats per minute) detection utility.

It uses SoundTouch's BPM library for detecting BPMs, ffmpeg for reading and writing BPMs to tags
and Qt for the GUI. It supports detection with any audio format that ffmpeg can read. However, for
files containing multiple audio tracks, only the first seen will be used for detection.

On Windows, this application requires
[Media Feature Pack](https://support.microsoft.com/en-us/topic/media-feature-pack-list-for-windows-n-editions-c1c6fffa-d052-8338-7a79-a4bb980a700a)
to be installed.

Some formats cannot save tags. The application will not warn you about these. M4A (AAC, 3GP, etc)
store tags in the `tmpo` atom which is limited to integers but this will not be seen as saved on
restart because ffmpeg does not parse the `tmpo` atom when reading the file.

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

On MacPorts, set the `CMAKE_PREFIX_PATH` variable to
`/opt/local/libexec/ffmpeg7;/opt/local/libexec/qt6`.

To build tests, add `-DBUILD_TESTS=ON`. Add `-DCOVERAGE=ON` to enable coverage (Clang and GCC only).

Translation support has been added but there are currently no translations. This can be enabled with
`-DI18N=ON`.
