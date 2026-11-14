#pragma once
#include <time.h>
#include "dc-ipcs/status.h"

struct IPCCredentials {
	char* user_name;
	char* password;
};

struct SuntimeGeoConfig {
	double lattitude;
	double longitude;
	double elevation;
};

struct ConfigParameters {
	char** ipc_addresses;
	size_t ipc_addr_list_size;

	struct IPCCredentials credentials;
	struct SuntimeGeoConfig geo_params;

	int reattempt_delay;
	double update_interval;

	char* ls_data_file_path;
	char* cert_path;
};

dcs_storage_stat_t read_config_file(const char* file_name, struct ConfigParameters* config_params);
dcs_storage_stat_t get_last_setting_time(const char* file_name, time_t* last_setting_timestamp);
dcs_storage_stat_t write_last_setting_time(const char* file_name, time_t* last_setting_timestamp);
dcs_storage_stat_t open_log_file(const char* file_name, int log_level);
