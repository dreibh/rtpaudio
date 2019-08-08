#
# $FreeBSD: ports/net/rtpaudio/Makefile,v 1.1 2010/08/04 20:00:20 ohauer Exp $
#

PORTNAME=	rtpaudio
PORTVERSION=	2.0.1
CATEGORIES=	net
MASTER_SITES=	https://www.uni-due.de/~be0001/rtpaudio/download/
EXTRACT_SUFX=	.tar.xz

MAINTAINER=	dreibh@iem.uni-due.de
COMMENT=	RTP Real-Time Audio Streaming System

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/COPYING

GNU_CONFIGURE=  yes
USES=           cmake
CMAKE_ARGS+=    -DUSE_KERNEL_SCTP=1 -DENABLE_CSP=1 -DENABLE_QT=1
USE_QT5=        buildtools_build core gui widgets

PLIST_FILES=	bin/rtpa-server bin/rtpa-client bin/rtpa-qclient bin/rtpa-vclient
MAN1=		rtpa-server.1 rtpa-client.1 rtpa-qclient.1 rtpa-vclient.1

.include <bsd.port.mk>
