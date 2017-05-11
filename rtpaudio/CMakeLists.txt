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


# ====== librtpcommon =======================================================
LIST(APPEND rtpcommon_headers
   rtcppacket.h rtcppacket.icc
   rtppacket.h rtppacket.icc
)
LIST(APPEND rtpcommon_sources
   rtcppacket.cc
   rtppacket.cc
)
ADD_LIBRARY(rtpcommon SHARED ${rtpcommon_sources})

set_target_properties(rtpcommon PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(rtpcommon-static STATIC ${rtpcommon_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC rtpcommon "-pthread")
endif()
TARGET_LINK_LIBRARIES (rtpcommon ${rtpcommon_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== librtspserver =======================================================
LIST(APPEND rtspserver_headers
   abstractlayerdescription.h abstractlayerdescription.icc
   abstractqosdescription.h abstractqosdescription.icc
   bandwidthinfo.h bandwidthinfo.icc
   encoderinterface.h
   encoderrepositoryinterface.h
   frameratescalabilityinterface.h framesizescalabilityinterface.h
   managedstreaminterface.h qosmanagerinterface.h
   resourceutilizationpoint.h resourceutilizationpoint.icc
   rtcpabstractserver.h rtcpabstractserver.icc
   rtcpreceiver.h
   rtpsender.h rtpsender.icc
   trafficshaper.h trafficshaper.icc
)
LIST(APPEND rtspserver_sources
   abstractlayerdescription.cc
   abstractqosdescription.cc
   bandwidthinfo.cc
   resourceutilizationpoint.cc
   rtcpabstractserver.cc
   rtcpreceiver.cc
   rtpsender.cc
   trafficshaper.cc 
)
ADD_LIBRARY(rtspserver SHARED ${rtspserver_sources})

set_target_properties(rtspserver PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(rtspserver-static STATIC ${rtspserver_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC rtspserver "-pthread")
endif()
TARGET_LINK_LIBRARIES (rtspserver rtpcommon tdtoolbox ${rtspserver_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== librtpclient =======================================================
LIST(APPEND rtpclient_headers
   decoderinterface.h decoderrepositoryinterface.h
   rtcpsender.h
   rtpreceiver.h rtpreceiver.icc
   sourcestateinfo.h sourcestateinfo.icc
)
LIST(APPEND rtpclient_sources
   rtcpsender.cc
   rtpreceiver.cc
   sourcestateinfo.cc
)
ADD_LIBRARY(rtpclient SHARED ${rtpclient_sources})

set_target_properties(rtpclient PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(rtpclient-static STATIC ${rtpclient_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC rtpclient "-pthread")
endif()
TARGET_LINK_LIBRARIES (rtpclient rtpcommon tdtoolbox ${rtpclient_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libaudiocodeccommon =======================================================
LIST(APPEND audiocodeccommon_headers
   advancedaudiopacket.h
   simpleaudiopacket.h
)
LIST(APPEND audiocodeccommon_sources
   advancedaudiopacket.cc
   simpleaudiopacket.cc
)
ADD_LIBRARY(audiocodeccommon SHARED ${audiocodeccommon_sources})

set_target_properties(audiocodeccommon PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(audiocodeccommon-static STATIC ${audiocodeccommon_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC audiocodeccommon "-pthread")
endif()
TARGET_LINK_LIBRARIES (audiocodeccommon ${audiocodeccommon_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libaudioencoder =======================================================
LIST(APPEND audioencoder_headers
   advancedaudioencoder.h
   audioencoder.h
   audioencoderinterface.h
   audioencoderrepository.h audioencoderrepository.icc
   simpleaudioencoder.h
)
LIST(APPEND audioencoder_sources
 advancedaudioencoder.cc
 audioencoderinterface.cc
 audioencoderrepository.cc 
 simpleaudioencoder.cc 
)
ADD_LIBRARY(audioencoder SHARED ${audioencoder_sources})

set_target_properties(audioencoder PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(audioencoder-static STATIC ${audioencoder_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC audioencoder "-pthread")
endif()
TARGET_LINK_LIBRARIES (audioencoder audiocodeccommon audiocommon ${audioencoder_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libaudiodecoder =======================================================
LIST(APPEND audiodecoder_headers
   advancedaudiodecoder.h 
   audiodecoderinterface.h
   udiodecoderrepository.h audiodecoderrepository.icc
   simpleaudiodecoder.h
)
LIST(APPEND audiodecoder_sources
   advancedaudiodecoder.cc 
   audiodecoderinterface.cc 
   audiodecoderrepository.cc a
   simpleaudiodecoder.cc
)
ADD_LIBRARY(audiodecoder SHARED ${audiodecoder_sources})

set_target_properties(audiodecoder PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(audiodecoder-static STATIC ${audiodecoder_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC audiodecoder "-pthread")
endif()
TARGET_LINK_LIBRARIES (audiodecoder audiocodeccommon audiocommon mediainfo tdtoolbox ${audiodecoder_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== librtpaudiocommon =======================================================
LIST(APPEND rtpaudiocommon_headers
   audioclientapppacket.h
)
LIST(APPEND rtpaudiocommon_sources
   audioclientapppacket.cc
)
ADD_LIBRARY(rtpaudiocommon SHARED ${rtpaudiocommon_sources})

set_target_properties(rtpaudiocommon PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(rtpaudiocommon-static STATIC ${rtpaudiocommon_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC rtpaudiocommon "-pthread")
endif()
TARGET_LINK_LIBRARIES (rtpaudiocommon ${rtpaudiocommon_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== librtpaudioserver =======================================================
LIST(APPEND rtpaudioserver_headers
   audioserver.h audioserver.icc
)
LIST(APPEND rtpaudioserver_sources
   audioserver.cc
)
ADD_LIBRARY(rtpaudioserver SHARED ${rtpaudioserver_sources})

set_target_properties(rtpaudioserver PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(rtpaudioserver-static STATIC ${rtpaudioserver_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC rtpaudioserver "-pthread")
endif()
TARGET_LINK_LIBRARIES (rtpaudioserver rtpaudiocommon audioencoder rtpserver audioreader tdtoolbox ${rtpaudioserver_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== librtpaudioclient =======================================================
LIST(APPEND rtpaudioclient_headers
   audioclient.h audioclient.icc
)
LIST(APPEND rtpaudioclient_sources
   audioclient.cc
)
ADD_LIBRARY(rtpaudioclient SHARED ${rtpaudioclient_sources})

set_target_properties(rtpaudioclient PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(rtpaudioclient-static STATIC ${rtpaudioclient_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC rtpaudioclient "-pthread")
endif()
TARGET_LINK_LIBRARIES (rtpaudioclient rtpaudiocommon audiodecoder audiocommon rtpclient mediainfo tdtoolbox ${rtpaudioclient_libs} "${CMAKE_THREAD_LIBS_INIT}")


# ====== libqosmgr =======================================================
LIST(APPEND qosmgr_headers
   bandwidthmanager.h bandwidthmanager.icc
   servicelevelagreement.h servicelevelagreement.icc
   pingerhost.h pingerhost.icc
   roundtriptimepinger.h roundtriptimepinger.icc
   sessiondescription.h
   streamdescription.h
)
LIST(APPEND qosmgr_sources
   bandwidthmanager.cc
   servicelevelagreement.cc
   
   roundtriptimepinger.cc
   streamdescription.cc
)
ADD_LIBRARY(qosmgr SHARED ${qosmgr_sources})

set_target_properties(qosmgr PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

ADD_LIBRARY(qosmgr-static STATIC ${qosmgr_sources})

if(THREADS_HAVE_PTHREAD_ARG)
   TARGET_COMPILE_OPTIONS(PUBLIC qosmgr "-pthread")
endif()
TARGET_LINK_LIBRARIES (qosmgr rtpserver tdtoolbox ${qosmgr_libs} "${CMAKE_THREAD_LIBS_INIT}")
