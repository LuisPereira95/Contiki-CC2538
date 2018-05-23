/**
 *
 * \file
 *      Storage all configurations
 */
#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <stdio.h>

/* ----------------------------------------------------- */

#ifdef APPCONFIG_CONF_SAMPLING_RATE
#define APPCONFIG_SAMPLING_RATE APPCONFIG_CONF_SAMPLING_RATE
#else
#define APPCONFIG_SAMPLING_RATE 15  // 15 seconds
#endif /* APPCONFIG_CONF_SAMPLING_RATE */

#ifdef APPCONFIG_CONF_DATA_KA
#define APPCONFIG_DATA_KA APPCONFIG_CONF_DATA_KA
#else
#define APPCONFIG_DATA_KA 0x04      // only send previous RSSI
#endif /* APPCONFIG_CONF_DATA_KA */

#if !UIP_CONF_ROUTER
PROCESS_NAME(sensor_node_process);
#endif
uint16_t sampling_rate;
uint8_t data_to_received;

// The global variables should't have a static parameter
// the local variables need to be declared as a static
#endif /* ENDDEVICETASK_H */
