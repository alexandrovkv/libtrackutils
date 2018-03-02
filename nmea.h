/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils library, NMEA parser.
 *
 * $Id$
 *
 * $Log:
 *
 */

/**
 * @file nmea.h NMEA parser header.
 */

#ifndef NMEA_H_INCLUDED
#define NMEA_H_INCLUDED


#include "track.h"


int trk_parse_nmea( track_t track, void * data, size_t size );


#endif

