Name: rtpaudio
Version: 2.0.0~beta4
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
Requires: %{name} = %{version}-%{release}

%description libmpegsound
 libmpegsound decodes a couple of sound formats (e.g. MP3) and
 returns a raw data stream.
 .
 The library is provided by this package.

%files libmpegsound
%defattr(-,root,root,-)
/usr/lib/libmpegsound.so*


%package libmpegsound-devel
Summary: Development files for the sound decoder library
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description libmpegsound-devel
 libmpegsound decodes a couple of sound formats (e.g. MP3) and
 returns a raw data stream.
 .
 The library is provided by this package.

%files libmpegsound-devel
%defattr(-,root,root,-)
/usr/include/mpegsound.h
/usr/include/mpegsound_locals.h
/usr/lib/libmpegsound.so*
/usr/lib/libmpegsound*.a
/usr/lib/libmpegsound*.so


%package libtdtoolbox
Summary: Shared libraries for RTP Audio (common helper functions)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description libtdtoolbox
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides a shared library for common helper functions.

%files libtdtoolbox
%defattr(-,root,root,-)
/usr/lib/libtdtoolbox.so*


%package libmediainfo
Summary: Shared libraries for RTP Audio (media information handling)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description libmediainfo
Shared libraries for RTP Audio (media information handling)
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides a shared library for media information handling.

%files libmediainfo
%defattr(-,root,root,-)
/usr/lib/libmediainfo.so*


%package libaudiocommon
Summary:   Shared libraries for RTP Audio (common audio data handling)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: %{name}-libtdtoolbox

%description libaudiocommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common audio data handling.

%files libaudiocommon
%defattr(-,root,root,-)
/usr/lib/libaudiocommon.so*


%package libaudioreader
Summary: Shared libraries for RTP Audio (audio input reading)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: %{name}libaudiocommon
Requires: %{name}libmediainfo
Requires: %{name}libmpegsound
Requires: %{name}libtdtoolbox

%description libaudioreader
The RTP Audio system is a network sound streaming system. It has been
designed for QoS performance analysis and teaching purposes.
.
This package provides a shared library for audio input reading.

%files libaudioreader
%defattr(-,root,root,-)
/usr/lib/libaudioreader.so*


%package libaudiowriter
Summary: Shared libraries for RTP Audio (audio output writing)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: %{name}-libaudiocommon
Requires: %{name}-libpulse
Requires: %{name}-libtdtoolbox

%description libaudiowriter
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for audio output writing.

%files libaudiowriter
%defattr(-,root,root,-)
/usr/lib/libaudiowriter.so*


%package libaudiocodeccommon
Summary: Shared libraries for RTP Audio (common audio codec handling)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: %{name}-libaudiocommon

%description libaudiocodeccommon
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for common audio codec handling.

%files libaudiocodeccommon
%defattr(-,root,root,-)
/usr/lib/libaudiocodeccommon.so*


%package libaudioencoder
Summary: Shared libraries for RTP Audio (audio encoding)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: %{name}-libaudiocodeccommon
Requires: %{name}-libaudiocommon

%description libaudioencoder
 The RTP Audio system is a network sound streaming system. It has been
 designed for QoS performance analysis and teaching purposes.
 .
 This package provides a shared library for audio encoding.

%files libaudioencoder
%defattr(-,root,root,-)
/usr/lib/libaudioencoder.so*


%changelog
* Thu Nov 23 2017 Thomas Dreibholz <dreibh@simula.no> 2.0.0~beta4
- Initial RPM release
