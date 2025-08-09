if(FLAC_FOUND)
  set(FLAC_SRCS trackflac.cpp trackflac.h)
endif()
if(MAD_FOUND)
  set(MP3_SRCS trackmp3.cpp trackmp3.h)
endif()
if(VORBIS_FOUND)
  set(VORBIS_SRCS trackoggvorbis.cpp trackoggvorbis.h)
endif()

set(TRACK_SRCS track.cpp track.h trackproxy.cpp trackproxy.h trackwav.cpp trackwav.h)
add_library(bpmdetect-track STATIC ${FLAC_SRCS} ${MP3_SRCS} ${TRACK_SRCS} ${VORBIS_SRCS})
target_compile_definitions(
  bpmdetect-track
  PRIVATE $<$<BOOL:${FLAC_FOUND}>:HAVE_FLAC> $<$<BOOL:${MAD_FOUND}>:HAVE_MAD>
          $<$<BOOL:${VORBIS_FOUND}>:HAVE_VORBISFILE> $<$<BOOL:${TAGLIB_FOUND}>:HAVE_TAGLIB>)
target_include_directories(bpmdetect-track PRIVATE ${FLAC_INCLUDE_DIRS} ${MAD_INCLUDE_DIRS}
                                                   ${VORBIS_INCLUDE_DIRS} ${TAGLIB_INCLUDE_DIRS} ..)
target_link_libraries(
  bpmdetect-track PRIVATE ${FLAC_LIBRARIES} ${MAD_LIBRARIES} ${TAGLIB_LIBRARIES}
                          ${VORBIS_LIBRARIES} Qt6::Core bpmdetect-core)
