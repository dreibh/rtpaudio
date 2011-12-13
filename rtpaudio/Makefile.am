EXTRA_DIST = AudioServer.config Makefile.am SLA.config Test.list


lib_LTLIBRARIES = libtdtoolbox.la libmediainfo.la libaudioreader.la libaudiowriter.la librtpaudio.la

libtdtoolbox_la_SOURCES = \
 breakdetector.cc breakdetector.h \
 condition.cc condition.h condition.icc \
 ext_socket.h \
 multitimerthread.h multitimerthread.icc \
 packetaddress.cc packetaddress.h packetaddress.icc \
 portableaddress.h portableaddress.icc \
 randomizer.cc randomizer.h randomizer.icc \
 ringbuffer.cc ringbuffer.h ringbuffer.icc \
 seqnumvalidator.cc seqnumvalidator.h seqnumvalidator.icc \
 synchronizable.cc synchronizable.h synchronizable.icc \
 tdsystem.h tdin6.h \
 tdmessage.h tdmessage.icc \
 tdsocket.cc tdsocket.h tdsocket.icc \
 tdstrings.cc tdstrings.h tdstrings.icc \
 thread.cc thread.h thread.icc timedthread.cc timedthread.h timedthread.icc \
 tools.cc tools.h tools.icc \
 trafficclassvalues.cc trafficclassvalues.h trafficclassvalues.icc \
 unixaddress.cc unixaddress.h unixaddress.icc

libmediainfo_la_SOURCES = \
 mediainfo.cc mediainfo.h
libmediainfo_la_LIBADD = libtdtoolbox.la


libaudioreader_la_SOURCES = \
 audioreaderinterface.cc audioreaderinterface.h \
 mp3audioreader.cc mp3audioreader.h \
 multiaudioreader.cc multiaudioreader.h \
 wavaudioreader.cc wavaudioreader.h
libaudioreader_la_LIBADD = libmediainfo.la  ../libmpegsound/libmpegsound.la


libaudiowriter_la_SOURCES = \
 audiowriterinterface.cc audiowriterinterface.h \
 multiaudiowriter.cc multiaudiowriter.h \
 audiodebug.cc audiodebug.h \
 audiodevice.cc audiodevice.h audiodevice.icc \
 spectrumanalyzer.cc spectrumanalyzer.h \
 fft.cc fft.h
libaudiowriter_la_LIBADD = libmediainfo.la


librtpaudio_la_SOURCES = \
 abstractlayerdescription.cc abstractlayerdescription.h abstractlayerdescription.icc \
 abstractqosdescription.cc abstractqosdescription.h abstractqosdescription.icc \
 advancedaudiodecoder.cc advancedaudiodecoder.h advancedaudioencoder.cc advancedaudioencoder.h \
 advancedaudiopacket.cc advancedaudiopacket.h \
 audioclient.cc audioclient.h audioclient.icc \
 audioclientapppacket.cc audioclientapppacket.h \
 audioconverter.cc audioconverter.h \
 audiodecoderinterface.h \
 audiodecoderrepository.cc audiodecoderrepository.h audiodecoderrepository.icc \
 audioencoderinterface.h \
 audioencoderrepository.cc audioencoderrepository.h audioencoderrepository.icc \
 audiomixer.cc audiomixer.h audiomixer.icc \
 audionull.cc audionull.h \
 audioquality.cc audioquality.h audioquality.icc \
 audioqualityinterface.h audioqualityinterface.icc \
 audioserver.cc audioserver.h audioserver.icc \
 bandwidthinfo.cc bandwidthinfo.h bandwidthinfo.icc \
 decoderinterface.cc decoderinterface.h decoderrepositoryinterface.h \
 encoderinterface.cc encoderinterface.h \
 encoderrepositoryinterface.h \
 frameratescalabilityinterface.h frameratescalabilityinterface.cc framesizescalabilityinterface.h framesizescalabilityinterface.cc \
 internetaddress.cc internetaddress.h internetaddress.icc internetflow.cc internetflow.h internetflow.icc \
 managedstreaminterface.h \
 qosmanagerinterface.h \
 resourceutilizationpoint.cc resourceutilizationpoint.h resourceutilizationpoint.icc \
 rtcpabstractserver.cc rtcpabstractserver.h rtcpabstractserver.icc \
 rtcppacket.cc rtcppacket.h rtcppacket.icc \
 rtcpreceiver.cc rtcpreceiver.h \
 rtcpsender.cc rtcpsender.h \
 rtppacket.cc rtppacket.h rtppacket.icc \
 rtpreceiver.cc rtpreceiver.h rtpreceiver.icc \
 rtpsender.cc rtpsender.h rtpsender.icc \
 simpleaudiodecoder.cc simpleaudiodecoder.h \
 simpleaudioencoder.cc simpleaudioencoder.h \
 simpleaudiopacket.cc simpleaudiopacket.h \
 socketaddress.cc socketaddress.h socketaddress.icc \
 sourcestateinfo.cc sourcestateinfo.h sourcestateinfo.icc \
 staticglobals.cc \
 trafficshaper.cc trafficshaper.h trafficshaper.icc
librtpaudio_la_LIBADD = libaudioreader.la libaudiowriter.la libmediainfo.la libtdtoolbox.la


INCLUDES = $(all_includes) -I../libmpegsound


if BUILD_QT_PROGRAMS
%.moc.cc: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cc: %.ui
	$(UIC) -o $@ -impl $*.h $<
endif


bin_PROGRAMS  = rtpa-server   rtpa-client   rtpa-vclient
dist_man_MANS = rtpa-server.1 rtpa-client.1 rtpa-vclient.1
if BUILD_QT_PROGRAMS
bin_PROGRAMS  += rtpa-qclient
dist_man_MANS += rtpa-qclient.1
endif


rtpa_server_SOURCES = rtpa-server.cc
rtpa_server_LDADD   = librtpaudio.la libaudioreader.la libtdtoolbox.la -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS)

rtpa_client_SOURCES = rtpa-client.cc
rtpa_client_LDADD   = librtpaudio.la libaudiowriter.la libtdtoolbox.la -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS)

rtpa_vclient_SOURCES = rtpa-vclient.cc
rtpa_vclient_LDADD   = librtpaudio.la libaudiowriter.la libtdtoolbox.la -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS)


if BUILD_QT_PROGRAMS
rtpa_qclient_MOC      = rtpa-qclient.moc.cc qinfotabwidget.moc.cc qspectrumanalyzer.moc.cc qaudiomixer.moc.cc
rtpa_qclient_CXXFLAGS = $(QT_CXXFLAGS)
rtpa_qclient_CXXADD   = $(QT_CXXFLAGS)
rtpa_qclient_SOURCES  = rtpa-qclient.cc rtpa-qclient.h qinfotabwidget.cc qinfotabwidget.h qspectrumanalyzer.cc qspectrumanalyzer.h qaudiomixer.cc qaudiomixer.h rtpa-qclient.moc.cc qinfotabwidget.moc.cc qspectrumanalyzer.moc.cc qaudiomixer.moc.cc
rtpa_qclient_LDADD    = $(QT_LDADD) librtpaudio.la libaudiowriter.la libtdtoolbox.la -lpthread $(sctplib_LIBS) $(socketapi_LIBS) $(glib_LIBS)
endif
