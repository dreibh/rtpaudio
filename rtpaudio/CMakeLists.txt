include_directories(. ..)

# ====== libtdtoolbox =======================================================
LIST(APPEND tdtoolbox_headers
   breakdetector.h
   condition.h condition.icc
   internetaddress.h internetaddress.icc 
   internetflow.h internetflow.icc
   multitimerthread.h multitimerthread.icc
   packetaddress.h packetaddress.icc
   portableaddress.h portableaddress.icc
   randomizer.h randomizer.icc
   ringbuffer.h ringbuffer.icc
   seqnumvalidator.h seqnumvalidator.icc
   socketaddress.h socketaddress.icc
   synchronizable.h synchronizable.icc
   tdsystem.h tdin6.h
   tdmessage.h tdmessage.icc
   tdsocket.h tdsocket.icc
   tdstrings.h tdstrings.icc
   thread.h thread.icc timedthread.h timedthread.icc
   tools.h tools.icc
   trafficclassvalues.h trafficclassvalues.icc
   unixaddress.h unixaddress.icc
)
LIST(APPEND tdtoolbox_sources
   breakdetector.cc 
   condition.cc
   ext_socket.h
   internetaddress.cc internetflow.cc
   packetaddress.cc
   randomizer.cc
   ringbuffer.cc
   seqnumvalidator.cc
   socketaddress.cc
   synchronizable.cc
   tdsocket.cc
   tdstrings.cc
   thread.cc timedthread.cc
   tools.cc
   trafficclassvalues.cc
   unixaddress.cc
)
ADD_LIBRARY(tdtoolbox SHARED ${tdtoolbox_sources})

set_target_properties(tdtoolbox PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(tdtoolbox-static STATIC ${tdtoolbox_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC tdtoolbox "-pthread")
endif()
TARGET_LINK_LIBRARIES (tdtoolbox ${tdtoolbox_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libmediainfo =======================================================
LIST(APPEND mediainfo_headers
   mediainfo.h
)
LIST(APPEND mediainfo_sources
   mediainfo.cc mediainfo.h
)
ADD_LIBRARY(mediainfo SHARED ${mediainfo_sources})

set_target_properties(mediainfo PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(mediainfo-static STATIC ${mediainfo_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC mediainfo "-pthread")
endif()
TARGET_LINK_LIBRARIES (mediainfo ${mediainfo_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libaudiocommon =======================================================
LIST(APPEND audiocommon_headers
   audioquality.h audioqualityinterface.h audioconverter.h
   audioquality.icc audioqualityinterface.icc
)
LIST(APPEND audiocommon_sources
   audioquality.cc audioconverter.cc
)
ADD_LIBRARY(audiocommon SHARED ${audiocommon_sources})

set_target_properties(audiocommon PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(audiocommon-static STATIC ${audiocommon_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC audiocommon "-pthread")
endif()
TARGET_LINK_LIBRARIES (audiocommon toolbox ${audiocommon_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libaudioreader =======================================================
LIST(APPEND audioreader_headers
   audioreaderinterface.h mp3audioreader.h multiaudioreader.h wavaudioreader.h
)
LIST(APPEND audioreader_sources
   audioreaderinterface.cc mp3audioreader.cc multiaudioreader.cc wavaudioreader.cc
)
ADD_LIBRARY(audioreader SHARED ${audioreader_sources})

set_target_properties(audioreader PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(audioreader-static STATIC ${audioreader_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC audioreader "-pthread")
endif()
TARGET_LINK_LIBRARIES (audioreader audiocommon mediainfo tdtoolbox mpegsound ${audioreader_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libaudiowriter =======================================================
LIST(APPEND audiowriter_headers
   audiowriterinterface.h
   multiaudiowriter.h
   audiodebug.h
   audiodevice.h audiodevice.icc
   audiomixer.h audiomixer.icc
   audionull.h
   spectrumanalyzer.h
   fft.h
)
LIST(APPEND audiowriter_sources
 audiowriterinterface.cc 
 multiaudiowriter.cc 
 audiodebug.cc 
 audiodevice.cc
 audiomixer.cc
 audionull.cc
 spectrumanalyzer.cc
 fft.cc
)
ADD_LIBRARY(audiowriter SHARED ${audiowriter_sources})

set_target_properties(audiowriter PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(audiowriter-static STATIC ${audiowriter_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC audiowriter "-pthread")
endif()
TARGET_LINK_LIBRARIES (audiowriter audiocommon tdtoolbox ${audiowriter_libs} "${CMAKE_THREAD_LIBS_INIT}")
