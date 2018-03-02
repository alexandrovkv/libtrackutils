/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils library, GPX parser.
 *
 * $Id$
 *
 * $Log:
 *
 */

/**
 * @file gpx.h GPX parser header.
 */

#ifndef GPX_H_INCLUDED
#define GPX_H_INCLUDED


#include <libxml/parser.h>

#include "track.h"


int trk_parse_gpx( track_t track, xmlDocPtr doc, xmlNodePtr node );


#endif

