/*
 *  $Id: tdin6.h,v 1.1.1.1 2002/06/17 14:16:47 dreibh Exp $
 *
 * SCTP implementation according to RFC 2960.
 * Copyright (C) 1999-2002 by Thomas Dreibholz
 *
 * Realized in co-operation between Siemens AG
 * and University of Essen, Institute of Computer Networking Technology.
 *
 * Acknowledgement
 * This work was partially funded by the Bundesministerium fr Bildung und
 * Forschung (BMBF) of the Federal Republic of Germany (F�derkennzeichen 01AK045).
 * The authors alone are responsible for the contents.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * There are two mailinglists available at http://www.sctp.de which should be
 * used for any discussion related to this implementation.
 *
 * Contact: discussion@sctp.de
 *          dreibh@exp-math.uni-essen.de
 *
 * Purpose: IPv6 definitions
 *
 */


/**
  * IMPORTANT NOTE:
  * This is an extraction of <linux/in6.h>, which cannot be included due
  * to incompatibilities!
  * This file will be replaced, when incompatibilities are solved!
  */


#include "tdsystem.h"


#ifndef TDIN6_H
#define TDIN6_H


#if (SYSTEM == OS_Linux)


#include <linux/types.h>


struct in6_flowlabel_req
{
   struct in6_addr   flr_dst;
   __u32 flr_label;
   __u8  flr_action;
   __u8  flr_share;
   __u16 flr_flags;
   __u16    flr_expires;
   __u16 flr_linger;
   __u32 __flr_pad;
   /* Options in format of IPV6_PKTOPTIONS */
};

#define IPV6_FL_A_GET   0
#define IPV6_FL_A_PUT   1
#define IPV6_FL_A_RENEW 2

#define IPV6_FL_F_CREATE   1
#define IPV6_FL_F_EXCL     2

#define IPV6_FL_S_NONE     0
#define IPV6_FL_S_EXCL     1
#define IPV6_FL_S_PROCESS  2
#define IPV6_FL_S_USER     3
#define IPV6_FL_S_ANY      255


/*
 * Bitmask constant declarations to help applications select out the
 * flow label and priority fields.
 *
 * Note that this are in host byte order while the flowinfo field of
 * sockaddr_in6 is in network byte order.
 */

#define IPV6_FLOWINFO_FLOWLABEL     0x000fffff
#define IPV6_FLOWINFO_PRIORITY      0x0ff00000

/*
 * IPv6 TLV options.
 */
#define IPV6_TLV_PAD0      0
#define IPV6_TLV_PADN      1
#define IPV6_TLV_ROUTERALERT  5
#define IPV6_TLV_JUMBO     194

/*
 * IPV6 socket options
 */

/* Flowlabel */
#define IPV6_FLOWINFO      11
#define IPV6_FLOWLABEL_MGR 32
#define IPV6_FLOWINFO_SEND 33


#endif


#endif
