# Created by: Thomas Dreibholz <dreibh@iem.uni-due.de>
# $FreeBSD$

PORTNAME=	rtpaudio
PORTVERSION=	2.0.1
CATEGORIES=	net
MASTER_SITES=	https://www.uni-due.de/~be0001/rtpaudio/download/

MAINTAINER=	dreibh@iem.uni-due.de
COMMENT=	RTP Real-Time Audio Streaming System

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/COPYING

USES=           tar:xz cmake

CMAKE_ARGS+=    -DUSE_KERNEL_SCTP=1 -DENABLE_CSP=1 -DENABLE_QT=1
USE_QT5=        buildtools_build core gui widgets

.include <bsd.port.mk>
