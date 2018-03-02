
lib_LTLIBRARIES = libtu.la

libtu_la_SOURCES  = geodesic.h geodesic.c minmea.h minmea.c sunriset.h sunriset.c gpx.h gpx.c tcx.h tcx.c nmea.h nmea.c point.h point.c track.h track_priv.h track.c
libtu_la_CPPFLAGS =
libtu_la_CFLAGS   = -I/usr/include/libxml2 -Wall -fvisibility=hidden -ffunction-sections -fdata-sections
libtu_la_LDFLAGS  = -version-info 1:0:0 -no-undefined -lmagic -lxml2 -lm
libtu_la_LIBADD   =

libtu_la_includedir      = $(includedir)
libtu_la_include_HEADERS = track.h


bin_PROGRAMS    = tu-test
tu_test_SOURCES = tu_test.c
tu_test_LDFLAGS = -ltu
tu_test_LDADD   = libtu.la

