#include <math.h>
#include <time.h>
#include "dc-ipcs/suntime-calc.h"


static dcs_radian day_angle(int day_order) {
	return deg2rad((360.0/365.0) * ((double)day_order - 81.0));
}

static double longtitude_correction(dcs_degree standard_longtitude, dcs_degree longtitude) {
	return (standard_longtitude - longtitude) * 4;
}

static double equation_of_time(dcs_radian day_angle) {
	return 9.87 * sin(2.0 * day_angle) - 7.53 * cos(day_angle) - 1.5 * sin(day_angle);
}

static double solar_noon(double ltd_crc, double eot) {
	return 720.0 + ltd_crc - eot;
}


static dcs_radian sun_declination_angle(dcs_radian day_angle) {
	return deg2rad(23.44 * sin(day_angle));
}

static dcs_degree suntime_solar_hour_angle(dcs_degree lattitude, dcs_radian sun_decl, dcs_radian sun_altitude_angle) {
	dcs_radian rad_lattitude = deg2rad(lattitude);
	double solar_hour_angle_cosine = ( sin(sun_altitude_angle) / (cos(sun_decl) * cos(rad_lattitude)) ) - tan(sun_decl) * tan(rad_lattitude);

	if (solar_hour_angle_cosine > 1) {
		return 0.0;
	
	} else if (solar_hour_angle_cosine < -1) {
		return 180.0;
	}

	return rad2deg(acos(solar_hour_angle_cosine));
}

static dcs_degree dip_of_the_horizon(dcs_degree sun_altitude_angle, double elevation) {
	return sun_altitude_angle - (0.0347 * sqrt(elevation));
}

dcs_degree get_system_standard_longtitude(time_t system_time) {
	struct tm gm_tm = *gmtime(&system_time);
	struct tm local_tm = *localtime(&system_time);

	time_t gm_time = mktime(&gm_tm);
	time_t local_time = mktime(&local_tm);

	double time_diff = difftime(local_time, gm_time);

	return time_diff / 240.0;
}


struct SuntimeSchedule get_suntime_schedule(dcs_degree lattitude, dcs_degree longtitude, double elevation, time_t local_time) {
	struct tm local_tm = *localtime(&local_time);

	dcs_degree std_ltd = get_system_standard_longtitude(local_time);

	dcs_radian day_ang, sun_decl;

	day_ang = day_angle(local_tm.tm_yday + 1);
	sun_decl = sun_declination_angle(day_ang);
	
	double eot, ltd_crc;

	eot = equation_of_time(day_ang);
	ltd_crc = longtitude_correction(std_ltd, longtitude);

	dcs_degree solar_hour_angle = suntime_solar_hour_angle(lattitude, sun_decl, deg2rad(dip_of_the_horizon(SUN_CENTER_ALTITUDE_DEGREE, elevation)));
	double suntime_noon_diff = solar_hour_angle * 4.0; // As minutes

	double noon_time, sunrise_time, sunset_time;

	noon_time = solar_noon(ltd_crc, eot);
	sunrise_time = noon_time - suntime_noon_diff;
	sunset_time = noon_time + suntime_noon_diff;

	struct SuntimeSchedule suntime_schedule;
	suntime_schedule.sun_rise = (time_t)(sunrise_time * 60);
	suntime_schedule.sun_set = (time_t)(sunset_time * 60); // As seconds, to convert time_t

	return suntime_schedule;
}
