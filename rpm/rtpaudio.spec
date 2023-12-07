Name: rtpaudio
Version: 2.0.8
Release: 1
Summary: RTP Audio network sound streaming system
License: GPL-3+
Group: Applications/Internet
URL: https://www.uni-due.de/~be0001/rtpaudio/
Source: https://www.uni-due.de/~be0001/rtpaudio/download/%{name}-%{version}.tar.xz

AutoReqProv: on
BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: lksctp-tools-devel
BuildRequires: qt5-qtbase-devel
BuildRequires: qtchooser
BuildRequires: pulseaudio-libs-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-build

# Meta-package rtpaudio: install rtpaudio-all => install all sub-packages!
Requires: %{name}-all


%description
The RTP Audio system is a network sound streaming system. It has been designed for QoS performance analysis and teaching purposes. RTP Audio supports IPv4 and IPv6 including flowlabels and traffic classes, QoS management as well as transport via UDP and SCTP.

%prep
%setup -q

%build
%cmake -DCMAKE_INSTALL_PREFIX=/usr .
%cmake_build

%install
%cmake_install

%files all


%package libmpegsound
Summary: Sound decoder library
Group: Development/Libraries

%description libmpegsound
 libmpegsound decodes a couple of sound formats (e.g. MP3) and
 returns a raw data stream.
 .
 The library is provided by this package.

%files libmpegsound
%{_libdir}/libmpegsound.so*


%package libmpegsound-devel
Summary: Development files for the sound decoder library
Group: Development/Libraries

%description libmpegsound-devel
 libmpegsound decodes a couple of sound formats (e.g. MP3) and
 returns a raw data stream.
 .
 The library is provided by this package.

%files libmpegsound-devel
%{_includedir}/mpegsound.h
%{_includedir}/mpegsound_locals.h
%{_libdir}/libmpegsound*.a
%{_libdir}/libmpegsound*.so


%package libtdtoolbox
Summary: Shared library for RTP Audio (common helper functions)
Group: Development/Libraries

%description libtdtoolbox
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides a shared library for common helper functions.

%files libtdtoolbox
%{_libdir}/libtdtoolbox.so*


%package libtdtoolbox-devel
Group: Development/Libraries
Requires: %{name}-libtdtoolbox = %{version}-%{release}
Summary: Development files for RTP Audio (common helper functions)

%description libtdtoolbox-devel
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides the development files for libtdtoolbox.

%files libtdtoolbox-devel
%{_libdir}/libtdtoolbox*.a
%{_libdir}/libtdtoolbox*.so
%{_includedir}/breakdetector.h
%{_includedir}/condition.h
%{_includedir}/condition.icc
%{_includedir}/ext_socket.h
%{_includedir}/internetaddress.h
%{_includedir}/internetaddress.icc
%{_includedir}/internetflow.h
%{_includedir}/internetflow.icc
%{_includedir}/multitimerthread.h
%{_includedir}/multitimerthread.icc
%{_includedir}/portableaddress.h
%{_includedir}/portableaddress.icc
%{_includedir}/randomizer.h
%{_includedir}/randomizer.icc
%{_includedir}/ringbuffer.h
%{_includedir}/ringbuffer.icc
%{_includedir}/seqnumvalidator.h
%{_includedir}/seqnumvalidator.icc
%{_includedir}/socketaddress.h
%{_includedir}/socketaddress.icc
%{_includedir}/synchronizable.h
%{_includedir}/synchronizable.icc
%{_includedir}/tdsystem.h
%{_includedir}/tdin6.h
%{_includedir}/tdmessage.h
%{_includedir}/tdmessage.icc
%{_includedir}/tdsocket.h
%{_includedir}/tdsocket.icc
%{_includedir}/tdstrings.h
%{_includedir}/tdstrings.icc
%{_includedir}/thread.h
%{_includedir}/thread.icc
%{_includedir}/timedthread.h
%{_includedir}/timedthread.icc
%{_includedir}/tools.h
%{_includedir}/tools.icc
%{_includedir}/trafficclassvalues.h
%{_includedir}/trafficclassvalues.icc
%{_includedir}/unixaddress.h
%{_includedir}/unixaddress.icc


%package libmediainfo
Summary: Shared library for RTP Audio (media information handling)
Group: Development/Libraries

%description libmediainfo
Shared library for RTP Audio (media information handling)
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides a shared library for media information handling.

%files libmediainfo
%{_libdir}/libmediainfo.so*


