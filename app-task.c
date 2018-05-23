/**
 *
 * \file
 *      Manage the received messages.
 */
// criar um timer a enviar a mensagem de request até o estado do device ficar conectado.
#include "app-task.h"

#define DEBUG DEBUG_PRINT
//#if DEBUG
#include "net/ip/uip-debug.h"
//#endif /* DEBUG */

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN]) // received package


/**
 * \brief Parsing the message received by OTA(over the air)
 *
 * \param pck The pck struct receive
 *
 */
void
input_message(uint8_t *data) {
    pck_t pck;
   // dest_conn = c;


    if (!pck_parse(data, &pck)) {    // if some error occur, drop the package
        PRINTF("message: the pck received is not valid \n");
        return;
    }

    switch (pck.pck_id.message_type) {  // send to a specific function to manage the data
      case MSG_CONECTION:
          parse_connection(pck.pck_id.mask);
        break;
      case MSG_CONFIGURATION:
          parse_configuration(&pck);
        break;
      case MSG_KEEPALIVE:
          parse_keepalive(&pck);
        break;
      default:
        break;
  }
}

/**
 * \brief Parsing the conection messages ( association, response, and dissociation)
 *
 * \param content Define the message type
 *
 */
void
parse_connection(uint8_t content) {
  pck_t pck;

  switch (content) {
#if UIP_CONF_ROUTER  // only the coordinator can receive this message
    case MSG_CONECTION_ASSOCIATION_REQUEST:
      // adicionar à tabela de dispositivos associados
      // manda mensagem de configuração
      pck.pck_id.message_type = MSG_CONECTION;
      pck.pck_id.device_type = DEST_COORDINATOR;
      pck.pck_id.mask = MSG_CONECTION_ASSOCIATION_RESPONSE;
      send_message(&pck, dest_conn);

      break;
    case MSG_CONECTION_CONFIGURATION_REQUEST:
        pck.pck_id.message_type = MSG_CONFIGURATION;
        pck.pck_id.device_type = DEST_COORDINATOR;
        pck.pck_id.mask = MSG_CONFIGURATION_KEEP_ALIVE_DATA;
        pck.keepalive_data = MSG_KEEPALIVE_RSSI;
        send_message(&pck, dest_conn);


      break;
#else
    case MSG_CONECTION_ASSOCIATION_RESPONSE:

        pck.pck_id.message_type = MSG_CONECTION;
        pck.pck_id.device_type = DEST_END_DEVICE;
        pck.pck_id.mask = MSG_CONECTION_CONFIGURATION_REQUEST;
        send_message(&pck, dest_conn);

      // device is associated
      break;
#endif /* UIP_CONF_ROUTER */
    default:
      PRINTF("app_task: connection message not suported \n");
      break;
  }
}

/*----------------------------------------------------------------------------*/
/**
 * \brief Parsing the configuration messages (KA data and Sampling rate)
 *
 * \param pck       The pck struct receive
 * \param content   Define the message type
 *
 */

void
parse_configuration(pck_t *pck) {
//#if !UIP_CONF_ROUTER
  uint8_t content = pck->pck_id.mask;
#if !UIP_CONF_ROUTER
  device_state = DEV_END_DEVICE;
  process_post(&sensor_node_process, PROCESS_EVENT_CONTINUE, NULL);
#endif

  if(content & MSG_CONFIGURATION_KEEP_ALIVE_DATA) {

  }
  if(content & MSG_CONFIGURATION_SAMPLING_RATE) {

  }
  if(device_state != DEV_END_DEVICE) { // if is the first configuration message
    // desligar o radio, configurando-o como low power
    device_state = DEV_END_DEVICE;
  }
//#endif /* UIP_CONF_ROUTER */
}

/*----------------------------------------------------------------------------*/
/**
 * \brief Parsing the Keep Alive messages ( received data)
 *
 * \param pck       The pck struct receive
 * \param content   Define the content of message
 *
 */

void
parse_keepalive(pck_t *pck) {
#if UIP_CONF_ROUTER
  uint8_t content = pck->pck_id.mask;

//  if(content & MSG_KEEPALIVE_ACCELERATION) {
//  }
//  if(content & MSG_KEEPALIVE_INCLINATION) {
//  }
//  if(content & MSG_KEEPALIVE_RSSI) {
//  }
  // ENVIAR ESTA INFORMAÇÃO POR JSON
#endif /* UIP_CONF_ROUTER */
}

/*
 * Sends a application message to a specific device
 *
 * This function send a message Over-the_Air
 *
 * \param pck The pck struct to send type
 * \param pck The uip_udp struct that contain the destination address and udp ports
 *
 */
void
send_message(pck_t *params, struct uip_udp_conn *conn) {
    int len;
    uint8_t buf[MAX_MSG_APP_SIZE];

    len = pck_create(params, (void*) &buf);


    //uip_ipaddr_copy(&dest_conn->ripaddr, &UIP_IP_BUF->srcipaddr); // put the origin address in the destination address
    uip_udp_packet_send(dest_conn, buf, len);
    PRINTF("Sending to ");
    PRINT6ADDR(&dest_conn->ripaddr);
    PRINTF(" local/remote port %u/%u\n", UIP_HTONS(dest_conn->lport),
           UIP_HTONS(dest_conn->rport));
}
