if(FLAC_FOUND)
  set(FLAC_SRCS trackflac.cpp trackflac.h)
endif()
if(MAD_FOUND)
  set(MP3_SRCS trackmp3.cpp trackmp3.h)
endif()
if(VORBIS_FOUND)
  set(VORBIS_SRCS trackoggvorbis.cpp trackoggvorbis.h)
endif()
if(WAVPACK_FOUND)
  set(WAVPACK_SRCS trackwavpack.cpp trackwavpack.h)
endif()

set(TRACK_SRCS track.cpp track.h trackproxy.cpp trackproxy.h trackwav.cpp trackwav.h)
add_library(bpmdetect-track STATIC ${FLAC_SRCS} ${MP3_SRCS} ${TRACK_SRCS} ${VORBIS_SRCS}
                                   ${WAVPACK_SRCS})
target_compile_definitions(
  bpmdetect-track
  PRIVATE $<$<BOOL:${FLAC_FOUND}>:HAVE_FLAC> $<$<BOOL:${MAD_FOUND}>:HAVE_MAD>
          $<$<BOOL:${VORBIS_FOUND}>:HAVE_VORBISFILE> $<$<BOOL:${WAVPACK_FOUND}>:HAVE_WAVPACK>)
target_include_directories(bpmdetect-track PRIVATE ..)
target_link_libraries(
  bpmdetect-track
  PRIVATE $<$<BOOL:${FLAC_FOUND}>:PkgConfig::FLAC>
          $<$<BOOL:${MAD_FOUND}>:PkgConfig::MAD>
          $<$<BOOL:${VORBIS_FOUND}>:PkgConfig::VORBIS>
          $<$<BOOL:${WAVPACK_FOUND}>:PkgConfig::WAVPACK>
          PkgConfig::SOUNDTOUCH
          PkgConfig::TAGLIB
          Qt6::Core)
target_compile_definitions(bpmdetect-track PRIVATE SAMPLE_MAX_VALUE=32768)