%package libmediainfo-devel
Summary: Development files for RTP Audio (media information handling)
Group: Development/Libraries
Requires: %{name}-libmediainfo = %{version}-%{release}

%description libmediainfo-devel
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides the development files for libmediainfo.

%files libmediainfo-devel
%{_libdir}/libmediainfo*.so
%{_libdir}/libmediainfo*.a
%{_includedir}/mediainfo.h


%package libaudiocommon
Summary:   Shared library for RTP Audio (common audio data handling)
Group: Development/Libraries
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description libaudiocommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common audio data handling.

%files libaudiocommon
%{_libdir}/libaudiocommon.so*


%package libaudiocommon-devel
Summary:   Development files for RTP Audio (common audio data handling)
Group: Development/Libraries
Requires: %{name}-libaudiocommon = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description libaudiocommon-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for common audio data handling.

%files libaudiocommon-devel
%{_libdir}/libaudiocommon*.a
%{_libdir}/libaudiocommon*.so
%{_includedir}/audioquality.h
%{_includedir}/audioqualityinterface.h
%{_includedir}/audioconverter.h
%{_includedir}/audioquality.icc
%{_includedir}/audioqualityinterface.icc


%package libaudioreader
Summary: Shared library for RTP Audio (audio input reading)
Group: Development/Libraries
Requires: %{name}-libaudiocommon = %{version}-%{release}
Requires: %{name}-libmediainfo = %{version}-%{release}
Requires: %{name}-libmpegsound = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description libaudioreader
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides a shared library for audio input reading.

%files libaudioreader
%{_libdir}/libaudioreader.so*


%package libaudioreader-devel
Summary: Development files for RTP Audio (audio input reading)
Group: Development/Libraries
Requires: %{name}-libaudioreader = %{version}-%{release}
Requires: %{name}-libaudiocommon-devel = %{version}-%{release}
Requires: %{name}-libmediainfo-devel = %{version}-%{release}
Requires: %{name}-libmpegsound-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description libaudioreader-devel
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides the development files for audio input reading.

%files libaudioreader-devel
%{_libdir}/libaudioreader*.a
%{_libdir}/libaudioreader*.so
%{_includedir}/audioreaderinterface.h
%{_includedir}/mp3audioreader.h
%{_includedir}/multiaudioreader.h
%{_includedir}/wavaudioreader.h


%package libaudiowriter
Summary: Shared library for RTP Audio (audio output writing)
Group: Development/Libraries
Requires: pulseaudio-libs
Requires: %{name}-libaudiocommon = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description libaudiowriter
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for audio output writing.

%files libaudiowriter
%{_libdir}/libaudiowriter.so*


%package libaudiowriter-devel
Summary: Development files for RTP Audio (audio output writing)
Group: Development/Libraries
Requires: pulseaudio-libs-devel
Requires: %{name}-libaudiowriter = %{version}-%{release}
Requires: %{name}-libaudiocommon-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description libaudiowriter-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for audio output writing.

%files libaudiowriter-devel
%{_libdir}/libaudiowriter*.a
%{_libdir}/libaudiowriter*.so
%{_includedir}/audiowriterinterface.h
%{_includedir}/multiaudiowriter.h
%{_includedir}/audiodebug.h
%{_includedir}/audiodevice.h
%{_includedir}/audiodevice.icc
%{_includedir}/audiomixer.h
%{_includedir}/audiomixer.icc
%{_includedir}/audionull.h
%{_includedir}/spectrumanalyzer.h
%{_includedir}/fft.h


%package libaudiocodeccommon
Summary: Shared library for RTP Audio (common audio codec handling)
Group: Development/Libraries
Requires: %{name}-libaudiocommon

%description libaudiocodeccommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common audio codec handling.

%files libaudiocodeccommon
%{_libdir}/libaudiocodeccommon.so*


%package libaudiocodeccommon-devel
Summary: Development files for RTP Audio (common audio codec handling)
Group: Development/Libraries
Requires: %{name}-libaudiocodeccommon = %{version}-%{release}
Requires: %{name}-libaudiocommon-devel = %{version}-%{release}

%description libaudiocodeccommon-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for common audio codec handling.

%files libaudiocodeccommon-devel
%{_libdir}/libaudiocodeccommon*.a
%{_libdir}/libaudiocodeccommon*.so
%{_includedir}/advancedaudiopacket.h
%{_includedir}/simpleaudiopacket.h


