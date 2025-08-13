# Created by: Thomas Dreibholz <thomas.dreibholz@gmail.com>

PORTNAME=	rtpaudio
DISTVERSION=	2.0.8
CATEGORIES=	net
MASTER_SITES=	https://www.nntb.no/~dreibh/rtpaudio/download/

MAINTAINER=	thomas.dreibholz@gmail.com
COMMENT=	RTP Real-Time Audio Streaming System
WWW=		https://www.nntb.no/~dreibh/rtpaudio/

LICENSE=	GPLv3+
LICENSE_FILE=	${WRKSRC}/COPYING

LIB_DEPENDS=    libpulse.so:audio/pulseaudio

USES=           cmake compiler:c++17-lang gl qt:6
USE_GL=         gl opengl
USE_QT=         base tools:build

CMAKE_ARGS+=    -DUSE_KERNEL_SCTP=1 -DENABLE_CSP=1 -DENABLE_QT=1
USE_LDCONFIG=   yes

PLIST_SUB=      VERSION=${PORTVERSION}

.include <bsd.port.mk>
