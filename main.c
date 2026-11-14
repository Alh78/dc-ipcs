#include <time.h>
#include <log.h>
#include <unistd.h>
#include "dc-ipcs/storage.h"
#include "dc-ipcs/status.h"
#include "dc-ipcs/suntime-calc.h"
#include "dc-ipcs/network.h"

int main(void) {
	log_set_quiet(true);

	time_t present_time = time(NULL);
	char log_file_name[32];

	strftime(log_file_name, 32, "dc-ipcs_%Y.log", localtime(&present_time));

	open_log_file(log_file_name, LOG_INFO);

	log_info("Reading config file...");

	struct ConfigParameters cfg_params = { 0 };
	dcs_storage_stat_t config_read_attempt = read_config_file("config.json", &cfg_params);

	if (config_read_attempt != DCS_STORAGE_SUCCESS) {
		log_fatal("Failed to read config file!");
		goto fatal_cleanup;
	}

	time_t last_setting_time = 0;

	dcs_storage_stat_t ls_file_read_attempt = get_last_setting_time(cfg_params.ls_data_file_path, &last_setting_time);

	if (ls_file_read_attempt != DCS_STORAGE_SUCCESS) {
		log_warn("Couldn't read the file containing last setting timestamp data");
		log_warn("Last update time check will be skipped.");
	}
	else if (difftime(present_time, last_setting_time) < cfg_params.update_interval) {
		log_info("Not enough time passed since the last update.");
		return 0;
	}

	
	log_info("Calculating sun rise/set times...");

	struct SuntimeSchedule suntime_schedule = { 0 };
	suntime_schedule = get_suntime_schedule(cfg_params.geo_params.lattitude,
						cfg_params.geo_params.longitude,
						cfg_params.geo_params.elevation,
						present_time);

	log_info("Starting to update the suntime schedules of specified IPCs...");


	for (size_t i = 0; i < cfg_params.ipc_addr_list_size; i++) {
		size_t retry_counter = 0;	
		dcs_net_stat_t ipc_update_attempt;

		do {
			log_info("Updating \"%s\"...", cfg_params.ipc_addresses[i]);
			ipc_update_attempt = update_ipc_suntime_schedule(cfg_params.ipc_addresses[i], &(cfg_params.credentials), &suntime_schedule);
			
			switch(ipc_update_attempt) {
				case DCS_NET_SUCCESS:
					log_info("Schedule has been updated successfully!");
					break;

				case DCS_NET_ERR_UNAUTHORIZED:
					log_fatal("Specified account is not authorized to complete the request.");
					goto fatal_cleanup;
				
				case DCS_NET_ERR_INVALID_REQUEST:
					log_fatal("Sent request is invalid.");
					goto fatal_cleanup;

				case DCS_NET_ERR_SERVER_TIMEOUT:
					log_warn("Too many requests in a short time.");
					log_info("Waiting for %d seconds...", cfg_params.reattempt_delay);
					sleep(cfg_params.reattempt_delay);
					break;

				default:
					log_error("An unexpected error occured while attempting to update! (Code: %d)", ipc_update_attempt);
					sleep(cfg_params.reattempt_delay);
					break;

			}

			retry_counter++;

		} while (ipc_update_attempt != DCS_NET_SUCCESS && retry_counter < 5);

		
	}

	dcs_storage_stat_t ls_write_attempt = write_last_setting_time(cfg_params.ls_data_file_path, &present_time);

	if (ls_write_attempt != DCS_STORAGE_SUCCESS) {
		log_error("Failed to write last setting timestamp!");
	}
	else {
		log_info("Process completed successfuly.");
	}
	
	return 0;

fatal_cleanup:
	log_info("Aborting the process with error code...");
	return 1;
}