%package libaudioencoder
Summary: Shared library for RTP Audio (audio encoding)
Group: Development/Libraries
Requires: %{name}-libaudiocodeccommon = %{version}-%{release}
Requires: %{name}-libaudiocommon = %{version}-%{release}

%description libaudioencoder
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for audio encoding.

%files libaudioencoder
%{_libdir}/libaudioencoder.so*


%package libaudioencoder-devel
Summary: Development files for RTP Audio (audio encoding)
Group: Development/Libraries
Requires: %{name}-libaudioencoder = %{version}-%{release}
Requires: %{name}-libaudiocodeccommon-devel = %{version}-%{release}
Requires: %{name}-libaudiocommon-devel = %{version}-%{release}

%description libaudioencoder-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for audio encoding.

%files libaudioencoder-devel
%{_libdir}/libaudioencoder*.a
%{_libdir}/libaudioencoder*.so
%{_includedir}/advancedaudioencoder.h
%{_includedir}/audioencoderinterface.h
%{_includedir}/audioencoderrepository.h
%{_includedir}/audioencoderrepository.icc
%{_includedir}/simpleaudioencoder.h


%package libaudiodecoder
Summary: Shared library for RTP Audio (audio decoding)
Group: Development/Libraries
Requires: %{name}-libaudiocodeccommon = %{version}-%{release}
Requires: %{name}-libaudiocommon = %{version}-%{release}
Requires: %{name}-libmediainfo = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description libaudiodecoder
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for audio decoding.

%files libaudiodecoder
%{_libdir}/libaudiodecoder.so*


%package libaudiodecoder-devel
Summary: Development files for RTP Audio (audio decoding)
Group: Development/Libraries
Requires: %{name}-libaudiodecoder = %{version}-%{release}
Requires: %{name}-libaudiocodeccommon-devel = %{version}-%{release}
Requires: %{name}-libaudiocommon-devel = %{version}-%{release}
Requires: %{name}-libmediainfo-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description libaudiodecoder-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for audio decoding.

%files libaudiodecoder-devel
%{_libdir}/libaudiodecoder*.a
%{_libdir}/libaudiodecoder*.so
%{_includedir}/advancedaudiodecoder.h
%{_includedir}/audiodecoderinterface.h
%{_includedir}/audiodecoderrepository.h
%{_includedir}/audiodecoderrepository.icc
%{_includedir}/simpleaudiodecoder.h


%package librtpaudiocommon
Summary: Shared library for RTP Audio (common RTP audio handling)
Group: Development/Libraries

%description librtpaudiocommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common RTP audio handling.

%files librtpaudiocommon
%{_libdir}/librtpaudiocommon.so*


%package librtpaudiocommon-devel
Summary: Development files for RTP Audio (common RTP audio handling)
Group: Development/Libraries
Requires: %{name}-librtpaudiocommon = %{version}-%{release}

%description librtpaudiocommon-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for common RTP audio handling.

%files librtpaudiocommon-devel
%{_libdir}/librtpaudiocommon*.a
%{_libdir}/librtpaudiocommon*.so
%{_includedir}/audioclientapppacket.h


%package librtpaudioclient
Summary: Shared library for RTP Audio (RTP client-side audio handling)
Group: Development/Libraries
Requires: %{name}-libaudiocommon = %{version}-%{release}
Requires: %{name}-libaudiodecoder = %{version}-%{release}
Requires: %{name}-libmediainfo = %{version}-%{release}
Requires: %{name}-librtpaudiocommon = %{version}-%{release}
Requires: %{name}-librtpclient = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description librtpaudioclient
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for RTP client-side audio handling.

%files librtpaudioclient
%{_libdir}/librtpaudioclient.so*


%package librtpaudioclient-devel
Summary: Development files for RTP Audio (RTP client-side audio handling)
Group: Development/Libraries
Requires: %{name}-librtpaudioclient = %{version}-%{release}
Requires: %{name}-libaudiocommon-devel = %{version}-%{release}
Requires: %{name}-libaudiodecoder-devel = %{version}-%{release}
Requires: %{name}-libmediainfo-devel = %{version}-%{release}
Requires: %{name}-librtpaudiocommon-devel = %{version}-%{release}
Requires: %{name}-librtpclient-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description librtpaudioclient-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for RTP client-side audio handling.

%files librtpaudioclient-devel
%{_libdir}/librtpaudioclient*.a
%{_libdir}/librtpaudioclient*.so
%{_includedir}/audioclient.h
%{_includedir}/audioclient.icc


