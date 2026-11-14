#include <curl/curl.h>
#include <string.h>
#include <time.h>
#include <log.h>
#include "dc-ipcs/suntime-calc.h"
#include "dc-ipcs/storage.h"
#include "dc-ipcs/status.h"


dcs_net_stat_t update_ipc_suntime_schedule(const char* ipc_address, struct IPCCredentials* credentials, struct SuntimeSchedule* suntime_schedule) {
	log_trace("Creating libcurl handle...");

	CURL* curl_handle = curl_easy_init();

	if (!curl_handle) {
		log_error("Couldn't initiate libcurl handle");
		return DCS_NET_CONNECTION_ERROR;
	}

	log_trace("Setting handle parameters...");

	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "DC-IPCS/0.1");
	curl_easy_setopt(curl_handle, CURLOPT_DEFAULT_PROTOCOL, "https");

	log_trace("Building URL...");
	
	CURLU* ipc_cgi_url_handle = curl_url();
	
	curl_url_set(ipc_cgi_url_handle, CURLUPART_URL, ipc_address, CURLU_GUESS_SCHEME);

	char* url_proto_scheme;
	CURLUcode scheme_check = curl_url_get(ipc_cgi_url_handle, CURLUPART_SCHEME, &url_proto_scheme, 0);

	if (scheme_check != CURLUE_OK || url_proto_scheme == NULL || strncmp(url_proto_scheme, "http", 4)) {
		curl_url_set(ipc_cgi_url_handle, CURLUPART_SCHEME, "https", 0);
	}

	if (url_proto_scheme) {
		curl_free(url_proto_scheme);
	}

	curl_url_set(ipc_cgi_url_handle, CURLUPART_PATH, "/cgi-bin/configManager.cgi", 0);

	struct tm sunset_tm = *gmtime(&(suntime_schedule->sun_set));
	struct tm sunrise_tm = *gmtime(&(suntime_schedule->sun_rise));

	char ipc_cgi_params[7][64];

	strcpy(ipc_cgi_params[0], "action=setConfig");
	snprintf(ipc_cgi_params[1], sizeof(ipc_cgi_params[1]), "VideoInOptions[0].NightOptions.SunsetHour=%d", sunset_tm.tm_hour);
	snprintf(ipc_cgi_params[2], sizeof(ipc_cgi_params[2]), "VideoInOptions[0].NightOptions.SunsetMinute=%d", sunset_tm.tm_min);
	snprintf(ipc_cgi_params[3], sizeof(ipc_cgi_params[3]), "VideoInOptions[0].NightOptions.SunsetSecond=%d", sunset_tm.tm_sec);
	snprintf(ipc_cgi_params[4], sizeof(ipc_cgi_params[4]), "VideoInOptions[0].NightOptions.SunriseHour=%d", sunrise_tm.tm_hour);
	snprintf(ipc_cgi_params[5], sizeof(ipc_cgi_params[5]), "VideoInOptions[0].NightOptions.SunriseMinute=%d", sunrise_tm.tm_min);
	snprintf(ipc_cgi_params[6], sizeof(ipc_cgi_params[6]), "VideoInOptions[0].NightOptions.SunriseSecond=%d", sunrise_tm.tm_sec);

	curl_url_set(ipc_cgi_url_handle, CURLUPART_QUERY, ipc_cgi_params[0], CURLU_APPENDQUERY);
	curl_url_set(ipc_cgi_url_handle, CURLUPART_QUERY, ipc_cgi_params[1], CURLU_APPENDQUERY);
	curl_url_set(ipc_cgi_url_handle, CURLUPART_QUERY, ipc_cgi_params[2], CURLU_APPENDQUERY);
	curl_url_set(ipc_cgi_url_handle, CURLUPART_QUERY, ipc_cgi_params[3], CURLU_APPENDQUERY);
	curl_url_set(ipc_cgi_url_handle, CURLUPART_QUERY, ipc_cgi_params[4], CURLU_APPENDQUERY);
	curl_url_set(ipc_cgi_url_handle, CURLUPART_QUERY, ipc_cgi_params[5], CURLU_APPENDQUERY);
	curl_url_set(ipc_cgi_url_handle, CURLUPART_QUERY, ipc_cgi_params[6], CURLU_APPENDQUERY);

	char* ipc_cgi_url;
	curl_url_get(ipc_cgi_url_handle, CURLUPART_URL, &ipc_cgi_url, 0);

	curl_url_cleanup(ipc_cgi_url_handle);

	log_trace("Building request content...");

	curl_easy_setopt(curl_handle, CURLOPT_URL, ipc_cgi_url);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
	curl_easy_setopt(curl_handle, CURLOPT_USERNAME, credentials->user_name);
	curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, credentials->password);

	// !!! REMOVE AFTER!
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

	log_debug("Performing request...");

	curl_easy_perform(curl_handle);

	long request_response_code;
	CURLcode result = curl_easy_perform(curl_handle);

    if(result != CURLE_OK) {
    	result = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &request_response_code);

    	if(result != CURLE_OK) {
    		// Assuming that request couldn't be send if there is not any present return code available.
    		// Logging...
		log_error("Request failed and couldn't receive a response.");
    		return DCS_NET_CONNECTION_ERROR;
    	}

    	dcs_net_stat_t return_status;

    	if(request_response_code == 401) {
		log_error("Specified account is unauthorized to perform the request.");
    		return_status = DCS_NET_ERR_UNAUTHORIZED;
    	}
    	else if(request_response_code == 429) {
		log_error("Too many requests sent in a short time.");
    		return_status = DCS_NET_ERR_SERVER_TIMEOUT;
    	}
    	else if(((int)request_response_code / 400) == 1) {
		log_error("Invalid request. Code: %d", request_response_code);
    		return_status = DCS_NET_ERR_INVALID_REQUEST;
    	}
    	else if(((int)request_response_code / 500) == 1) {
		log_error("A server-side error has occured. Code: %d", request_response_code);
    		return_status = DCS_NET_ERR_SERVER;
    	}
	else {
		log_error("Received a response with an undefined status code");
		log_error("Response code: %d", request_response_code);
	}

    	return_status = DCS_NET_ERR_REQUEST_FAILED;
    	// Logging...
    	return return_status;
        //fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(result));
    }

    log_trace("Cleaning up used elements...");
    
    curl_free(ipc_cgi_url);
    curl_easy_cleanup(curl_handle);

    log_debug("Update request has been completed succesfully.");

    return DCS_NET_SUCCESS;


}
