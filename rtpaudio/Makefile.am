EXTRA_DIST = AudioServer.config Makefile.am SLA.config Test.list


noinst_LIBRARIES = librtpaudio.a

librtpaudio_a_SOURCES = \
 abstractlayerdescription.cc abstractlayerdescription.h abstractlayerdescription.icc \
 abstractqosdescription.cc abstractqosdescription.h abstractqosdescription.icc \
 advancedaudiodecoder.cc advancedaudiodecoder.h advancedaudioencoder.cc advancedaudioencoder.h \
 advancedaudiopacket.cc advancedaudiopacket.h \
 audioclient.cc audioclient.h audioclient.icc \
 audioclientapppacket.cc audioclientapppacket.h \
 audioconverter.cc audioconverter.h \
 audiodebug.cc audiodebug.h \
 audiodecoderinterface.h \
 audiodecoderrepository.cc audiodecoderrepository.h audiodecoderrepository.icc \
 audiodevice.cc audiodevice.h audiodevice.icc \
 audioencoderinterface.h \
 audioencoderrepository.cc audioencoderrepository.h audioencoderrepository.icc \
 audiomixer.cc audiomixer.h audiomixer.icc \
 audionull.cc audionull.h \
 audioquality.cc audioquality.h audioquality.icc \
 audioqualityinterface.h audioqualityinterface.icc \
 audioreaderinterface.cc audioreaderinterface.h \
 audioserver.cc audioserver.h audioserver.icc \
 audiowriterinterface.cc audiowriterinterface.h \
 bandwidthinfo.cc bandwidthinfo.h bandwidthinfo.icc \
 bandwidthmanager.h \
 breakdetector.cc breakdetector.h \
 cbrframesizescalability.cc  cbrframesizescalability.h \
 condition.cc condition.h condition.icc \
 decoderinterface.cc decoderinterface.h decoderrepositoryinterface.h \
 encoderinterface.cc encoderinterface.h \
 encoderrepositoryinterface.h \
 ext_socket.h \
 extsocketdescriptor.h \
 fft.cc fft.h \
 frameratescalabilityinterface.h framesizescalabilityinterface.h \
 genericframesizescalability.cc  genericframesizescalability.h \
 internetaddress.cc internetaddress.h internetaddress.icc internetflow.cc internetflow.h internetflow.icc \
 managedstreaminterface.h \
 mediainfo.cc mediainfo.h \
 mp3audioreader.cc mp3audioreader.h \
 multiaudioreader.cc multiaudioreader.h \
 multiaudiowriter.cc multiaudiowriter.h \
 multitimerthread.h multitimerthread.icc \
 packetaddress.cc packetaddress.h packetaddress.icc \
 portableaddress.h portableaddress.icc \
 randomizer.cc randomizer.h randomizer.icc \
 resourceutilizationpoint.cc resourceutilizationpoint.h resourceutilizationpoint.icc \
 ringbuffer.cc ringbuffer.h ringbuffer.icc \
 rtcpabstractserver.cc rtcpabstractserver.h rtcpabstractserver.icc \
 rtcppacket.cc rtcppacket.h rtcppacket.icc \
 rtcpreceiver.cc rtcpreceiver.h \
 rtcpsender.cc rtcpsender.h \
 rtppacket.cc rtppacket.h rtppacket.icc \
 rtpreceiver.cc rtpreceiver.h rtpreceiver.icc \
 rtpsender.cc rtpsender.h rtpsender.icc \
 seqnumvalidator.cc seqnumvalidator.h seqnumvalidator.icc \
 simpleaudiodecoder.cc simpleaudiodecoder.h \
 simpleaudioencoder.cc simpleaudioencoder.h \
 simpleaudiopacket.cc simpleaudiopacket.h \
 socketaddress.cc socketaddress.h socketaddress.icc \
 sourcestateinfo.cc sourcestateinfo.h sourcestateinfo.icc \
 spectrumanalyzer.cc spectrumanalyzer.h \
 staticglobals.cc \
 streamsrcdest.cc streamsrcdest.h \
 synchronizable.cc synchronizable.h synchronizable.icc \
 tdsystem.h tdin6.h \
 tdmessage.h tdmessage.icc \
 tdsocket.cc tdsocket.h tdsocket.icc \
 tdstrings.cc tdstrings.h tdstrings.icc \
 thread.cc thread.h thread.icc timedthread.cc timedthread.h timedthread.icc \
 tools.cc tools.h tools.icc \
 trafficclassvalues.cc trafficclassvalues.h trafficclassvalues.icc \
 trafficshaper.cc trafficshaper.h trafficshaper.icc \
 unixaddress.cc unixaddress.h unixaddress.icc \
 wavaudioreader.cc wavaudioreader.h


INCLUDES = $(all_includes) -I../libmpegsound


if BUILD_QT_PROGRAMS
%.moc.cc: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cc: %.ui
	$(UIC) -o $@ -impl $*.h $<
endif


if BUILD_QT_PROGRAMS
bin_PROGRAMS = rtpa-server rtpa-client rtpa-vclient rtpa-qclient
else
bin_PROGRAMS = rtpa-server rtpa-client rtpa-vclient
endif


rtpa_server_SOURCES = rtpa-server.cc
rtpa_server_LDADD   = librtpaudio.a -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS) ../libmpegsound/libmpegsound.a

rtpa_client_SOURCES = rtpa-client.cc
rtpa_client_LDADD   = librtpaudio.a -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS)

rtpa_vclient_SOURCES = rtpa-vclient.cc
rtpa_vclient_LDADD   = librtpaudio.a -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS)


# if BUILD_QT_PROGRAMS
rtpa_qclient_MOC      = rtpa-qclient.moc.cc qinfotabwidget.moc.cc qspectrumanalyzer.moc.cc qaudiomixer.moc.cc
rtpa_qclient_CXXFLAGS = $(QT_CXXFLAGS)
rtpa_qclient_CXXADD   = $(QT_CXXFLAGS)
rtpa_qclient_SOURCES  = rtpa-qclient.cc qinfotabwidget.cc qspectrumanalyzer.cc qaudiomixer.cc rtpa-qclient.moc.cc qinfotabwidget.moc.cc qspectrumanalyzer.moc.cc qaudiomixer.moc.cc
rtpa_qclient_LDADD    = $(QT_LDADD) librtpaudio.a -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS)
# endif
