Name: rtpaudio
Version: 2.0.0~beta7
Release: 1
Summary: Reliable Server Pooling (RSerPool) implementation
License: GPL-3.0
Group: Applications/Internet
URL: https://www.uni-due.de/~be0001/rtpaudio/
Source: https://www.uni-due.de/~be0001/rtpaudio/download/%{name}-%{version}.tar.gz

AutoReqProv: on
BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: lksctp-tools-devel
BuildRequires: qt5-qtbase-devel
BuildRequires: qtchooser
BuildRequires: pulseaudio-libs-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-build

%description
The RTP Audio system is a network sound streaming systen. It has been designed for QoS performance analysis and teaching purposes. RTP Audio supports IPv4 and IPv6 including flowlabels and traffic classes, QoS management as well as transport via UDP and SCTP.

# FIXME: The RPM packages should contain everything!
# %define _unpackaged_files_terminate_build 0

%prep
%setup -q

%build
%cmake -DCMAKE_INSTALL_PREFIX=/usr .
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}



%package libmpegsound
Summary: Sound decoder library
Group: Development/Libraries

%description libmpegsound
 libmpegsound decodes a couple of sound formats (e.g. MP3) and
 returns a raw data stream.
 .
 The library is provided by this package.

%files libmpegsound
/usr/lib*/libmpegsound.so*


%package libmpegsound-devel
Summary: Development files for the sound decoder library
Group: Development/Libraries

%description libmpegsound-devel
 libmpegsound decodes a couple of sound formats (e.g. MP3) and
 returns a raw data stream.
 .
 The library is provided by this package.

%files libmpegsound-devel
/usr/include/mpegsound.h
/usr/include/mpegsound_locals.h
/usr/lib*/libmpegsound*.a
/usr/lib*/libmpegsound*.so


%package libtdtoolbox
Summary: Shared library for RTP Audio (common helper functions)
Group: Development/Libraries

%description libtdtoolbox
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides a shared library for common helper functions.

%files libtdtoolbox
/usr/lib*/libtdtoolbox.so*


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
/usr/lib*/libtdtoolbox*.a
/usr/lib*/libtdtoolbox*.so
/usr/include/breakdetector.h
/usr/include/condition.h
/usr/include/condition.icc
/usr/include/ext_socket.h
/usr/include/internetaddress.h
/usr/include/internetaddress.icc
/usr/include/internetflow.h
/usr/include/internetflow.icc
/usr/include/multitimerthread.h
/usr/include/multitimerthread.icc
/usr/include/portableaddress.h
/usr/include/portableaddress.icc
/usr/include/randomizer.h
/usr/include/randomizer.icc
/usr/include/ringbuffer.h
/usr/include/ringbuffer.icc
/usr/include/seqnumvalidator.h
/usr/include/seqnumvalidator.icc
/usr/include/socketaddress.h
/usr/include/socketaddress.icc
/usr/include/synchronizable.h
/usr/include/synchronizable.icc
/usr/include/tdsystem.h
/usr/include/tdin6.h
/usr/include/tdmessage.h
/usr/include/tdmessage.icc
/usr/include/tdsocket.h
/usr/include/tdsocket.icc
/usr/include/tdstrings.h
/usr/include/tdstrings.icc
/usr/include/thread.h
/usr/include/thread.icc
/usr/include/timedthread.h
/usr/include/timedthread.icc
/usr/include/tools.h
/usr/include/tools.icc
/usr/include/trafficclassvalues.h
/usr/include/trafficclassvalues.icc
/usr/include/unixaddress.h
/usr/include/unixaddress.icc


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
/usr/lib*/libmediainfo.so*


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
/usr/lib*/libmediainfo*.so
/usr/lib*/libmediainfo*.a
/usr/include/mediainfo.h


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
/usr/lib*/libaudiocommon.so*


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
/usr/lib*/libaudiocommon*.a
/usr/lib*/libaudiocommon*.so
/usr/include/audioquality.h
/usr/include/audioqualityinterface.h
/usr/include/audioconverter.h
/usr/include/audioquality.icc
/usr/include/audioqualityinterface.icc


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
/usr/lib*/libaudioreader.so*


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
/usr/lib*/libaudioreader*.a
/usr/lib*/libaudioreader*.so
/usr/include/audioreaderinterface.h
/usr/include/mp3audioreader.h
/usr/include/multiaudioreader.h
/usr/include/wavaudioreader.h


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
/usr/lib*/libaudiowriter.so*


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
/usr/lib*/libaudiowriter*.a
/usr/lib*/libaudiowriter*.so
/usr/include/audiowriterinterface.h
/usr/include/multiaudiowriter.h
/usr/include/audiodebug.h
/usr/include/audiodevice.h
/usr/include/audiodevice.icc
/usr/include/audiomixer.h
/usr/include/audiomixer.icc
/usr/include/audionull.h
/usr/include/spectrumanalyzer.h
/usr/include/fft.h


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
/usr/lib*/libaudiocodeccommon.so*


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
/usr/lib*/libaudiocodeccommon*.a
/usr/lib*/libaudiocodeccommon*.so
/usr/include/advancedaudiopacket.h
/usr/include/simpleaudiopacket.h


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
/usr/lib*/libaudioencoder.so*


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
/usr/lib*/libaudioencoder*.a
/usr/lib*/libaudioencoder*.so
/usr/include/advancedaudioencoder.h
/usr/include/audioencoderinterface.h
/usr/include/audioencoderrepository.h
/usr/include/audioencoderrepository.icc
/usr/include/simpleaudioencoder.h


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
/usr/lib*/libaudiodecoder.so*


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
/usr/lib*/libaudiodecoder*.a
/usr/lib*/libaudiodecoder*.so
/usr/include/advancedaudiodecoder.h
/usr/include/audiodecoderinterface.h
/usr/include/audiodecoderrepository.h
/usr/include/audiodecoderrepository.icc
/usr/include/simpleaudiodecoder.h


