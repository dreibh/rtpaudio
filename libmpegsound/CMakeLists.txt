include_directories(.)

# ====== libmpegsound =======================================================
LIST(APPEND mpegsound_headers
   mpegsound.h mpegsound_locals.h
)
LIST(APPEND mpegsound_sources
   soundinputstream.cc fileinput.cc httpinput.cc
   soundplayer.cc rawplayer.cc rawtofile.cc
   mpegtable.cc filter.cc filter_2.cc
   mpegtoraw.cc mpeglayer1.cc mpeglayer2.cc
   mpeglayer3.cc bitwindow.cc huffmantable.cc
   wavetoraw.cc sidplayer.cc
   fileplayer.cc nasplayer.cc oggplayer.cc
   xingheader.cc esdplayer.cc
   sdlplayer.cc cyclicbuffer.cc
)
ADD_LIBRARY(mpegsound SHARED ${mpegsound_sources})

set_target_properties(mpegsound PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(mpegsound-static STATIC ${mpegsound_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC mpegsound "-pthread")
endif()
TARGET_LINK_LIBRARIES (mpegsound ${mpegsound_libs} "${CMAKE_THREAD_LIBS_INIT}")


# FIXME: The libmpegsound sources need a review!
SET_TARGET_PROPERTIES(mpegsound        PROPERTIES COMPILE_FLAGS "-Wno-aggressive-loop-optimizations -Wno-narrowing -Wno-sequence-point -Wno-unused-result -Wno-unused-but-set-variable -Wno-strict-aliasing")
SET_TARGET_PROPERTIES(mpegsound-static PROPERTIES COMPILE_FLAGS "-Wno-aggressive-loop-optimizations -Wno-narrowing -Wno-sequence-point -Wno-unused-result -Wno-unused-but-set-variable -Wno-strict-aliasing")


INSTALL(TARGETS mpegsound mpegsound-static DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL(FILES ${mpegsound_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
