/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils library, TCX parser.
 *
 */

/**
 * @file tcx.c TCX parser implementation.
 */


#include <string.h>
#include <math.h>

#include "tcx.h"
#include "track_priv.h"


static int trk_parse_tcx_activities( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_tcx_activity( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_tcx_activity_lap( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_tcx_track( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_tcx_trackpoint( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_tcx_trackpoint_position( track_t      track,
					      xmlDocPtr    doc,
					      xmlNodePtr   node,
					      double     * latitude,
					      double     * longitude );
static int trk_parse_tcx_trackpoint_extensions( track_t      track,
						xmlDocPtr    doc,
						xmlNodePtr   node,
						double     * speed );


int trk_parse_tcx( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"Author" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Folders" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Activities" ) ) {
	    if( !trk_parse_tcx_activities( track, doc, node ) ) {
		return 0;
	    }
	}
    }

    return 1;
}

static int trk_parse_tcx_activities( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"Activity" ) ) {
	    if( !trk_parse_tcx_activity( track, doc, node ) ) {
		return 0;
	    }
	}
    }

    return 1;
}

static int trk_parse_tcx_activity( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"Id" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Creator" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Lap" ) ) {
	    if( !trk_parse_tcx_activity_lap( track, doc, node ) ) {
		return 0;
	    }
	}
    }

    return 1;
}

static int trk_parse_tcx_activity_lap( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"TotalTimeSeconds" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"DistanceMeters" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"MaximumSpeed" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Calories" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"AverageHeartRateBpm" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"MaximumHeartRateBpm" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Intensity" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"TriggerMethod" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Track" ) ) {
	    if( !trk_parse_tcx_track( track, doc, node ) ) {
		return 0;
	    }
	}
    }

    return 1;
}

static int trk_parse_tcx_track( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"Trackpoint" ) ) {
	    if( !trk_parse_tcx_trackpoint( track, doc, node ) ) {
		return 0;
	    }
	}
    }

    return 1;
}

static int trk_parse_tcx_trackpoint( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    double latitude = NAN, longitude = NAN, altitude = NAN, azimuth = NAN, speed = NAN;
    time_t time = 0;
    int nsat = -1, fix_type = -1;
    double hdop = NAN, vdop = NAN, pdop = NAN;

    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"Time" ) ) {
	    xmlChar *val;
	    struct tm tm;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );

	    strptime( ( const char * )val, "%FT%TZ", &tm );
	    tm.tm_isdst = -1;
	    time = mktime( &tm );

	    xmlFree( val );

	    if( time == -1 )
		return 1;
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Position" ) ) {
	    if( !trk_parse_tcx_trackpoint_position( track, doc, node,
						    &latitude, &longitude ) ) {
		return 1;
	    }
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"AltitudeMeters" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    altitude = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Extensions" ) ) {
	    if( !trk_parse_tcx_trackpoint_extensions( track, doc, node, &speed ) ) {
		return 1;
	    }
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"DistanceMeters" ) ) {
	}
    }

    return trk_add_point( track,
			  time,
			  latitude,
			  longitude,
			  altitude,
			  azimuth,
			  speed,
			  nsat,
			  fix_type,
			  hdop,
			  vdop,
			  pdop );
}

static int trk_parse_tcx_trackpoint_position( track_t      track,
					      xmlDocPtr    doc,
					      xmlNodePtr   node,
					      double     * latitude,
					      double     * longitude )
{
    double lat = NAN, lng = NAN;

    ( void )track;

    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"LatitudeDegrees" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    lat = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"LongitudeDegrees" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    lng = atof( ( const char * )val );

	    xmlFree( val );
	}
    }

    if( latitude )
	*latitude = lat;
    if( longitude )
	*longitude = lng;

    return 1;
}

static int trk_parse_tcx_trackpoint_extensions( track_t      track,
						xmlDocPtr    doc,
						xmlNodePtr   node,
						double     * speed )
{
    double spd = NAN;

    ( void )track;

    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"Speed" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    spd = atof( ( const char * )val );

	    xmlFree( val );
	}
    }

    if( speed )
	*speed = spd;

    return 1;
}

