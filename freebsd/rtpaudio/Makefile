# Created by: Thomas Dreibholz <thomas.dreibholz@gmail.com>
# $FreeBSD$

PORTNAME=	rtpaudio
PORTVERSION=	2.0.7~rc1
CATEGORIES=	net
MASTER_SITES=	https://www.nntb.no/~dreibh/rtpaudio/download/

MAINTAINER=	thomas.dreibholz@gmail.com
COMMENT=	RTP Real-Time Audio Streaming System

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/COPYING

LIB_DEPENDS=    libpulse.so:audio/pulseaudio

USES=           cmake qt:5 tar:xz

CMAKE_ARGS+=    -DUSE_KERNEL_SCTP=1 -DENABLE_CSP=1 -DENABLE_QT=1
USE_LDCONFIG=   yes
USE_QT=         buildtools_build core gui widgets qmake_build

PLIST_SUB=      VERSION=${PORTVERSION}

.include <bsd.port.mk>
