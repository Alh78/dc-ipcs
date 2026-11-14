#include <cJSON.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <log.h>
#include "dc-ipcs/status.h"
#include "dc-ipcs/storage.h"

dcs_storage_stat_t read_config_file(const char* file_name, struct ConfigParameters* config_params) {
	log_debug("Opening config file: %s", file_name);
	FILE* cfg_fp = fopen(file_name, "rb");
	
	if (cfg_fp == NULL) {
		log_error("Couldn't open the file: %s", file_name);
		return DCS_STORAGE_ERR_FILE_OPEN;
	}

	log_trace("Initializing config file status struct...");
	struct stat cfg_fs = { 0 };

	if (stat(file_name, &cfg_fs)) {
		log_error("Couldn't receive stats of the file: %s", file_name);
		return DCS_STORAGE_ERR_FILE_STAT;
	}

	size_t cfg_file_size = cfg_fs.st_size;

	log_debug("Reading config file...");

	char* raw_cfg_content = calloc(cfg_file_size, sizeof(char));
	size_t read_content_size = fread(raw_cfg_content, sizeof(char), cfg_file_size, cfg_fp);

	log_debug("Closing file handle...");
	fclose(cfg_fp);

	if (read_content_size != cfg_file_size) {
		if (ferror(cfg_fp)) {
	        log_error("I/O error while reading %s: %s", file_name, strerror(errno));

	    } else if (feof(cfg_fp)) {
	        log_error("Unexpected end of file while reading %s", file_name);

	    } else {
	    	log_debug("Content size read from the file and expected file size mismatch.");
		log_error("An error occured during reading the file: %s", file_name);

		}
		
		return DCS_STORAGE_ERR_FILE_READ;
	}

	log_trace("Parsing config file content...");
	cJSON* cfg_json = cJSON_Parse(raw_cfg_content);

	if (!cfg_json) {
		log_error("Couldn't parse the config file: %s", file_name);

		const char* json_error_ptr = cJSON_GetErrorPtr();
		if (json_error_ptr) {
			log_error("Parsing error is as follows: %s", json_error_ptr);
		}

		free(raw_cfg_content);
		cJSON_Delete(cfg_json);
		return DCS_STORAGE_ERR_JSON_PARSE;
	}

	log_trace("Fetching IP addresses from content...");
	cJSON* ipc_addresses = cJSON_GetObjectItemCaseSensitive(cfg_json, "ipc_addresses");
	
	config_params->ipc_addr_list_size = cJSON_GetArraySize(ipc_addresses);
	config_params->ipc_addresses = calloc(config_params->ipc_addr_list_size, sizeof(char*));
	
	for (size_t i = 0; i < config_params->ipc_addr_list_size; i++) {
		cJSON* ipc_address = cJSON_GetArrayItem(ipc_addresses, i);

		config_params->ipc_addresses[i] = calloc(strlen(ipc_address->valuestring) + 1, sizeof(char));
		strcpy(config_params->ipc_addresses[i], ipc_address->valuestring);
	}

	log_trace("Fetching credentials...");
	cJSON* credentials = cJSON_GetObjectItemCaseSensitive(cfg_json, "credentials");

	cJSON* user_name = cJSON_GetObjectItemCaseSensitive(credentials, "name");
	cJSON* password = cJSON_GetObjectItemCaseSensitive(credentials, "password");

	if (!(cJSON_IsString(user_name) && (user_name->valuestring != NULL) || cJSON_IsString(password) && (password->valuestring != NULL))) {
		log_error("Found insufficient or invalid credentials in config file!");

		free(raw_cfg_content);
		cJSON_Delete(cfg_json);
		return DCS_STORAGE_ERR_INSUFFICIENT_PARAMS;
	}


	config_params->credentials.user_name = calloc(strlen(user_name->valuestring) + 1, sizeof(char));
	config_params->credentials.password = calloc(strlen(password->valuestring) + 1, sizeof(char));
 
	strcpy(config_params->credentials.user_name, user_name->valuestring);
	strcpy(config_params->credentials.password, password->valuestring);

	log_trace("Fetching geographical parameters...");

	cJSON* longitude = cJSON_GetObjectItemCaseSensitive(cfg_json, "longitude");
	cJSON* lattitude = cJSON_GetObjectItemCaseSensitive(cfg_json, "lattitude");

	if (!cJSON_IsNumber(longitude) || !cJSON_IsNumber(lattitude)) {
		log_error("Missing required geographical parameters in config file!");

		free(raw_cfg_content);
		cJSON_Delete(cfg_json);
		return DCS_STORAGE_ERR_INSUFFICIENT_PARAMS;
	}


	config_params->geo_params.lattitude = lattitude->valuedouble;
	config_params->geo_params.longitude = longitude->valuedouble;

	cJSON* elevation = cJSON_GetObjectItemCaseSensitive(cfg_json, "elevation");

	if (elevation == NULL  || !cJSON_IsNumber(elevation)) {
		log_warn("No elevation is specified. Assuming elevation has been set as 0...");
		config_params->geo_params.elevation = 0.0;
	} else {
		config_params->geo_params.elevation = elevation->valuedouble;
	}
	
	log_trace("Fetching runtime parameters...");

	cJSON* reattempt_delay = cJSON_GetObjectItemCaseSensitive(cfg_json, "reattempt_delay");

	config_params->reattempt_delay = reattempt_delay->valueint;

	cJSON* update_interval = cJSON_GetObjectItemCaseSensitive(cfg_json, "update_interval");

	config_params->reattempt_delay = reattempt_delay->valuedouble;
	
	cJSON* ls_data_file_path = cJSON_GetObjectItemCaseSensitive(cfg_json, "ls_data_file_path");
	cJSON* cert_file_path = cJSON_GetObjectItemCaseSensitive(cfg_json, "cert_path");

	config_params->ls_data_file_path = calloc(strlen(ls_data_file_path->valuestring) + 1, sizeof(char));
	config_params->cert_path = calloc(strlen(cert_file_path->valuestring) + 1, sizeof(char));

	strcpy(config_params->ls_data_file_path, ls_data_file_path->valuestring);
	strcpy(config_params->cert_path, cert_file_path->valuestring);

	free(raw_cfg_content);
	cJSON_Delete(cfg_json);

	log_debug("Configuration has been loaded successfully.");	

	return DCS_STORAGE_SUCCESS;
}


