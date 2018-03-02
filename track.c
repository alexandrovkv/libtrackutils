/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils library.
 *
 * $Id$
 *
 * $Log:
 *
 */

char *track_c_cvsid =
    "$Id$";

/**
 * @file track.c Track utils implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <float.h>

#include <magic.h>

#include <libxml/parser.h>

#include "geodesic.h"

#include "track.h"
#include "track_priv.h"
#include "point.h"
#include "gpx.h"
#include "tcx.h"
#include "nmea.h"



static int trk_parse_data( track_t track, void * data, size_t size );
static int trk_parse_xml( track_t track, void * data, size_t size );

static char * trk_dump_point( point_t point );

static inline void trk_linear_interpolate( double   x1, double   y1,
					   double   x2, double * y2,
					   double   x3, double   y3 );
static inline void trk_ac_interpolate( double   x1, double   y1, double   v1,
				       double   x2, double * y2, double * v2,
				       double   x3, double   y3, double   v3 );



struct track_o {
    log_hndl    err_hndl;
    log_hndl    out_hndl;
    void      * env;

    time_t      start;
    time_t      end;
    point_t   * points;
    size_t      npoints;
};



TU_EXPORT track_t trk_make( log_hndl   err_hndl,
			    log_hndl   out_hndl,
			    void     * env )
{
    track_t track;

    track = malloc( sizeof( *track ) );
    if( !track )
	return NULL;

    track->err_hndl = err_hndl;
    track->out_hndl = out_hndl;
    track->env      = env;
    track->start    = 0;
    track->end      = 0;
    track->points   = NULL;
    track->npoints  = 0;

    return track;
}

TU_EXPORT void trk_drop( track_t track )
{
    if( !track )
	return;

    while( track->npoints-- )
	trk_point_free( track->points[track->npoints] );
    free( track->points );

    free( track );
}

TU_EXPORT int trk_from_file( track_t track, const char * file )
{
    void * data;
    size_t size;
    int fd;
    char msg[4096];
    int ret;

    assert( track );
    assert( file );

    fd = open( file, O_RDONLY, 0 );
    if( fd == -1 ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "can not open '%s': %s",
		      file, strerror( errno ) );
	    track->err_hndl( track->env, msg );
	}
	return 0;
    }

    size = lseek( fd, 0, SEEK_END );
    lseek( fd, 0, SEEK_SET );

    data = mmap( NULL, size, PROT_READ, MAP_SHARED, fd, 0 );
    if( data == MAP_FAILED ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "can not map '%s': %s",
		      file, strerror( errno ) );
	    track->err_hndl( track->env, msg );
	}
	close( fd );
	return 0;
    }

    close( fd );

    ret = trk_parse_data( track, data, size );

    munmap( data, size );

    return ret;
}

TU_EXPORT int trk_from_buffer( track_t track, void * buffer, size_t size )
{
    assert( track );

    return trk_parse_data( track, buffer, size );
}

TU_EXPORT int trk_get_coord_by_utime( track_t  track,
				      time_t   time,
				      double * latitude,
				      double * longitude,
				      double * altitude,
				      double * azimuth,
				      double * speed )
{
    size_t i;
    point_t cur_point, next_point;
    double lat = 0., lng = 0., d = 0., alt = NAN, spd = NAN;
    double a = 6378137, f = 1 / 298.257223563;
    double az11, az12, s12;
    struct geod_geodesic g;

    assert( track );

    if( time < track->start || time > track->end ) {
	if( track->err_hndl ) {
	    struct tm *tm;
	    char tmbuf[3][64];
	    char msg[4096];

	    tm = localtime( &time );
	    strftime( tmbuf[0], sizeof( tmbuf[0] ), "%FT%TZ", tm );
	    tm = localtime( &track->start );
	    strftime( tmbuf[1], sizeof( tmbuf[1] ), "%FT%TZ", tm );
	    tm = localtime( &track->end );
	    strftime( tmbuf[2], sizeof( tmbuf[2] ), "%FT%TZ", tm );

	    snprintf( msg, sizeof( msg ),
		      "time %s is out of track range [%s - %s]",
		      tmbuf[0], tmbuf[1], tmbuf[2] );
	    track->err_hndl( track->env, msg );
	}

	return 0;
    }

    geod_init( &g, a, f );

    for( i = 0; i < track->npoints; i++ ) {
	cur_point = track->points[i];

	if( time == cur_point->time ) {
	    lat = cur_point->latitude;
	    lng = cur_point->longitude;
	    alt = cur_point->altitude;
	    az11 = cur_point->azimuth;
	    spd = cur_point->speed;

	    if( isnan( az11 ) ) {
		next_point = track->points[i+1];
		if( next_point ) {
		    geod_inverse( &g, cur_point->latitude, cur_point->longitude,
				  next_point->latitude, next_point->longitude,
				  &s12, &az11, &az12 );
		}
	    }

	    if( isnan( spd ) ) {
		next_point = track->points[i+1];
		if( next_point ) {
		    geod_inverse( &g, cur_point->latitude, cur_point->longitude,
				  next_point->latitude, next_point->longitude,
				  &s12, &az11, &az12 );

		    spd = s12 / ( double )( next_point->time - cur_point->time );
		}
	    }

	    if( az11 < 0. )
		az11 += 360.;

	    break;
	}

	next_point = track->points[i+1];

	if( next_point && time < next_point->time ) {
	    geod_inverse( &g, cur_point->latitude, cur_point->longitude,
			  next_point->latitude, next_point->longitude,
			  &s12, &az11, &az12 );

	    trk_linear_interpolate( ( double )cur_point->time,  cur_point->altitude,
				    ( double )time,             &alt,
				    ( double )next_point->time, next_point->altitude );

	    if( isnan( cur_point->speed ) || isnan( next_point->speed ) ) {
		trk_linear_interpolate( ( double )cur_point->time,  0.,
					( double )time,             &d,
					( double )next_point->time, s12 );

		spd = s12 / ( double )( next_point->time - cur_point->time );
	    } else {
		trk_ac_interpolate( ( double )cur_point->time,  0.,  cur_point->speed,
				    ( double )time,             &d,  &spd,
				    ( double )next_point->time, s12, next_point->speed );
	    }

	    if( az11 < 0. )
		az11 += 360.;

	    geod_direct( &g, cur_point->latitude, cur_point->longitude, az11, d,
			 &lat, &lng, &az12 );

	    break;
	}
    }

    if( latitude )
	*latitude = lat;
    if( longitude )
	*longitude = lng;
    if( altitude )
	*altitude = alt;
    if( azimuth )
	*azimuth = az11;
    if( speed )
	*speed = spd;

    return 1;
}

TU_EXPORT int trk_get_coord_by_ISOdate( track_t      track,
					const char * date,
					double     * latitude,
					double     * longitude,
					double     * altitude,
					double     * azimuth,
					double     * speed )
{
    time_t time;
    struct tm tm;

    assert( track );
    assert( date );

    strptime( date, "%FT%TZ", &tm );
    tm.tm_isdst = -1;
    time = mktime( &tm );

    return trk_get_coord_by_utime( track,
				   time,
				   latitude,
				   longitude,
				   altitude,
				   azimuth,
				   speed );
}

TU_EXPORT int trk_get_track_summary( track_t  track,
				     size_t * npoints,
				     time_t * start,
				     time_t * end,
				     double * distance,
				     double * min_speed,
				     double * max_speed,
				     double * min_altitude,
				     double * max_altitude )
{
    size_t i;
    point_t cur_point, next_point;
    double s12, d = 0., avg_spd,
	min_spd = DBL_MAX, max_spd = DBL_MIN,
	min_alt = DBL_MAX, max_alt = DBL_MIN;
    double a = 6378137, f = 1 / 298.257223563;
    struct geod_geodesic g;

    assert( track );

    geod_init( &g, a, f );

    for( i = 0; i < track->npoints; i++ ) {
	cur_point = track->points[i];

	if( !isnan( cur_point->speed ) ) {
	    if( cur_point->speed < min_spd )
		min_spd = cur_point->speed;
	    if( cur_point->speed > max_spd )
		max_spd = cur_point->speed;
	}

	if( !isnan( cur_point->altitude ) ) {
	    if( cur_point->altitude < min_alt )
		min_alt = cur_point->altitude;
	    if( cur_point->altitude > max_alt )
		max_alt = cur_point->altitude;
	}

	if( i < track->npoints - 1 ) {
	    next_point = track->points[i+1];
	    if( next_point ) {
		geod_inverse( &g, cur_point->latitude, cur_point->longitude,
			      next_point->latitude, next_point->longitude,
			      &s12, NULL, NULL );

		d += s12;
	    }
	}
    }

    if( min_spd == DBL_MAX && max_spd == DBL_MIN )
	avg_spd = d / ( double )( track->end - track->start );
    else
	avg_spd = ( min_spd + max_spd ) / 2;

    if( npoints )
	*npoints = track->npoints;
    if( start )
	*start = track->start;
    if( end )
	*end = track->end;
    if( distance )
	*distance = d;
    if( min_speed )
	*min_speed = min_spd == DBL_MAX ? avg_spd : min_spd;
    if( max_speed )
	*max_speed = max_spd == DBL_MIN ? avg_spd : max_spd;
    if( min_altitude )
	*min_altitude = min_alt == DBL_MAX ? NAN : min_alt;
    if( max_altitude )
	*max_altitude = max_alt == DBL_MIN ? NAN : max_alt;

    return 1;
}

TU_EXPORT int trk_dump_track( track_t track )
{
    size_t i;
    point_t point;

    assert( track );

    for( i = 0; i < track->npoints; i++ ) {
	point = track->points[i];

	if( track->out_hndl ) {
	    track->out_hndl( track->env,
			     trk_dump_point( point ) );
	}
    }

    return 1;
}

static char * trk_dump_point( point_t point )
{
    struct tm *tm;
    char tmbuf[64];
    static char msg[4096];

    tm = localtime( &point->time );
    strftime( tmbuf, sizeof( tmbuf ), "%FT%TZ", tm );

    snprintf( msg, sizeof( msg ),
	      "[%s]  lattitude: %.6lf, longitude: %.6lf, "		\
	      "altitude: %.6lf, azimuth: %.6lf, speed: %.6lf, "		\
	      "satellites: %d, fix: %d, HDOP: %.6lf, VDOP: %.6lf, PDOP: %.6lf",
	      tmbuf, point->latitude, point->longitude,
	      point->altitude, point->azimuth, point->speed,
	      point->nsat, point->fix_type, point->hdop, point->vdop, point->pdop);

    return msg;
}


static int trk_parse_data( track_t track, void * data, size_t size )
{
    const char *mime;
    magic_t magic;
    int ret;
    char msg[4096];

    magic = magic_open( MAGIC_MIME_TYPE );
    if( !magic ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "magic_open(): %s",
		      strerror( errno ) );
	    track->err_hndl( track->env, msg );
	}
        return 0;
    }

    if( magic_load( magic, NULL ) != 0 ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "magic_load(): %s",
		      magic_error( magic ) );
	    track->err_hndl( track->env, msg );
	}
        magic_close( magic );
        return 0;
    }

    mime = magic_buffer( magic, data, size );
    if( !mime ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "magic_buffer(): %s",
		      magic_error( magic ) );
	    track->err_hndl( track->env, msg );
	}
        magic_close( magic );
        return 0;
    }

    if( !strcmp( mime, "application/xml" ) ||
	!strcmp( mime, "text/xml" ) ) {
	ret = trk_parse_xml( track, data, size );
    } else if( !strcmp( mime, "text/plain" ) ) {
	ret = trk_parse_nmea( track, data, size );
    } else {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "format is not supported: '%s'", mime );
	    track->err_hndl( track->env, msg );
	}
	ret = 0;
    }

    magic_close( magic );

    if( track->npoints == 0 ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "No valid points found" );
	    track->err_hndl( track->env, msg );
	}
	ret = 0;
    }

    return ret;
}

static int trk_parse_xml( track_t track, void * data, size_t size )
{
    xmlDocPtr doc;
    xmlNodePtr root;
    char msg[4096];
    int ret;

    LIBXML_TEST_VERSION;

    doc = xmlReadMemory( ( const char * )data,
			 size, "XML", NULL, 0 );
    if( !doc ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "can not parse XML" );
	    track->err_hndl( track->env, msg );
	}
        return 0;
    }

    root = xmlDocGetRootElement( doc );
    if( !root ) {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "empty XML" );
	    track->err_hndl( track->env, msg );
	}
        xmlFreeDoc( doc );
        return 0;
    }

    if( !xmlStrcmp( root->name, ( const xmlChar * )"gpx" ) ) {
	ret = trk_parse_gpx( track, doc, root );
    } else if( !xmlStrcmp( root->name, ( const xmlChar * )"TrainingCenterDatabase" ) ) {
	ret = trk_parse_tcx( track, doc, root );
    } else {
	if( track->err_hndl ) {
	    snprintf( msg, sizeof( msg ),
		      "XML type is not supported: '%s'", root->name );
	    track->err_hndl( track->env, msg );
	}
	ret = 0;
    }

    xmlFreeDoc( doc );

    return ret;
}

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
		   double  pdop )
{
    point_t point;

    point = trk_point_make( time,
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
    if( !point )
	return 0;

    track->points = realloc( track->points, ++track->npoints * sizeof( point ) );
    if( !track->points ) {
	track->npoints--;
	trk_point_free( point );
	return 0;
    }
    track->points[track->npoints - 1] = point;

    if( track->start == 0 ) {
	track->start = time;
	track->end = time;
    } else if( time < track->start ) {
	track->start = time;
    } else if( time > track->end ) {
	track->end = time;
    }

    return 1;
}


static inline void trk_linear_interpolate( double   x1, double   y1,
					   double   x2, double * y2,
					   double   x3, double   y3 )
{
    double y;

    y = ( x2 - x1 ) * ( y3 - y1 ) / ( x3 - x1 ) + y1;

    if( y2 )
	*y2 = y;
}

static inline void trk_ac_interpolate( double   x1, double   y1, double   v1,
				       double   x2, double * y2, double * v2,
				       double   x3, double   y3, double   v3 )
{
    double v, a, d;

    ( void )y1;
    ( void )y3;

    trk_linear_interpolate( x1, v1, x2, &v, x3, v3 );
    a = ( v3 - v1 ) / ( x3 - x1 );
    d = v1 * ( x2 - x1 ) + a * ( x2 - x1 ) * ( x2 - x1 ) / 2;

    if( y2 )
	*y2 = d;
    if( v2 )
	*v2 = v;
}

