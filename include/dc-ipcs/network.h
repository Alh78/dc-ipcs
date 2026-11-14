#pragma once
#include "dc-ipcs/suntime-calc.h"
#include "dc-ipcs/storage.h"
#include "dc-ipcs/status.h"

//int fetch_current_suntime_schedule(double lattitude, double longtitude, struct SuntimeSchedule* suntime_schedule);
dcs_net_stat_t update_ipc_suntime_schedule(const char* ipc_address, struct IPCCredentials* credentials, struct SuntimeSchedule* suntime_schedule);
