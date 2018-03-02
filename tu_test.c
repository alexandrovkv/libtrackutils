/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * libtu test
 *
 */


#include <stdio.h>
#include <getopt.h>

#include "track.h"


static int parse_cmdline( int argc, char **argv );
static void err_hndl( void * env, const char * msg );
static void out_hndl( void * env, const char * msg );

static char * s2dhms( time_t s );


static char * opt_track_file  = NULL;
static char * opt_date_time   = NULL;
static int    opt_debug       = 0;



int main( int argc, char **argv )
{
    track_t track;
    double latitude, longitude, altitude, azimuth, speed;
    size_t npoints;
    time_t start, end;
    double distance, min_speed, max_speed, min_altitude, max_altitude;
    int ret;

    if( !parse_cmdline( argc, argv ) )
	return 1;

    track = trk_make( err_hndl, out_hndl, NULL );
    if( !track )
	return 1;

    if( !trk_from_file( track, opt_track_file ) ) {
	trk_drop( track );
	return 1;
    }

    ret = trk_get_coord_by_ISOdate( track, opt_date_time,
				    &latitude, &longitude,
				    &altitude, &azimuth,
				    &speed );
    if( ret )
	fprintf( stdout, "latitude: %lf, longitude: %lf, altitude: %lf," \
		 " azimuth: %lf, speed: %lf kph\n",
		 latitude, longitude, altitude, azimuth, speed * 3.6 );

    ret = trk_get_track_summary( track, &npoints, &start, &end,
				 &distance, &min_speed, &max_speed,
				 &min_altitude, &max_altitude );
    if( ret ) {
	struct tm *tm;
	char tmbuf[2][64];

	tm = localtime( &start );
	strftime( tmbuf[0], sizeof( tmbuf[0] ), "%FT%TZ", tm );
	tm = localtime( &end );
	strftime( tmbuf[1], sizeof( tmbuf[1] ), "%FT%TZ", tm );

	fprintf( stdout, "track summary:\n"	\
		 "  num points:    %u\n"	\
		 "  start time:    %s\n"	\
		 "  end time:      %s\n"	\
		 "  duration:      %s\n"	\
		 "  distance:      %lf m\n"	\
		 "  min speed:     %lf kph\n"	\
		 "  average speed: %lf kph\n"	\
		 "  max speed:     %lf kph\n"	\
		 "  min altitude:  %lf m\n"	\
		 "  max altitude:  %lf m\n",
		 npoints, tmbuf[0], tmbuf[1], s2dhms( end - start ), distance,
		 min_speed * 3.6, distance * 3.6 / ( end - start ), max_speed * 3.6,
		 min_altitude, max_altitude );
    }

    opt_debug && trk_dump_track( track );

    trk_drop( track );

    return ret ? 0 : 1;
}


static void err_hndl( void * env, const char * msg )
{
    ( void )env;

    fprintf( stderr, "%s\n", msg );
}

static void out_hndl( void * env, const char * msg )
{
    ( void )env;

    fprintf( stdout, "%s\n", msg );
}


static char * s2dhms( time_t s )
{
    int secInMin = 60;
    int secInHour = 60 * secInMin;
    int secInDay = 24 * secInHour;
    int D = s / secInDay;
    int hS = s % secInDay;
    int H = ( hS ) / secInHour;
    int mS = hS % secInHour;
    int M = mS / secInMin;
    int S = mS % secInMin;
    static char buf[32];

    snprintf( buf, sizeof( buf ), "%02d %02d:%02d:%02d", D, H, M, S );

    return buf;
}


static const char *usage =
    PACKAGE_NAME " v. " PACKAGE_VERSION "\n"
    "\n"
    " usage: " PACKAGE_NAME " <options>"
    "\n"
    "  -h, --help                   - This help\n"
    "\n"
    "  -T <str>, --track=<str>      - track file (GPX, TCX, NMEA)\n"
    "  -D <str>, --date=<str>       - ISO 8601 UTC datetime\n";

static int parse_cmdline( int argc, char **argv )
{
    struct option long_options[] = {
        { "help",       no_argument,        0,  'h' },
        { "debug",      no_argument,        0,  'd' },
        { "track",      required_argument,  0,  'T' },
        { "date",       required_argument,  0,  'D' },
        { 0,            0,                  0,   0  }
    };
    const char *short_options = "hdT:D:";
    int opt = 0, option_index = 0;
    int help = 0, err = 0;

    opterr = 1;
    while( ( opt = getopt_long( argc,
                                argv,
                                short_options,
                                long_options,
                                &option_index ) ) != -1 ) {
        switch( opt ) {
        case 'h':
            help = 1;
            break;
        case 'd':
            opt_debug = 1;
            break;
        case 'T':
            opt_track_file = optarg;
            break;
        case 'D':
            opt_date_time = optarg;
            break;
        default:
            err = 1;
	    break;
        }
    }

    if( err || help || argc == 1 ) {
        fprintf( stderr, "%s", usage );
        return 0;
    }

    if( optind < argc ) {
	fprintf( stderr, "non-options elements:" );
	while( optind < argc )
	    fprintf( stderr, " %s", argv[optind++] );
	fprintf( stderr, "\n\n" );
        fprintf( stderr, "%s", usage );
	return 0;
    }

    if( !opt_track_file || !opt_date_time ) {
	fprintf( stderr, "Missing required parameters\n" );
	return 0;
    }

    return 1;
}

