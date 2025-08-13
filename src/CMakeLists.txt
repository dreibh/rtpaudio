SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(../libmpegsound)

#############################################################################
# LIBRARIES
#############################################################################

# ====== libtdtoolbox =======================================================
LIST(APPEND libtdtoolbox_headers
   breakdetector.h
   condition.h condition.icc
   ext_socket.h
   internetaddress.h internetaddress.icc
   internetflow.h internetflow.icc
   multitimerthread.h multitimerthread.icc
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
LIST(APPEND libtdtoolbox_sources
   breakdetector.cc
   condition.cc
   internetaddress.cc internetflow.cc
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

INSTALL(FILES ${libtdtoolbox_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libtdtoolbox-shared SHARED ${libtdtoolbox_sources})
ADD_LIBRARY(libtdtoolbox-static STATIC ${libtdtoolbox_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libtdtoolbox-${TYPE} PROPERTIES OUTPUT_NAME tdtoolbox CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libtdtoolbox-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libtdtoolbox-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libmediainfo =======================================================
LIST(APPEND libmediainfo_headers
   mediainfo.h
)
LIST(APPEND libmediainfo_sources
   mediainfo.cc mediainfo.h
)
ADD_LIBRARY(libmediainfo SHARED ${libmediainfo_sources})

SET_TARGET_PROPERTIES(libmediainfo PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

INSTALL(FILES ${libmediainfo_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libmediainfo-shared SHARED ${libmediainfo_sources})
ADD_LIBRARY(libmediainfo-static STATIC ${libmediainfo_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libmediainfo-${TYPE} PROPERTIES OUTPUT_NAME mediainfo CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libmediainfo-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libmediainfo-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libmediainfo-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libaudiocommon =====================================================
LIST(APPEND libaudiocommon_headers
   audioquality.h audioqualityinterface.h audioconverter.h
   audioquality.icc audioqualityinterface.icc
)
LIST(APPEND libaudiocommon_sources
   audioquality.cc audioconverter.cc
)

INSTALL(FILES ${libaudiocommon_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libaudiocommon-shared SHARED ${libaudiocommon_sources})
ADD_LIBRARY(libaudiocommon-static STATIC ${libaudiocommon_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libaudiocommon-${TYPE} PROPERTIES OUTPUT_NAME audiocommon CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libaudiocommon-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libaudiocommon-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libaudiocommon-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libaudioreader =====================================================
LIST(APPEND libaudioreader_headers
   audioreaderinterface.h mp3audioreader.h multiaudioreader.h wavaudioreader.h
)
LIST(APPEND libaudioreader_sources
   audioreaderinterface.cc mp3audioreader.cc multiaudioreader.cc wavaudioreader.cc
)

INSTALL(FILES ${libaudioreader_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libaudioreader-shared SHARED ${libaudioreader_sources})
ADD_LIBRARY(libaudioreader-static STATIC ${libaudioreader_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libaudioreader-${TYPE} PROPERTIES OUTPUT_NAME audioreader CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libaudioreader-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libaudioreader-${TYPE} libaudiocommon-${TYPE} libmediainfo-${TYPE} libtdtoolbox-${TYPE} libmpegsound-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libaudioreader-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libaudiowriter =====================================================
LIST(APPEND libaudiowriter_headers
   audiowriterinterface.h
   multiaudiowriter.h
   audiodebug.h
   audiodevice.h audiodevice.icc
   audiomixer.h audiomixer.icc
   audionull.h
   spectrumanalyzer.h
   fft.h
)
LIST(APPEND libaudiowriter_sources
   audiowriterinterface.cc
   multiaudiowriter.cc
   audiodebug.cc
   audiodevice.cc
   audiomixer.cc
   audionull.cc
   spectrumanalyzer.cc
   fft.cc
)

INSTALL(FILES ${libaudiowriter_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libaudiowriter-shared SHARED ${libaudiowriter_sources})
ADD_LIBRARY(libaudiowriter-static STATIC ${libaudiowriter_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libaudiowriter-${TYPE} PROPERTIES OUTPUT_NAME audiowriter CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libaudiowriter-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libaudiowriter-${TYPE} libaudiocommon-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${PULSEAUDIO_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libaudiowriter-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== librtpcommon =======================================================
LIST(APPEND librtpcommon_headers
   rtcppacket.h rtcppacket.icc
   rtppacket.h rtppacket.icc
)
LIST(APPEND librtpcommon_sources
   rtcppacket.cc
   rtppacket.cc
)

INSTALL(FILES ${librtpcommon_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(librtpcommon-shared SHARED ${librtpcommon_sources})
ADD_LIBRARY(librtpcommon-static STATIC ${librtpcommon_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(librtpcommon-${TYPE} PROPERTIES OUTPUT_NAME rtpcommon CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(librtpcommon-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (librtpcommon-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS librtpcommon-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== librtpserver =======================================================
LIST(APPEND librtpserver_headers
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
LIST(APPEND librtpserver_sources
   abstractlayerdescription.cc
   abstractqosdescription.cc
   bandwidthinfo.cc
   resourceutilizationpoint.cc
   rtcpabstractserver.cc
   rtcpreceiver.cc
   rtpsender.cc
   trafficshaper.cc
)

INSTALL(FILES ${librtpserver_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(librtpserver-shared SHARED ${librtpserver_sources})
ADD_LIBRARY(librtpserver-static STATIC ${librtpserver_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(librtpserver-${TYPE} PROPERTIES OUTPUT_NAME rtpserver CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(librtpserver-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (librtpserver-${TYPE} librtpcommon-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS librtpserver-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== librtpclient =======================================================
LIST(APPEND librtpclient_headers
   decoderinterface.h decoderrepositoryinterface.h
   rtcpsender.h
   rtpreceiver.h rtpreceiver.icc
   sourcestateinfo.h sourcestateinfo.icc
)
LIST(APPEND librtpclient_sources
   rtcpsender.cc
   rtpreceiver.cc
   sourcestateinfo.cc
)

INSTALL(FILES ${librtpclient_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(librtpclient-shared SHARED ${librtpclient_sources})
ADD_LIBRARY(librtpclient-static STATIC ${librtpclient_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(librtpclient-${TYPE} PROPERTIES OUTPUT_NAME rtpclient CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(librtpclient-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (librtpclient-${TYPE} librtpcommon-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS librtpclient-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libaudiocodeccommon ================================================
LIST(APPEND libaudiocodeccommon_headers
   advancedaudiopacket.h
   simpleaudiopacket.h
)
LIST(APPEND libaudiocodeccommon_sources
   advancedaudiopacket.cc
   simpleaudiopacket.cc
)

INSTALL(FILES ${libaudiocodeccommon_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libaudiocodeccommon-shared SHARED ${libaudiocodeccommon_sources})
ADD_LIBRARY(libaudiocodeccommon-static STATIC ${libaudiocodeccommon_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libaudiocodeccommon-${TYPE} PROPERTIES OUTPUT_NAME audiocodeccommon CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libaudiocodeccommon-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libaudiocodeccommon-${TYPE} libaudiocommon-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libaudiocodeccommon-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libaudioencoder ====================================================
LIST(APPEND libaudioencoder_headers
   advancedaudioencoder.h
   audioencoderinterface.h
   audioencoderrepository.h audioencoderrepository.icc
   simpleaudioencoder.h
)
LIST(APPEND libaudioencoder_sources
   advancedaudioencoder.cc
   audioencoderinterface.cc
   audioencoderrepository.cc
   simpleaudioencoder.cc
)

INSTALL(FILES ${libaudioencoder_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libaudioencoder-shared SHARED ${libaudioencoder_sources})
ADD_LIBRARY(libaudioencoder-static STATIC ${libaudioencoder_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libaudioencoder-${TYPE} PROPERTIES OUTPUT_NAME audioencoder CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libaudioencoder-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libaudioencoder-${TYPE} libaudiocodeccommon-${TYPE} libaudiocommon-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libaudioencoder-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libaudiodecoder ====================================================
LIST(APPEND libaudiodecoder_headers
   advancedaudiodecoder.h
   audiodecoderinterface.h
   audiodecoderrepository.h audiodecoderrepository.icc
   simpleaudiodecoder.h
)
LIST(APPEND libaudiodecoder_sources
   advancedaudiodecoder.cc
   audiodecoderinterface.cc
   audiodecoderrepository.cc
   simpleaudiodecoder.cc
)

INSTALL(FILES ${libaudiodecoder_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libaudiodecoder-shared SHARED ${libaudiodecoder_sources})
ADD_LIBRARY(libaudiodecoder-static STATIC ${libaudiodecoder_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libaudiodecoder-${TYPE} PROPERTIES OUTPUT_NAME audiodecoder CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libaudiodecoder-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libaudiodecoder-${TYPE} libaudiocodeccommon-${TYPE} libaudiocommon-${TYPE} libmediainfo-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libaudiodecoder-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== librtpaudiocommon ==================================================
LIST(APPEND librtpaudiocommon_headers
   audioclientapppacket.h
)
LIST(APPEND librtpaudiocommon_sources
   audioclientapppacket.cc
)

INSTALL(FILES ${librtpaudiocommon_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(librtpaudiocommon-shared SHARED ${librtpaudiocommon_sources})
ADD_LIBRARY(librtpaudiocommon-static STATIC ${librtpaudiocommon_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(librtpaudiocommon-${TYPE} PROPERTIES OUTPUT_NAME rtpaudiocommon CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(librtpaudiocommon-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (librtpaudiocommon-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS librtpaudiocommon-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== librtpaudioserver ==================================================
LIST(APPEND librtpaudioserver_headers
   audioserver.h audioserver.icc
)
LIST(APPEND librtpaudioserver_sources
   audioserver.cc
)
ADD_LIBRARY(librtpaudioserver SHARED ${librtpaudioserver_sources})

SET_TARGET_PROPERTIES(librtpaudioserver PROPERTIES
   VERSION ${BUILD_VERSION}
   SOVERSION ${BUILD_MAJOR}
)

INSTALL(FILES ${librtpaudioserver_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(librtpaudioserver-shared SHARED ${librtpaudioserver_sources})
ADD_LIBRARY(librtpaudioserver-static STATIC ${librtpaudioserver_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(librtpaudioserver-${TYPE} PROPERTIES OUTPUT_NAME rtpaudioserver CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(librtpaudioserver-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (librtpaudioserver-${TYPE} librtpaudiocommon-${TYPE} libaudioencoder-${TYPE} librtpserver-${TYPE} libaudioreader-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS librtpaudioserver-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== librtpaudioclient ==================================================
LIST(APPEND librtpaudioclient_headers
   audioclient.h audioclient.icc
)
LIST(APPEND librtpaudioclient_sources
   audioclient.cc
)

INSTALL(FILES ${librtpaudioclient_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(librtpaudioclient-shared SHARED ${librtpaudioclient_sources})
ADD_LIBRARY(librtpaudioclient-static STATIC ${librtpaudioclient_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(librtpaudioclient-${TYPE} PROPERTIES OUTPUT_NAME rtpaudioclient CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(librtpaudioclient-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (librtpaudioclient-${TYPE} librtpaudiocommon-${TYPE} libaudiocommon-${TYPE} libaudiodecoder-${TYPE} librtpclient-${TYPE} libmediainfo-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS librtpaudioclient-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()


# ====== libqosmgr ==========================================================
LIST(APPEND libqosmgr_headers
   bandwidthmanager.h bandwidthmanager.icc
   servicelevelagreement.h servicelevelagreement.icc
   pingerhost.h pingerhost.icc
   roundtriptimepinger.h roundtriptimepinger.icc
   sessiondescription.h
   streamdescription.h
)
LIST(APPEND libqosmgr_sources
   bandwidthmanager.cc
   roundtriptimepinger.cc
   servicelevelagreement.cc
   streamdescription.cc
)

INSTALL(FILES ${libqosmgr_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ADD_LIBRARY(libqosmgr-shared SHARED ${libqosmgr_sources})
ADD_LIBRARY(libqosmgr-static STATIC ${libqosmgr_sources})
FOREACH(TYPE shared;static)
   SET_TARGET_PROPERTIES(libqosmgr-${TYPE} PROPERTIES OUTPUT_NAME qosmgr CLEAN_DIRECT_OUTPUT 1)
   SET_TARGET_PROPERTIES(libqosmgr-${TYPE} PROPERTIES
      VERSION   ${BUILD_VERSION}
      SOVERSION ${BUILD_MAJOR}
   )
   TARGET_LINK_LIBRARIES (libqosmgr-${TYPE} librtpserver-${TYPE} libtdtoolbox-${TYPE} ${SCTP_LIB} ${CMAKE_THREAD_LIBS_INIT})
   INSTALL(TARGETS libqosmgr-${TYPE} DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDFOREACH()



#############################################################################
# PROGRAMS
#############################################################################

ADD_EXECUTABLE(rtpa-server rtpa-server.cc)
TARGET_LINK_LIBRARIES(rtpa-server libqosmgr-shared librtpaudioserver-shared librtpserver-shared libtdtoolbox-shared ${CMAKE_THREAD_LIBS_INIT})
INSTALL(TARGETS             rtpa-server
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION  ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES rtpa-server.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)


ADD_EXECUTABLE(rtpa-client rtpa-client.cc)
TARGET_LINK_LIBRARIES(rtpa-client librtpaudioclient-shared libaudiodecoder-shared libaudiowriter-shared libaudiocommon-shared libtdtoolbox-shared ${CMAKE_THREAD_LIBS_INIT})
INSTALL(TARGETS             rtpa-client
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION  ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES rtpa-client.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

ADD_EXECUTABLE(rtpa-vclient rtpa-vclient.cc)
TARGET_LINK_LIBRARIES(rtpa-vclient librtpaudioclient-shared libaudiodecoder-shared libaudiowriter-shared libaudiocommon-shared libtdtoolbox-shared ${CMAKE_THREAD_LIBS_INIT})
INSTALL(TARGETS             rtpa-vclient
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION  ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES rtpa-vclient.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

IF (WITH_QT)
   ADD_EXECUTABLE(rtpa-qclient rtpa-qclient.cc qaudiomixer.cc qinfotabwidget.cc qspectrumanalyzer.cc)
   TARGET_LINK_LIBRARIES(rtpa-qclient
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Widgets
   )
   TARGET_LINK_LIBRARIES(rtpa-qclient librtpaudioclient-shared libaudiodecoder-shared libaudiowriter-shared libaudiocommon-shared libtdtoolbox-shared "${CMAKE_THREAD_cLIBS_INIT}")
   INSTALL(TARGETS             rtpa-qclient
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           BUNDLE DESTINATION  ${CMAKE_INSTALL_BINDIR})
   INSTALL(FILES rtpa-qclient.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
ENDIF()
