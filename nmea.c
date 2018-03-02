/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils library, NMEA parser.
 *
 * $Id$
 *
 * $Log:
 *
 */

char *nmea_c_cvsid =
    "$Id$";

/**
 * @file nmea.c NMEA parser implementation.
 */


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "minmea.h"

#include "nmea.h"
#include "track_priv.h"



int trk_parse_nmea( track_t track, void * data, size_t size )
{
    char *delim = "\r\n";
    char *buf, *line;
    struct minmea_sentence_rmc rmc_frame;
    struct minmea_sentence_gga gga_frame;
    struct minmea_sentence_gsa gsa_frame;
    struct minmea_sentence_gst gst_frame;
    struct minmea_sentence_gll gll_frame;
    struct minmea_sentence_gsv gsv_frame;
    struct minmea_sentence_vtg vtg_frame;
    bool have_rmc = false,
	have_gga = false,
	have_gsa = false,
	have_gst = false,
	have_gll = false,
	have_gsv = false,
	have_vtg = false;
    double latitude = NAN, longitude = NAN, altitude = NAN, azimuth = NAN, speed = NAN;
    int nsat = -1, fix_type = -1;
    double hdop = NAN, vdop = NAN, pdop = NAN;
    struct timespec ts;

    buf = strdup( data );

    line = strtok( buf, delim );
    while( line ) {
	switch( minmea_sentence_id( line, true ) ) {
	case MINMEA_SENTENCE_RMC:
	    if( minmea_parse_rmc( &rmc_frame, line ) ) {
		if( rmc_frame.valid ) {
		    latitude = minmea_tocoord( &rmc_frame.latitude );
		    longitude = minmea_tocoord( &rmc_frame.longitude );
		    azimuth = minmea_tofloat( &rmc_frame.course );
		    speed = minmea_tofloat( &rmc_frame.speed ) * .514444;
		    minmea_gettime( &ts, &rmc_frame.date, &rmc_frame.time );

		    have_rmc = true;
		}
	    }
	    break;
	case MINMEA_SENTENCE_GGA:
	    if( minmea_parse_gga( &gga_frame, line ) ) {
		nsat = gga_frame.satellites_tracked;
		altitude = minmea_tofloat( &gga_frame.altitude );

		have_gga = true;
	    }
	    break;
	case MINMEA_SENTENCE_GSA:
	    if( minmea_parse_gsa( &gsa_frame, line ) ) {
		fix_type = gsa_frame.fix_type;
		hdop = minmea_tofloat( &gsa_frame.hdop );
		vdop = minmea_tofloat( &gsa_frame.vdop );
		pdop = minmea_tofloat( &gsa_frame.pdop );

		have_gsa = true;
	    }
	    break;
	case MINMEA_SENTENCE_GST:
	    if( minmea_parse_gst( &gst_frame, line ) ) {
		have_gst = true;
	    }
	    break;
	case MINMEA_SENTENCE_GLL:
	    if( minmea_parse_gll( &gll_frame, line ) ) {
		have_gll = true;
	    }
	    break;
	case MINMEA_SENTENCE_GSV:
	    if( minmea_parse_gsv( &gsv_frame, line ) ) {
		have_gsv = true;
	    }
	    break;
	case MINMEA_SENTENCE_VTG:
	    if( minmea_parse_vtg( &vtg_frame, line ) ) {
		have_vtg = true;
	    }
	    break;
	case MINMEA_INVALID:
	    break;
	default:
	    break;
	}

	if( have_rmc && have_gga && have_gsa ) {
	    if( !trk_add_point( track,
				ts.tv_sec,
				latitude,
				longitude,
				altitude,
				azimuth,
				speed,
				nsat,
				fix_type,
				hdop,
				vdop,
				pdop ) ) {
		free( buf );
		return 0;
	    }

	    have_rmc = false;
	    have_gga = false;
	    have_gsa = false;
	    have_gst = false;
	    have_gll = false;
	    have_gsv = false;
	    have_vtg = false;
	}

	line = strtok( NULL, delim );
    }

    free( buf );

    return 1;
}

