#pragma once

#include <math.h>
#include <time.h>

#define rad2deg(rad) ((rad) * 180.0 / M_PI)
#define deg2rad(deg) ((deg) * M_PI / 180.0)

#define SUN_CIVIL_TWILIGHT_ALTITUDE_DEGREE -6.0
#define SUN_CENTER_ALTITUDE_DEGREE -0.833

typedef double dcs_radian;
typedef double dcs_degree;

struct SuntimeSchedule {
	time_t sun_rise;
	time_t sun_set;
};

dcs_degree get_system_standard_longtitude(time_t system_time);
struct SuntimeSchedule get_suntime_schedule(dcs_degree lattitude, dcs_degree longtitude, double elevation, time_t local_time);