%package librtpaudioserver
Summary: Shared library for RTP Audio (RTP server-side audio handling)
Group: Development/Libraries
Requires: %{name}-libaudioencoder = %{version}-%{release}
Requires: %{name}-libaudioreader = %{version}-%{release}
Requires: %{name}-librtpaudiocommon = %{version}-%{release}
Requires: %{name}-librtpserver = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description librtpaudioserver
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for RTP server-side audio handling.

%files librtpaudioserver
%{_libdir}/librtpaudioserver.so*


%package librtpaudioserver-devel
Summary: Development files for RTP Audio (RTP server-side audio handling)
Group: Development/Libraries
Requires: %{name}-librtpaudioserver = %{version}-%{release}
Requires: %{name}-libaudioencoder-devel = %{version}-%{release}
Requires: %{name}-libaudioreader-devel = %{version}-%{release}
Requires: %{name}-librtpaudiocommon-devel = %{version}-%{release}
Requires: %{name}-librtpserver-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description librtpaudioserver-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for RTP server-side audio handling.

%files librtpaudioserver-devel
%{_libdir}/librtpaudioserver*.a
%{_libdir}/librtpaudioserver*.so
%{_includedir}/audioserver.h
%{_includedir}/audioserver.icc


%package librtpcommon
Summary: Shared library for RTP Audio (common RTP handling)
Group: Development/Libraries

%description librtpcommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common RTP handling.

%files librtpcommon
%{_libdir}/librtpcommon.so*


%package librtpcommon-devel
Summary: Development files for RTP Audio (common RTP handling)
Group: Development/Libraries
Requires: %{name}-librtpcommon = %{version}-%{release}

%description librtpcommon-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for common RTP handling.

%files librtpcommon-devel
%{_libdir}/librtpcommon*.a
%{_libdir}/librtpcommon*.so
%{_includedir}/rtcppacket.h
%{_includedir}/rtcppacket.icc
%{_includedir}/rtppacket.h
%{_includedir}/rtppacket.icc


%package librtpclient
Summary: Shared library for RTP Audio (RTP client-side handling)
Group: Development/Libraries
Requires: %{name}-librtpcommon = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description librtpclient
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for RTP client-side handling.

%files librtpclient
%{_libdir}/librtpclient.so*


%package librtpclient-devel
Summary: Development files for RTP Audio (RTP client-side handling)
Group: Development/Libraries
Requires: %{name}-librtpclient = %{version}-%{release}
Requires: %{name}-librtpcommon-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description librtpclient-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for RTP client-side handling.

%files librtpclient-devel
%{_libdir}/librtpclient*.a
%{_libdir}/librtpclient*.so
%{_includedir}/decoderinterface.h
%{_includedir}/decoderrepositoryinterface.h
%{_includedir}/rtcpsender.h
%{_includedir}/rtpreceiver.h
%{_includedir}/rtpreceiver.icc
%{_includedir}/sourcestateinfo.h
%{_includedir}/sourcestateinfo.icc


%package librtpserver
Summary: Shared library for RTP Audio (RTP server-side handling)
Group: Development/Libraries
Requires: %{name}-librtpcommon = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description librtpserver
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for RTP server-side handling.

%files librtpserver
%{_libdir}/librtpserver.so*


%package librtpserver-devel
Summary: Development files for RTP Audio (RTP server-side handling)
Group: Development/Libraries
Requires: %{name}-librtpserver = %{version}-%{release}
Requires: %{name}-librtpcommon-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description librtpserver-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for RTP server-side handling.

%files librtpserver-devel
%{_libdir}/librtpserver*.a
%{_libdir}/librtpserver*.so
%{_includedir}/abstractlayerdescription.h
%{_includedir}/abstractlayerdescription.icc
%{_includedir}/abstractqosdescription.h
%{_includedir}/abstractqosdescription.icc
%{_includedir}/bandwidthinfo.h
%{_includedir}/bandwidthinfo.icc
%{_includedir}/encoderinterface.h
%{_includedir}/encoderrepositoryinterface.h
%{_includedir}/frameratescalabilityinterface.h
%{_includedir}/framesizescalabilityinterface.h
%{_includedir}/managedstreaminterface.h
%{_includedir}/qosmanagerinterface.h
%{_includedir}/resourceutilizationpoint.h
%{_includedir}/resourceutilizationpoint.icc
%{_includedir}/rtcpabstractserver.h
%{_includedir}/rtcpabstractserver.icc
%{_includedir}/rtcpreceiver.h
%{_includedir}/rtpsender.h
%{_includedir}/rtpsender.icc
%{_includedir}/trafficshaper.h
%{_includedir}/trafficshaper.icc


