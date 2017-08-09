# New ports collection makefile for: rtpaudio
# Date created:                      27 July 2010
# Whom:                              Thomas Dreibholz <dreibh@iem.uni-due.de>
#
# $FreeBSD: ports/net/rtpaudio/Makefile,v 1.1 2010/08/04 20:00:20 ohauer Exp $
#

PORTNAME=	rtpaudio
PORTVERSION=	2.0.0~alpha1.10
CATEGORIES=	net
MASTER_SITES=	https://www.uni-due.de/~be0001/rtpaudio/download/ \
		http://www.exp-math.uni-essen.de/~dreibh/rtpaudio/download/

MAINTAINER=	dreibh@iem.uni-due.de
COMMENT=	RTP Real-Time Audio Streaming System

LIB_DEPENDS=

USE_QT_VER=     4
QT_COMPONENTS=  gui moc_build
# xml

GNU_CONFIGURE=	yes
CONFIGURE_ARGS=	--disable-maintainer-mode \
                --enable-kernel-sctp \
                --enable-qt
USE_GMAKE=	yes

LICENSE=	GPLv3
LICENSE_FILE=	${WRKSRC}/COPYING

PLIST_FILES=	bin/rtpa-server bin/rtpa-client bin/rtpa-qclient bin/rtpa-vclient
MAN1=		rtpa-server.1 rtpa-client.1 rtpa-qclient.1 rtpa-vclient.1

.include <bsd.port.mk>