%package librtpaudiocommon
Summary: Shared library of the RTP Audio sound streaming system
Group: Development/Libraries

%description librtpaudiocommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common RTP audio handling.

%files librtpaudiocommon
/usr/lib*/librtpaudiocommon.so*


%package librtpaudiocommon-devel
Summary: Shared library of the RTP Audio sound streaming system
Group: Development/Libraries
Requires: %{name}-librtpaudiocommon = %{version}-%{release}

%description librtpaudiocommon-devel
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides the development files for common RTP audio handling.

%files librtpaudiocommon-devel
/usr/lib*/librtpaudiocommon*.a
/usr/lib*/librtpaudiocommon*.so
/usr/include/audioclientapppacket.h


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
/usr/lib*/librtpaudioclient.so*


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
/usr/lib*/librtpaudioclient*.a
/usr/lib*/librtpaudioclient*.so
/usr/include/audioclient.h
/usr/include/audioclient.icc


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
/usr/lib*/librtpaudioserver.so*


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
/usr/lib*/librtpaudioserver*.a
/usr/lib*/librtpaudioserver*.so
/usr/include/audioserver.h
/usr/include/audioserver.icc


%package librtpcommon
Summary: Shared library for RTP Audio (common RTP handling)
Group: Development/Libraries

%description librtpcommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common RTP handling.

%files librtpcommon
/usr/lib*/librtpcommon.so*


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
/usr/lib*/librtpcommon*.a
/usr/lib*/librtpcommon*.so
/usr/include/rtcppacket.h
/usr/include/rtcppacket.icc
/usr/include/rtppacket.h
/usr/include/rtppacket.icc


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
/usr/lib*/librtpclient.so*


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
/usr/lib*/librtpclient*.a
/usr/lib*/librtpclient*.so
/usr/include/decoderinterface.h
/usr/include/decoderrepositoryinterface.h
/usr/include/rtcpsender.h
/usr/include/rtpreceiver.h
/usr/include/rtpreceiver.icc
/usr/include/sourcestateinfo.h
/usr/include/sourcestateinfo.icc


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
/usr/lib*/librtpserver.so*


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
/usr/lib*/librtpserver*.a
/usr/lib*/librtpserver*.so
/usr/include/abstractlayerdescription.h
/usr/include/abstractlayerdescription.icc
/usr/include/abstractqosdescription.h
/usr/include/abstractqosdescription.icc
/usr/include/bandwidthinfo.h
/usr/include/bandwidthinfo.icc
/usr/include/encoderinterface.h
/usr/include/encoderrepositoryinterface.h
/usr/include/frameratescalabilityinterface.h
/usr/include/framesizescalabilityinterface.h
/usr/include/managedstreaminterface.h
/usr/include/qosmanagerinterface.h
/usr/include/resourceutilizationpoint.h
/usr/include/resourceutilizationpoint.icc
/usr/include/rtcpabstractserver.h
/usr/include/rtcpabstractserver.icc
/usr/include/rtcpreceiver.h
/usr/include/rtpsender.h
/usr/include/rtpsender.icc
/usr/include/trafficshaper.h
/usr/include/trafficshaper.icc


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
/usr/lib*/libqosmgr.so*


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
/usr/lib*/libqosmgr*.a
/usr/lib*/libqosmgr*.so
/usr/include/bandwidthmanager.h
/usr/include/bandwidthmanager.icc
/usr/include/servicelevelagreement.h
/usr/include/servicelevelagreement.icc
/usr/include/pingerhost.h
/usr/include/pingerhost.icc
/usr/include/roundtriptimepinger.h
/usr/include/roundtriptimepinger.icc
/usr/include/sessiondescription.h
/usr/include/streamdescription.h


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


%changelog
* Thu Nov 23 2017 Thomas Dreibholz <dreibh@simula.no> 2.0.0~beta4
- Initial RPM release