%package libqosmgr
Summary: Shared library for RTP Audio (QoS management)
Group: Development/Libraries
Requires: %{name}-librtpserver = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}

%description libqosmgr
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for the QoS manager.

%files libqosmgr
%{_libdir}/libqosmgr.so*


%package libqosmgr-devel
Summary: Development files for RTP Audio (QoS management)
Group: Development/Libraries
Requires: %{name}-libqosmgr = %{version}-%{release}
Requires: %{name}-librtpserver-devel = %{version}-%{release}
Requires: %{name}-libtdtoolbox-devel = %{version}-%{release}

%description libqosmgr-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for the QoS manager.

%files libqosmgr-devel
%{_libdir}/libqosmgr*.a
%{_libdir}/libqosmgr*.so
%{_includedir}/bandwidthmanager.h
%{_includedir}/bandwidthmanager.icc
%{_includedir}/servicelevelagreement.h
%{_includedir}/servicelevelagreement.icc
%{_includedir}/pingerhost.h
%{_includedir}/pingerhost.icc
%{_includedir}/roundtriptimepinger.h
%{_includedir}/roundtriptimepinger.icc
%{_includedir}/sessiondescription.h
%{_includedir}/streamdescription.h


%package rtpaudio-clients
Summary: RTP Audio clients
Requires: %{name}-libaudiocommon = %{version}-%{release}
Requires: %{name}-libaudiodecoder = %{version}-%{release}
Requires: %{name}-libaudiowriter = %{version}-%{release}
Requires: %{name}-librtpaudioclient = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}
Recommends: netperfmeter
Recommends: rsplib-tools
Recommends: subnetcalc
Recommends: traceroute
%description rtpaudio-clients
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 RTP Audio supports IPv4 and IPv6 including flowlabels and traffic
 classes, QoS management as well as transport via UDP and SCTP.
 .
 This package provides the RTP Audio clients.

%files rtpaudio-clients
%{_bindir}/rtpa-client
%{_bindir}/rtpa-qclient
%{_bindir}/rtpa-vclient
%{_mandir}/man1/rtpa-client.1.gz
%{_mandir}/man1/rtpa-qclient.1.gz
%{_mandir}/man1/rtpa-vclient.1.gz


%package rtpaudio-server
Summary: RTP Audio server
Requires: %{name}-libqosmgr = %{version}-%{release}
Requires: %{name}-librtpaudioserver = %{version}-%{release}
Requires: %{name}-librtpserver = %{version}-%{release}
Requires: %{name}-libtdtoolbox = %{version}-%{release}
Recommends: netperfmeter
Recommends: rsplib-tools
Recommends: subnetcalc
Recommends: traceroute
%description rtpaudio-server
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 RTP Audio supports IPv4 and IPv6 including flowlabels and traffic
 classes, QoS management as well as transport via UDP and SCTP.
 .
 This package provides the RTP Audio server.

%files rtpaudio-server
%{_bindir}/rtpa-server
%{_mandir}/man1/rtpa-server.1.gz


%package all
Summary: RTP Audio sound streaming system
Requires: %{name}-rtpaudio-clients = %{version}-%{release}
Requires: %{name}-rtpaudio-server = %{version}-%{release}

%description all
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 RTP Audio supports IPv4 and IPv6 including flowlabels and traffic
 classes, QoS management as well as transport via UDP and SCTP.

%files all


%changelog
* Thu Dec 07 2023 Thomas Dreibholz <thomas.dreibholz@gmail.com> - 2.0.8
- New upstream release.
* Wed Dec 06 2023 Thomas Dreibholz <thomas.dreibholz@gmail.com> - 2.0.7
- New upstream release.
* Sun Sep 11 2022 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.0.6
- New upstream release.
* Thu Feb 17 2022 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.0.5
- New upstream release.
* Wed Feb 16 2022 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.0.4
- New upstream release.
* Fri Nov 13 2020 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.0.3
- New upstream release.
* Fri Feb 07 2020 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.0.2
- New upstream release.
* Thu Aug 08 2019 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.0.1
- New upstream release.
* Wed Aug 07 2019 Thomas Dreibholz <dreibh@iem.uni-due.de> - 2.0.0
- New upstream release.
* Thu Nov 23 2017 Thomas Dreibholz <dreibh@simula.no> 2.0.0~beta4
- Initial RPM release
