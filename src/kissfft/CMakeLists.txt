set(KISSFFT_SRCS _kiss_fft_guts.h kiss_fft.c kiss_fft.h kiss_fftr.c kiss_fftr.h)
add_library(bpmdetect-kissfft STATIC ${KISSFFT_SRCS})
