#ifdef PBL_HEALTH
#include <pebble.h>
#include "logging.h"

HealthValue health_get_value_today(HealthMetric metric) {
    logf();
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
    return mask & HealthServiceAccessibilityMaskAvailable ? health_service_sum_today(metric) : 0;
}

#endif