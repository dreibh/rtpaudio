// ##########################################################################
// ####                                                                  ####
// ####                      RTP Audio Server Project                    ####
// ####                    ============================                  ####
// ####                                                                  ####
// #### Media Info                                                       ####
// ####                                                                  ####
// #### Version 1.50  --  August 01, 2001                                ####
// ####                                                                  ####
// ####            Copyright (C) 1999-2001 by Thomas Dreibholz           ####
// #### Contact:                                                         ####
// ####    EMail: dreibh@exp-math.uni-essen.de                           ####
// ####    WWW:   http://www.exp-math.uni-essen.de/~dreibh/rtpaudio      ####
// ####                                                                  ####
// #### ---------------------------------------------------------------- ####
// ####                                                                  ####
// #### This program is free software; you can redistribute it and/or    ####
// #### modify it under the terms of the GNU General Public License      ####
// #### as published by the Free Software Foundation; either version 2   ####
// #### of the License, or (at your option) any later version.           ####
// ####                                                                  ####
// #### This program is distributed in the hope that it will be useful,  ####
// #### but WITHOUT ANY WARRANTY; without even the implied warranty of   ####
// #### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ####
// #### GNU General Public License for more details.                     ####
// ####                                                                  ####
// ##########################################################################


#ifndef MEDIAINFO_H
#define MEDIAINFO_H


#include "tools.h"


/**
  * Definition of encoder errors.
  */
enum MediaError
{
   ME_NoError            = 0,
   ME_NoMedia            = 1,
   ME_EOF                = 2,

   ME_UnrecoverableError = 20,
   ME_BadMedia           = ME_UnrecoverableError + 0,
   ME_ReadError          = ME_UnrecoverableError + 1,
   ME_OutOfMemory        = ME_UnrecoverableError + 2,
};


/**
  * Constant for position steps per second: 1 step = 1 nanosecond;
  */
const card64 PositionStepsPerSecond = (card64)1000000000;



/**
  * This class contains information on a media.
  *
  * @short   Media Info
  * @author  Thomas Dreibholz (dreibh@exp-math.uni-essen.de)
  * @version 1.0
  */
class MediaInfo
{
   // ====== Constructor ====================================================
   /**
     * Constructor.
     */
   public:
   MediaInfo();


   // ====== Reset ==========================================================
   /**
     * Reset.
     */
   void reset();


   /**
     * Translate byte order.
     */
   void translate();


   // ====== MediaInfo data =================================================
   /**
     * Start time stamp of the media.
     */
   card64 StartTimeStamp;

   /**
     * End time stamp of the media.
     */
   card64 EndTimeStamp;


   /**
     * Constant for the maximum title length.
     */
   static const cardinal MaxTitleLength   = 47;

   /**
     * Constant for the maximum author length.
     */
   static const cardinal MaxArtistLength  = 47;

   /**
     * Constant for the maximum comment length.
     */
   static const cardinal MaxCommentLength = 47;


   /**
     * Title string.
     */
   char Title[MaxTitleLength + 1];

   /**
     * Artist string.
     */
   char Artist[MaxArtistLength + 1];

   /**
     * Comment string.
     */
   char Comment[MaxCommentLength + 1];
};


/**
  * Output operator.
  */
std::ostream& operator<<(std::ostream& os, const MediaInfo& mi);


#endif
