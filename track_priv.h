/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils
 *
 * $Id$
 *
 * $Log:
 *
 */

#ifndef TRACK_PRIV_H_INCLUDED
#define TRACK_PRIV_H_INCLUDED


#include <time.h>

#include "track.h"


#define TU_EXPORT __attribute__ ((visibility("default")))


int trk_add_point( track_t track,
		   time_t  time,
		   double  latitude,
		   double  longitude,
		   double  altitude,
		   double  azimuth,
		   double  speed,
		   int     nsat,
		   int     fix_type,
		   double  hdop,
		   double  vdop,
		   double  pdop );


#endif

