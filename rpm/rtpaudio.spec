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
BuildRequires: pulse-devel
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

%files
%defattr(-,root,root,-)
%{_libdir}/libmpegsound.so*


%package libmpegsound-devel
Summary: Headers for the sound decoder library
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description libmpegsound-devel
 libmpegsound decodes a couple of sound formats (e.g. MP3) and
 returns a raw data stream.
 .
 The library is provided by this package.

%files
%defattr(-,root,root,-)
%{_libdir}/libmpegsound.so*
%{_includedir}/mpegsound.h
%{_includedir}/mpegsound_locals.h
%{_libdir}/libmpegsound*.a
%{_libdir}/libmpegsound*.so




# %package devel
# Summary: Development files for rsplib
# Group: Development/Libraries
# Requires: %{name} = %{version}-%{release}
# 
# %description devel
# The RTP Audio system is a network sound streaming systen. It has been designed for QoS performance analysis and teaching purposes. RTP Audio supports IPv4 and IPv6 including flowlabels and traffic classes, QoS management as well as transport via UDP and SCTP.
# This package provides header files for the libraries. You need them to develop your own RTP-Audio-based clients and servers.
# 
# 
# %package docs
# Summary: Documentation files for rsplib
# Group: System Environment/Libraries
# Requires: %{name} = %{version}-%{release}
# 
# %description docs
# The RTP Audio system is a network sound streaming systen. It has been designed for QoS performance analysis and teaching purposes. RTP Audio supports IPv4 and IPv6 including flowlabels and traffic classes, QoS management as well as transport via UDP and SCTP.
# This package contains the documentation for RTP Audio.
# 
# 
# %package registrar
# Summary: RSerPool Registrar service
# Group: Applications/Internet
# Requires: %{name} = %{version}-%{release}
# Requires: %{name}-docs
# 
# %description registrar
# Reliable Server Pooling (RSerPool) is the IETF's standard (RFC 5351 to RFC 5356) for a lightweight server pool and session management framework. It provides highly available pool management (that is registration handling and load distribution/balancing) by components called Registrar and a client-side/server-side API for accessing the service of a pool.
# This package provides the registrar, which is the management component for RSerPool-based server pools. You need at least one registrar in a setup, but for redundancy reasons, you should have at least two.
# 
# 
# %package tools
# Summary: RSerPool test tools
# Group: Applications/Internet
# Requires: %{name} = %{version}-%{release}
# Requires: %{name}-docs
# Requires: chrpath
# 
# %description tools
# Reliable Server Pooling (RSerPool) is the IETF's standard (RFC 5351 to RFC 5356) for a lightweight server pool and session management framework. It provides highly available pool management (that is registration handling and load distribution/balancing) by components called Registrar and a client-side/server-side API for accessing the service of a pool.
# This package provides some test tools for RSerPool setups.
# 
# 
# %package services
# Summary: RSerPool example services
# Group: Applications/Internet
# Requires: %{name} = %{version}-%{release}
# Requires: %{name}-tools
# 
# %description services
# Reliable Server Pooling (RSerPool) is the IETF's standard (RFC 5351 to RFC 5356) for a lightweight server pool and session management framework. It provides highly available pool management (that is registration handling and load distribution/balancing) by components called Registrar and a client-side/server-side API for accessing the service of a pool.
# This package provides a set of input files for the Fractal Generator service.
# 
# 
# %prep
# %setup -q
# 
# %build
# autoreconf -if
# 
# %configure --disable-maintainer-mode --enable-kernel-sctp --enable-qt --enable-csp --prefix=/usr
# sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=""|g' libtool
# sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool
# make %{?_smp_mflags}
# 
# %install
# make install DESTDIR=%{buildroot}
# 
# %clean
# rm -rf "$RPM_BUILD_ROOT"
# 
# %files
# %defattr(-,root,root,-)
# %{_libdir}/librspcsp.so*
# %{_libdir}/librspdispatcher.so*
# %{_libdir}/librsphsmgt.so*
# %{_libdir}/librsplib.so*
# %{_libdir}/librspmessaging.so*
# %{_libdir}/libtdbreakdetector.so*
# %{_libdir}/libtdloglevel.so*
# %{_libdir}/libtdnetutilities.so*
# %{_libdir}/libtdrandomizer.so*
# %{_libdir}/libtdstorage.so*
# %{_libdir}/libtdstringutilities.so*
# %{_libdir}/libtdtagitem.so*
# %{_libdir}/libtdthreadsafety.so*
# %{_libdir}/libtdtimeutilities.so*
# %{_libdir}/libcpprspserver.so*
# %{_libdir}/libtdcppthread.so*
# 
# %files devel
# %{_includedir}/rtpaudio/rtpaudio-internals.h
# %{_includedir}/rtpaudio/rtpaudio-policytypes.h
# %{_includedir}/rtpaudio/rtpaudio.h
# %{_includedir}/rtpaudio/rtpaudio-csp.h
# %{_includedir}/rtpaudio/tagitem.h
# %{_includedir}/rtpaudio/cpprspserver.h
# %{_includedir}/rtpaudio/mutex.h
# %{_includedir}/rtpaudio/tcplikeserver.h
# %{_includedir}/rtpaudio/thread.h
# %{_includedir}/rtpaudio/udplikeserver.h
# %{_libdir}/librspcsp*.a
# %{_libdir}/librspcsp*.la
# %{_libdir}/librspcsp*.so
# %{_libdir}/librspdispatcher*.a
# %{_libdir}/librspdispatcher*.la
# %{_libdir}/librspdispatcher*.so
# %{_libdir}/librsphsmgt*.a
# %{_libdir}/librsphsmgt*.la
# %{_libdir}/librsphsmgt*.so
# %{_libdir}/librsplib*.a
# %{_libdir}/librsplib*.la
# %{_libdir}/librsplib*.so
# %{_libdir}/librspmessaging*.a
# %{_libdir}/librspmessaging*.la
# %{_libdir}/librspmessaging*.so
# %{_libdir}/libtdbreakdetector*.a
# %{_libdir}/libtdbreakdetector*.la
# %{_libdir}/libtdbreakdetector*.so
# %{_libdir}/libtdloglevel*.a
# %{_libdir}/libtdloglevel*.la
# %{_libdir}/libtdloglevel*.so
# %{_libdir}/libtdnetutilities*.a
# %{_libdir}/libtdnetutilities*.la
# %{_libdir}/libtdnetutilities*.so
# %{_libdir}/libtdrandomizer*.a
# %{_libdir}/libtdrandomizer*.la
# %{_libdir}/libtdrandomizer*.so
# %{_libdir}/libtdstorage*.a
# %{_libdir}/libtdstorage*.la
# %{_libdir}/libtdstorage*.so
# %{_libdir}/libtdstringutilities*.a
# %{_libdir}/libtdstringutilities*.la
# %{_libdir}/libtdstringutilities*.so
# %{_libdir}/libtdtagitem*.a
# %{_libdir}/libtdtagitem*.la
# %{_libdir}/libtdtagitem*.so
# %{_libdir}/libtdthreadsafety*.a
# %{_libdir}/libtdthreadsafety*.la
# %{_libdir}/libtdthreadsafety*.so
# %{_libdir}/libtdtimeutilities*.a
# %{_libdir}/libtdtimeutilities*.la
# %{_libdir}/libtdtimeutilities*.so
# %{_libdir}/libcpprspserver.so.*
# %{_libdir}/libtdcppthread.so.*
# %{_libdir}/libcpprspserver*.a
# %{_libdir}/libcpprspserver*.la
# %{_libdir}/libcpprspserver*.so
# %{_libdir}/libtdcppthread*.a
# %{_libdir}/libtdcppthread*.la
# %{_libdir}/libtdcppthread*.so
# 
# %files docs
# %doc docs/Handbook.pdf
# 
# %files registrar
# %{_bindir}/rspregistrar
# %{_datadir}/man/man1/rspregistrar.1.gz
# 
# %files tools
# %{_bindir}/cspmonitor
# %{_bindir}/hsdump
# %{_bindir}/rspserver
# %{_bindir}/rspterminal
# %{_datadir}/man/man1/rspserver.1.gz
# %{_datadir}/man/man1/rspterminal.1.gz
# %{_datadir}/man/man1/cspmonitor.1.gz
# %{_datadir}/man/man1/hsdump.1.gz
# 
# %files services
# %{_bindir}/calcappclient
# %{_bindir}/fractalpooluser
# %{_bindir}/pingpongclient
# %{_bindir}/scriptingclient
# %{_bindir}/scriptingcontrol
# %{_bindir}/scriptingserviceexample
# %{_datadir}/man/man1/calcappclient.1.gz
# %{_datadir}/man/man1/fractalpooluser.1.gz
# %{_datadir}/man/man1/pingpongclient.1.gz
# %{_datadir}/man/man1/scriptingclient.1.gz
# %{_datadir}/man/man1/scriptingcontrol.1.gz
# %{_datadir}/man/man1/scriptingserviceexample.1.gz
# %{_datadir}/fractalpooluser/*.qm
# %{_datadir}/fgpconfig/*.fsf


%changelog
* Thu Nov 23 2017 Thomas Dreibholz <dreibh@simula.no> 2.0.0~beta4
- Initial RPM release
