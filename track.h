/* -*- Mode: C; c-basic-offset: 4; -*-
 *
 * Track utils
 *
 */

/**
 * @file track.h Track utils library header.
 */

#ifndef TRACK_H_INCLUDED
#define TRACK_H_INCLUDED


#define __USE_XOPEN
#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef void ( * log_hndl ) ( void * env, const char * msg );

typedef struct track_o * track_t;


/**
 * Make track object.
 *
 * @param  err_hndl    Error handler.
 * @param  out_hndl    Output handler.
 * @return             New track object or NULL.
 */
track_t trk_make( log_hndl   err_hndl,
		  log_hndl   out_hndl,
		  void     * env );

/**
 * Drop track object.
 *
 * @param  track  Track object.
 */
void trk_drop( track_t track );

/**
 * Load track from file.
 *
 * @param  track  Track object.
 * @param  file   File name.
 * @retval 1      Success.
 * @retval 0      Failure.
 */
int trk_from_file( track_t track, const char * file );

/**
 * Load track from buffer.
 *
 * @param  track  Track object.
 * @param  buffer Data buffer.
 * @param  size   Data size.
 * @retval 1      Success.
 * @retval 0      Failure.
 */
int trk_from_buffer( track_t track, void * buffer, size_t size );

/**
 * Get coordinates at given time.
 *
 * @param  track      Track object.
 * @param  time       Unixtime.
 * @param  latitude   Placeholder for latitude.
 * @param  longitude  Placeholder for longitude.
 * @param  altitude   Placeholder for altitude.
 * @param  azimuth    Placeholder for azimuth.
 * @param  speed      Placeholder for speed.
 * @retval 1          Success.
 * @retval 0          Failure.
 */
int trk_get_coord_by_utime( track_t  track,
			    time_t   time,
			    double * latitude,
			    double * longitude,
			    double * altitude,
			    double * azimuth,
			    double * speed );

/**
 * Get coordinates at given time.
 *
 * @param  track      Track object.
 * @param  date       ISO 8601 UTC datetime.
 * @param  latitude   Placeholder for latitude.
 * @param  longitude  Placeholder for longitude.
 * @param  altitude   Placeholder for altitude.
 * @param  azimuth    Placeholder for azimuth.
 * @param  speed      Placeholder for speed.
 * @retval 1          Success.
 * @retval 0          Failure.
 */
int trk_get_coord_by_ISOdate( track_t      track,
			      const char * date,
			      double     * latitude,
			      double     * longitude,
			      double     * altitude,
			      double     * azimuth,
			      double     * speed );

/**
 * Get track summary information.
 *
 * @param  track         Track object.
 * @param  npoints       Placeholder for number of track points.
 * @param  start         Placeholder for track start time.
 * @param  end           Placeholder for track end time.
 * @param  distance      Placeholder for track distance.
 * @param  min_speed     Placeholder for min speed.
 * @param  max_speed     Placeholder for max speed.
 * @param  min_altitude  Placeholder for min altitude.
 * @param  max_altitude  Placeholder for max altitude.
 * @retval 1             Success.
 * @retval 0             Failure.
 */
int trk_get_track_summary( track_t  track,
			   size_t * npoints,
			   time_t * start,
			   time_t * end,
			   double * distance,
			   double * min_speed,
			   double * max_speed,
			   double * min_altitude,
			   double * max_altitude );

/**
 * Dump track points.
 *
 * @param  track         Track object.
 * @retval 1             Success.
 * @retval 0             Failure.
 */
int trk_dump_track( track_t track );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

