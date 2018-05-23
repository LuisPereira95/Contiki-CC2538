/**
 *
 * \file
 *      Manage the received messages.
 */

#ifndef APPTASK_H
#define APPTASK_H

#include <stdio.h>

#include "net/ip/uip.h" // to get appdata, uip_len
#include "net/ip/uip-udp-packet.h"  // to send udp packages
#include "net/netstack.h"   // to access diferent layers
#include "messages.h"
#include "app-config.h"

#define MAX_MSG_APP_SIZE 31

/* States of devices( sensor node, router, coordinator*/
enum {
    /* Initialized but not connected */
    DEV_INIT,
    /* Connecting to a PAN */
    DEV_JOINING,
    /* Waiting for a configuration message (only for end-device) */
    DEV_WAIT_CONFIGURATION,
    /* Connected and authenticated as a end-device */
    DEV_END_DEVICE,
    /* Connected and authenticated as a router */
    DEV_ROUTER,
    /* Connected and authenticated as a coordinator */
    DEV_COORDINATOR,
    /* Disconnected from the network */
    DEV_DISCONNECTED
}dev_state;

uint8_t device_state;
struct uip_udp_conn *dest_conn;

/*----------------------------------------------------------------------------*/
/**
 * \brief Parsing the message received by OTA(over the air)
 *
 * \param pck The pck struct receive
 *
 */
void input_message(uint8_t *data);

/*----------------------------------------------------------------------------*/
/**
 * \brief Parsing the conection messages ( association, response, and dissociation)
 *
 * \param content Define the message type
 *
 */
void parse_connection(uint8_t content);
/*----------------------------------------------------------------------------*/
/**
 * \brief Parsing the configuration messages (KA data and Sampling rate)
 *
 * \param pck       The pck struct receive
 *
 */
void parse_configuration(pck_t *pck);
/*----------------------------------------------------------------------------*/
/**
 * \brief Parsing the Keep Alive messages ( received data)
 *
 * \param pck       The pck struct receive
 *
 */
void parse_keepalive(pck_t *pck);
/*
 * Sends a application message to a specific device
 *
 * This function send a message Over-the_Air
 *
 * \param pck The pck struct to send type
 * \param pck The uip_udp struct that contain the destination address and udp ports
 *
 */
void send_message(pck_t *pck, struct uip_udp_conn *conn);




#endif /* APPTASK_H */
