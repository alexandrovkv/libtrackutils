/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils library, GPX parser.
 *
 * $Id$
 *
 * $Log:
 *
 */

char *gpx_c_cvsid =
    "$Id$";

/**
 * @file gpx.c GPX parser implementation.
 */


#include <string.h>
#include <math.h>

#include "gpx.h"
#include "track_priv.h"


//#define PARSE_GPX_WAYPOINTS


static int trk_parse_gpx_track( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_gpx_track_segment( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_gpx_point( track_t track, xmlDocPtr doc, xmlNodePtr node );
static int trk_parse_gpx_extensions( track_t       track,
				     xmlDocPtr     doc,
				     xmlNodePtr    node,
				     double      * speed,
				     double      * azimuth,
				     const char ** address );
static int trk_parse_gpx_trackpoint_extension( track_t      track,
					       xmlDocPtr    doc,
					       xmlNodePtr   node,
					       double     * speed,
					       double     * azimuth );
static int trk_parse_gpx_waypoint_extension( track_t       track,
					     xmlDocPtr     doc,
					     xmlNodePtr    node,
					     const char ** address );
static int trk_parse_gpx_waypoint_address( track_t       track,
					   xmlDocPtr     doc,
					   xmlNodePtr    node,
					   const char ** address );



int trk_parse_gpx( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"trk" ) ) {
	    if( !trk_parse_gpx_track( track, doc, node ) ) {
		return 0;
	    }
#ifdef PARSE_GPX_WAYPOINTS
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"wpt" ) ) {
	    if( !trk_parse_gpx_point( track, doc, node ) ) {
		return 0;
	    }
#endif
	}
    }

    return 1;
}

static int trk_parse_gpx_track( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"name" ) ) {
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"trkseg" ) ) {
	    if( !trk_parse_gpx_track_segment( track, doc, node ) )
		return 0;
	}
    }

    return 1;
}

static int trk_parse_gpx_track_segment( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"trkpt" ) ) {
	    if( !trk_parse_gpx_point( track, doc, node ) )
		return 0;
	}
    }

    return 1;
}

static int trk_parse_gpx_point( track_t track, xmlDocPtr doc, xmlNodePtr node )
{
    xmlChar *lat, *lng;
    double latitude = NAN, longitude = NAN, altitude = NAN, azimuth = NAN, speed = NAN;
    time_t time = 0;
    int nsat = -1, fix_type = -1;
    double hdop = NAN, vdop = NAN, pdop = NAN;

    lat = xmlGetProp( node, ( const xmlChar * )"lat" );
    if( !lat )
	return 1;

    latitude = atof( ( const char * )lat );
    xmlFree( lat );

    lng = xmlGetProp( node, ( const xmlChar * )"lon" );
    if( !lng )
	return 1;

    longitude = atof( ( const char * )lng );
    xmlFree( lng );

    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"ele" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    altitude = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"time" ) ) {
	    xmlChar *val;
	    struct tm tm;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );

	    strptime( ( const char * )val, "%FT%TZ", &tm );
	    tm.tm_isdst = -1;
	    time = mktime( &tm );

	    xmlFree( val );

	    if( time == -1 )
		return 1;
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"course" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    azimuth = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"speed" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    speed = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"sat" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    nsat = atoi( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"fix" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    if( !xmlStrcmp( node->name, ( const xmlChar * )"2D" ) )
		fix_type = 2;
	    else if( !xmlStrcmp( node->name, ( const xmlChar * )"3D" ) )
		fix_type = 3;

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"hdop" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    hdop = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"vdop" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    vdop = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"pdop" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    pdop = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"extensions" ) ) {
	    if( !trk_parse_gpx_extensions( track, doc, node, &speed, &azimuth, NULL ) )
		return 1;
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

static int trk_parse_gpx_extensions( track_t       track,
				     xmlDocPtr     doc,
				     xmlNodePtr    node,
				     double      * speed,
				     double      * azimuth,
				     const char ** address )
{
    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"TrackPointExtension" ) ) {
	    if( !trk_parse_gpx_trackpoint_extension( track, doc, node, speed, azimuth ) )
		return 0;
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"WaypointExtension" ) ) {
	    if( !trk_parse_gpx_waypoint_extension( track, doc, node, address ) )
		return 0;
	}
    }

    return 1;
}

static int trk_parse_gpx_trackpoint_extension( track_t      track,
					       xmlDocPtr    doc,
					       xmlNodePtr   node,
					       double     * speed,
					       double     * azimuth )
{
    double spd = NAN, az = NAN;

    ( void )track;

    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"speed" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    spd = atof( ( const char * )val );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"course" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    az = atof( ( const char * )val );

	    xmlFree( val );
	}
    }

    if( speed )
	*speed = spd;
    if( azimuth )
	*azimuth = az;

    return 1;
}

static int trk_parse_gpx_waypoint_extension( track_t       track,
					     xmlDocPtr     doc,
					     xmlNodePtr    node,
					     const char ** address )
{
    ( void )track;

    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"Address" ) ) {
	    if( !trk_parse_gpx_waypoint_address( track, doc, node, address ) )
		return 0;
	}
    }

    return 1;
}

static int trk_parse_gpx_waypoint_address( track_t       track,
					   xmlDocPtr     doc,
					   xmlNodePtr    node,
					   const char ** address )
{
    char *addr = NULL;
    int xmlLen;
    size_t len;

    ( void )track;

    for( node = node->xmlChildrenNode; node; node = node->next ) {
	if( !xmlStrcmp( node->name, ( const xmlChar * )"StreetAddress" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    xmlLen = xmlStrlen( val );
	    len = addr ? strlen( addr ) : 0;
	    addr = realloc( addr, xmlLen + len + 1 );
	    strncpy( addr + len, ( const char * )val, xmlLen );
	    strncpy( addr + len + xmlLen, " ", 1 );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"City" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    xmlLen = xmlStrlen( val );
	    len = addr ? strlen( addr ) : 0;
	    addr = realloc( addr, xmlLen + len + 1 );
	    strncpy( addr + len, ( const char * )val, xmlLen );
	    strncpy( addr + len + xmlLen, " ", 1 );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"Country" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    xmlLen = xmlStrlen( val );
	    len = addr ? strlen( addr ) : 0;
	    addr = realloc( addr, xmlLen + len + 1 );
	    strncpy( addr + len, ( const char * )val, xmlLen );
	    strncpy( addr + len + xmlLen, " ", 1 );

	    xmlFree( val );
	} else if( !xmlStrcmp( node->name, ( const xmlChar * )"PostalCode" ) ) {
	    xmlChar *val;

	    val = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
	    xmlLen = xmlStrlen( val );
	    len = addr ? strlen( addr ) : 0;
	    addr = realloc( addr, xmlLen + len + 1 );
	    strncpy( addr + len, ( const char * )val, xmlLen );
	    strncpy( addr + len + xmlLen, " ", 1 );

	    xmlFree( val );
	}
    }

    if( address )
	*address = addr;
    else
	free( addr );

    return 1;
}