dcs_storage_stat_t get_last_setting_time(const char* file_name, time_t* last_setting_timestamp) {
	FILE* ls_fp = fopen(file_name, "rb");

	if (ls_fp == NULL) {
		log_error("Couldn't open the file: %s", file_name);
		return DCS_STORAGE_ERR_FILE_OPEN;
	}
	
	size_t read_count = fread(last_setting_timestamp, sizeof(time_t), 1, ls_fp);

	if (read_count != 1) {
		if (ferror(ls_fp)) {
                	log_error("I/O error while reading %s: %s", file_name, strerror(errno));

            	} else if (feof(ls_fp)) {
                	log_error("Unexpected end of file while reading %s", file_name);
	    	}

		fclose(ls_fp);
		return DCS_STORAGE_ERR_FILE_READ;
	}

	fclose(ls_fp);

	return DCS_STORAGE_SUCCESS;
}

dcs_storage_stat_t write_last_setting_time(const char* file_name, time_t* last_setting_timestamp) {
	FILE* ls_fp = fopen(file_name, "wb");

	if (ls_fp == NULL) {
		log_error("Couldn't open the file: %s", file_name);
		return DCS_STORAGE_ERR_FILE_OPEN;
	}

	size_t written_count = fwrite(last_setting_timestamp, sizeof(time_t), 1, ls_fp);
	
	if (written_count != 1) {
         	log_error("I/O error while writing %s: %s", file_name, strerror(errno));
		fclose(ls_fp);
                return DCS_STORAGE_ERR_FILE_WRITE;
	}

	if (fclose(ls_fp) != 0) {
		return DCS_STORAGE_ERR_FILE_CLOSE;

	}

	return DCS_STORAGE_SUCCESS;
}

dcs_storage_stat_t open_log_file(const char* file_name, int log_level) {
	FILE* log_fp = fopen(file_name, "a");

	if (log_fp == NULL) {
		log_error("Couldn't open the file: %s", file_name);
		return DCS_STORAGE_ERR_FILE_OPEN;
	}

	log_add_fp(log_fp, log_level);

	return DCS_STORAGE_SUCCESS;
}
