/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track point.
 *
 * $Id$
 *
 * $Log:
 *
 */

/**
 * @file point.h Track point header.
 */

#ifndef POINT_H_INCLUDED
#define POINT_H_INCLUDED


#include <time.h>


typedef struct point_o * point_t;

struct point_o {
    time_t   time;
    double   latitude;
    double   longitude;
    double   altitude;
    double   azimuth;
    double   speed;
    int      nsat;
    int      fix_type;
    double   hdop;
    double   vdop;
    double   pdop;
};


point_t trk_point_make( time_t  time,
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

void trk_point_free( point_t point );


#endif

