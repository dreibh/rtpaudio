# Created by: Thomas Dreibholz <dreibh@iem.uni-due.de>
# $FreeBSD$

PORTNAME=	rtpaudio
PORTVERSION=	2.0.2
CATEGORIES=	net
MASTER_SITES=	https://www.uni-due.de/~be0001/rtpaudio/download/

MAINTAINER=	dreibh@iem.uni-due.de
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
