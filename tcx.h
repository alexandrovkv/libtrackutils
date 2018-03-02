/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils library, TCX parser.
 *
 */

/**
 * @file tcx.h TCX parser header.
 */

#ifndef TCX_H_INCLUDED
#define TCX_H_INCLUDED


#include <libxml/parser.h>

#include "track.h"


int trk_parse_tcx( track_t track, xmlDocPtr doc, xmlNodePtr node );


#endif
