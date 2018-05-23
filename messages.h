/**
 *
 * \file
 *      Application messages creation and parsing functions
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <stdio.h>

/*
 * Device types to send a message*
 */
enum {
    DEST_ROUTER,
    DEST_COORDINATOR,
    DEST_END_DEVICE,
}dev_type;

/*
 * Types of message to send
 */
enum {
    MSG_CONECTION,
    MSG_CONFIGURATION,
    MSG_KEEPALIVE,
    MSG_UNKNOWN
}msg_type;
/*---------------------------------------------------------------------------*/
/** \name MSG_TYPE number
 * @{
 */
//#define MSG_CONECTION 	0x01 /**< Connection messages */
//#define MSG_CONFIG 		0x02 /**< Configuration messages */
//#define MSG_KEEPALIVE 	0x04 /**< Keep alive messages */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name MSG_CONNECTION offsets (only one of them can be set)
 * @{
 */
#define MSG_CONECTION_ASSOCIATION_REQUEST 	0x01 /**< Resquest connection */
#define MSG_CONECTION_ASSOCIATION_RESPONSE 	0x02 /**< Response connection */
#define MSG_CONECTION_CONFIGURATION_REQUEST 0x04 /**< Request a new configuration */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name MSG_CONFIGURATION bit masks
 * @{
 */
#define MSG_CONFIGURATION_KEEP_ALIVE_DATA 	0x01 /**< Data to be received in KA(keep alive) */
#define MSG_CONFIGURATION_SAMPLING_RATE 	0x02 /**< Keep alive sampling rate */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name MSG_KEEPALIVE bit masks
 * @{
 */
#define MSG_KEEPALIVE_ACCELERATION 	0x01 /**< Acceleration data */
#define MSG_KEEPALIVE_INCLINATION 	0x02 /**< Inclination */
#define MSG_KEEPALIVE_RSSI 			0x04 /**< RSSI of the last ack received */
/** @} */
/*---------------------------------------------------------------------------*/

typedef struct {     /**< 16 byte total. */
  uint16_t sampleCounter;   /**< 2 byte (4 each float).  */
  uint16_t sampleIndex;     /**< 2 byte (4 each float).  */
  float acc[3];             /**< 12 byte (4 each float). Acceleration data */
}acc_t;

typedef struct {
  float inclination[3]; /**< 12 byte (4 each float). Inclination data */
}inclination_t;

typedef struct {
  uint8_t message_type; /**< 2 bit. Message type. Defined by msg_type enum */
  uint8_t device_type;  /**< 2 bit. Source device type. Defined by dev_type enum */
  uint8_t mask;         /**< 4 bit. Bit masks. Define the contents of message */
} pck_id_t;

typedef struct {
  pck_id_t pck_id;          /**< package id field  */
  uint16_t sampling_rate;   /**< Keep alive messages sampling rate*/
  uint8_t keepalive_data;  /**< Data to send in a keep alive */
  acc_t accel; //12+2+2
  inclination_t inclination; //12
  int8_t rssi; // 1
} pck_t;


/**
 * \brief App message format
 *
 * This function is called by the 6lowpan code to create a compressed
 * \verbatim
 *                      1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |MSG|DEV|  MASK |Other APP fields|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * \endverbatim
 *
 * \param MSG   msg type
 * \param DEV   device type
 * \param Bits  This bits enables some functions depending of msg id
 *
 * \param MSG+DEV+Bits The package id
 *
 */
/*----------------------------------------------------------------------------*/
/*
 * \brief Create a package frame to send. This message is called by
 * Application layer
 *
 * \param p    Input data. Pointer to pck_t struct, which specifies the
 * data to send.
 *
 * \param buf  Pointer to the buffer to use for the frame
 *
 *  \return The length of the frame to sent
 */

int pck_create(pck_t *p, uint8_t *buf);

/*----------------------------------------------------------------------------*/
/**
 *   \brief Parses an input package frame. Scan the given data
 *
 *   \param data The input data from the radio chip.
 *   \param pf The pck struct to store the parsed frame information.
 *
 *   \return successful parsing
 */

int pck_parse(uint8_t *data, pck_t *pf);

/*----------------------------------------------------------------------------*/
/**
 *   \brief Print all content of package
 *
 *   \param pck The pck struct to print
 */

void print_all_pck(pck_t *pck);


#endif
