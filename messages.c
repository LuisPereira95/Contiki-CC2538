/**
 *
 * \file
 *      Application messages creation and parsing functions
 */

    // falta adicionar o length ao parser da mensagem
#include "messages.h"

#define DEBUG DEBUG_PRINT
#if DEBUG
#include "net/ip/uip-debug.h"
#endif /* DEBUG */
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
int
pck_create(pck_t *p, uint8_t *buf) {
  int error = 0;
  uint8_t pos;

  buf[0] = (p->pck_id.mask & 15) |
    ((p->pck_id.device_type & 3) << 4) |
    ((p->pck_id.message_type & 3) << 6);

  pos = 1;

  switch (p->pck_id.message_type) {
    case MSG_CONECTION:
      break;
    case MSG_CONFIGURATION:
      if((p->pck_id.mask) & MSG_CONFIGURATION_KEEP_ALIVE_DATA) {
        buf[pos++] = p->keepalive_data;
      }
      if((p->pck_id.mask) & MSG_CONFIGURATION_SAMPLING_RATE) {
        buf[pos++] = p->sampling_rate & 0xff;
        buf[pos++] = (p->sampling_rate >> 8) & 0xff;
      }
      break;
    case MSG_KEEPALIVE:
      if((p->pck_id.mask) & MSG_KEEPALIVE_ACCELERATION) {
        //buf[pos++] = p->accel.sampleCounter & 0xff;
        //buf[pos++] = (p->accel.sampleCounter >> 8) & 0xff;
        //buf[pos++] = p->accel.sampleIndex & 0xff;
        //buf[pos++] = (p->accel.sampleIndex >> 8) & 0xff;
        //memcpy(buf + pos, p->accel.acc, 12);
        //pos += 12;
        // try 2
        memcpy(buf + pos, &(p->accel), sizeof(acc_t));
        pos += sizeof(acc_t);
      }
      if((p->pck_id.mask) & MSG_KEEPALIVE_INCLINATION) {
        memcpy(buf + pos, p->inclination.inclination, sizeof(inclination_t));
        pos += sizeof(inclination_t);
      }
      if((p->pck_id.mask) & MSG_KEEPALIVE_RSSI) {
        buf[pos++] = p->rssi;
      }
      break;
    default:
      error = 1;
      break;
  }
  if(error) return 0;
  return (int)pos;
}

/*----------------------------------------------------------------------------*/
/**
 *   \brief Parses an input package frame. Scan the given data
 *
 *   \param data The input data from the radio chip.
 *   \param pf The pck struct to store the parsed frame information.
 *
 *   \return successful parsing
 */
int 
pck_parse(uint8_t *data, pck_t *pf) {
  uint8_t *p, error = 0;
  pck_id_t pck;

  p = data;

  pck.mask = p[0] & 15;
  pck.device_type = (p[0] >> 4) & 3;
  pck.message_type = (p[0] >> 6) & 3;
  //PRINTF("parse: PCK: %x \n", p[0]);


  memset(pf, 0, sizeof(pck_t)); // set all pck pf to zero;
  memcpy(&pf->pck_id, &pck, sizeof(pck_id_t));  // copy the pck_id to pck
  p += 1; // skip the first byte ( pck_id byte)

  switch (pck.message_type) {
    case MSG_CONECTION:
      break;
    case MSG_CONFIGURATION:
      if(pck.mask & MSG_CONFIGURATION_KEEP_ALIVE_DATA) {
        pf->keepalive_data = p[0];
        p++;    // add one position;
      }
      if(pck.mask & MSG_CONFIGURATION_SAMPLING_RATE) {
        pf->sampling_rate = p[0] + (p[1] << 8);
        p += 2;
      }
      break;
    case MSG_KEEPALIVE:
      if(pck.mask & MSG_KEEPALIVE_ACCELERATION) {
        memcpy(&(pf->accel), p, sizeof(acc_t));
        p += sizeof(acc_t);
      }
      if(pck.mask & MSG_KEEPALIVE_INCLINATION) {
        memcpy(&(pf->inclination), p, sizeof(inclination_t));
        p += sizeof(inclination_t);
      }
      if(pck.mask & MSG_KEEPALIVE_RSSI) {
        pf->rssi = p[0];
        p++;    // add one position;
      }
      break;
    default:
      error = 1;
      break;
}

  if(error) return 0;
  return 1;
}
/*
 * \brief Print all content of pck
 *
 */
void
print_all_pck(pck_t *pck) {
#if DEBUG
  uint8_t res[3][15];

  PRINTF("PRINT PCK: pck_id: %x, %x, %x \n", pck->pck_id.message_type, pck->pck_id.device_type, pck->pck_id.mask);
  PRINTF("Sampling rate: %d; DataKA: %x \n", pck->sampling_rate, pck->keepalive_data);

  ftos(pck->accel.acc[0], (void*) &res[0]);
  ftos(pck->accel.acc[1], (void*) &res[1]);
  ftos(pck->accel.acc[2], (void*) &res[2]);
  PRINTF("ACC, index: %d, counter: %d, x: %s, y: %s, z: %s \n", pck->accel.sampleIndex, pck->accel.sampleCounter, res[0], res[1], res[2]);

  memset(res, 0, sizeof(res));     // reset the vector
  ftos(pck->inclination.inclination[0], (void*) &res[0]);
  ftos(pck->inclination.inclination[1], (void*) &res[1]);
  ftos(pck->inclination.inclination[2], (void*) &res[2]);
  PRINTF("Inclination, x: %s, y: %s, z: %s \n", res[0], res[1], res[2]);
  PRINTF("RSSI: %d\n", pck->rssi);
#endif /* DEBUG */
}



