include_directories(.)

# ====== libmpegsound =======================================================
LIST(APPEND libmpegsound_headers
   mpegsound.h mpegsound_locals.h
)
LIST(APPEND libmpegsound_sources
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

INSTALL(FILES ${libmpegsound_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libmpegsound-shared SHARED ${libmpegsound_sources})
ADD_LIBRARY(libmpegsound-static STATIC ${libmpegsound_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libmpegsound-${TYPE} PROPERTIES OUTPUT_NAME mpegsound CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libmpegsound-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libmpegsound-${TYPE} librtpserver-${TYPE} "${CMAKE_THREAD_LIBS_INIT}")
   INSTALL(TARGETS libmpegsound-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})

   # FIXME: The libmpegsound sources need a review!
   SET_TARGET_PROPERTIES(libmpegsound-${TYPE} PROPERTIES COMPILE_FLAGS "-Wno-unused-private-field -Wno-aggressive-loop-optimizations -Wno-narrowing -Wno-sequence-point -Wno-unused-result -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-empty-body -Wno-unknown-warning-option -Wno-deprecated-register -Wno-self-assign")

ENDFOREACH()
