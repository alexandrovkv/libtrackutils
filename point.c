/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track point.
 *
 * $Id$
 *
 * $Log:
 *
 */

char *point_c_cvsid =
    "$Id$";

/**
 * @file point.c Track point.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "point.h"



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
			double  pdop )
{
    point_t point;

    point = malloc( sizeof( *point ) );
    if( !point )
	return NULL;

    point->time      = time;
    point->latitude  = latitude;
    point->longitude = longitude;
    point->altitude  = altitude;
    point->azimuth   = azimuth;
    point->speed     = speed;
    point->nsat      = nsat;
    point->fix_type  = fix_type;
    point->hdop      = hdop;
    point->vdop      = vdop;
    point->pdop      = pdop;

    return point;
}

void trk_point_free( point_t point )
{
    if( !point )
	return;

    free( point );
}

