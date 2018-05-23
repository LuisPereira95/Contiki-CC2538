/**
 * \file
 *         A low power RDC implementation that uses framer for headers.
 * \author
 */

#include "net/mac/mac-sequence.h"
#include "lprdc.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/netstack.h"
#include "net/rime/rimestats.h"
#include <string.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifdef NULLRDC_CONF_ADDRESS_FILTER
#define NULLRDC_ADDRESS_FILTER NULLRDC_CONF_ADDRESS_FILTER
#else
#define NULLRDC_ADDRESS_FILTER 1
#endif /* NULLRDC_CONF_ADDRESS_FILTER */

#ifndef NULLRDC_802154_AUTOACK_HW
#ifdef NULLRDC_CONF_802154_AUTOACK_HW
#define NULLRDC_802154_AUTOACK_HW NULLRDC_CONF_802154_AUTOACK_HW
#else
#define NULLRDC_802154_AUTOACK_HW 0
#endif /* NULLRDC_CONF_802154_AUTOACK_HW */
#endif /* NULLRDC_802154_AUTOACK_HW */


#include "sys/rtimer.h"
#include "dev/watchdog.h"

#ifdef NULLRDC_CONF_ACK_WAIT_TIME
#define ACK_WAIT_TIME NULLRDC_CONF_ACK_WAIT_TIME
#else /* NULLRDC_CONF_ACK_WAIT_TIME */
//#define ACK_WAIT_TIME                      RTIMER_SECOND / 667
#define ACK_WAIT_TIME                      RTIMER_SECOND / 1500
#endif /* NULLRDC_CONF_ACK_WAIT_TIME */

#ifdef NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#define AFTER_ACK_DETECTED_WAIT_TIME NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#else /* NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME */
//#define AFTER_ACK_DETECTED_WAIT_TIME       RTIMER_SECOND / 1500
#define AFTER_ACK_DETECTED_WAIT_TIME       0  // TENTATIVA PARA VER O QUE ACONTECE
#endif /* NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME */

#ifdef LPRDC_CONF_PCK_WAIT_TIME
#define PCK_WAIT_TIME LPRDC_CONF_PCK_WAIT_TIME
#else /* LPRDC_CONF_PCK_WAIT_TIME */
#define PCK_WAIT_TIME                      RTIMER_SECOND / 667
#endif /* LPRDC_CONF_PCK_WAIT_TIME */



#define ACK_LEN 3

static struct rtimer rt;
static volatile uint8_t lprdc_keep_radio_on = 0;
static volatile uint8_t radio_is_on = 0;

static void on();
static void off();
/*---------------------------------------------------------------------------*/
static void
turn_off_by_timeout(struct rtimer *t, void *ptr)
{
    PRINTF("lprdc: imeout \n");

    off();
}
/*---------------------------------------------------------------------------*/
static void
shedule_power_off(rtimer_clock_t fixed_time)
{

  if(!lprdc_keep_radio_on) {  // only shedule if the radio should be disable
      PRINTF("lprdc: shedule sht \n");
    if(RTIMER_CLOCK_LT(fixed_time, RTIMER_NOW() + 1)) { // check if fixed_time is not too high
      fixed_time = RTIMER_NOW() + 1;
    }
    //r = rtimer_set(t, fixed_time, 1, (void (*)(struct rtimer *, void *))turn_off, NULL);
    rtimer_set(&rt, fixed_time, 1, (void (*)(struct rtimer *, void *))turn_off_by_timeout, NULL);
  }
}


/*---------------------------------------------------------------------------*/
static int
send_one_packet(mac_callback_t sent, void *ptr)
{
  int ret;
  int last_sent_ok = 0;

  PRINTF("lprdc: Send\n");
  on();

  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
#if NULLRDC_802154_AUTOACK_HW
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);
#endif /* NULLRDC_802154_AUTOACK_HW */

  if(NETSTACK_FRAMER.create_and_secure() < 0) {
    /* Failed to allocate space for headers */
    PRINTF("lprdc: send failed, too large header\n");
    ret = MAC_TX_ERR_FATAL;
  } else {
    int is_broadcast;
    uint8_t dsn;
    dsn = ((uint8_t *)packetbuf_hdrptr())[2] & 0xff;

    NETSTACK_RADIO.prepare(packetbuf_hdrptr(), packetbuf_totlen());

    is_broadcast = packetbuf_holds_broadcast();

    if(NETSTACK_RADIO.receiving_packet() ||
       (!is_broadcast && NETSTACK_RADIO.pending_packet())) {

      /* Currently receiving a packet over air or the radio has
         already received a packet that needs to be read before
         sending with auto ack. */
      ret = MAC_TX_COLLISION;
    } else {
      if(!is_broadcast) {
        RIMESTATS_ADD(reliabletx);
      }

      switch(NETSTACK_RADIO.transmit(packetbuf_totlen())) {
      case RADIO_TX_OK:
        if(is_broadcast) {
          ret = MAC_TX_OK;
        } else {
          rtimer_clock_t wt;

          /* Check for ack */
          wt = RTIMER_NOW();
          watchdog_periodic();
          while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + ACK_WAIT_TIME)) { }

          ret = MAC_TX_NOACK;
          if(NETSTACK_RADIO.receiving_packet() ||
             NETSTACK_RADIO.pending_packet() ||
             NETSTACK_RADIO.channel_clear() == 0) {
            int len;
            uint8_t ackbuf[ACK_LEN];

            if(AFTER_ACK_DETECTED_WAIT_TIME > 0) {
              wt = RTIMER_NOW();
              watchdog_periodic();
              while(RTIMER_CLOCK_LT(RTIMER_NOW(),
                                    wt + AFTER_ACK_DETECTED_WAIT_TIME)) { }
            }

            if(NETSTACK_RADIO.pending_packet()) {
              len = NETSTACK_RADIO.read(ackbuf, ACK_LEN);
              if(len == ACK_LEN && ackbuf[2] == dsn) {
                /* Ack received */
                RIMESTATS_ADD(ackrx);
                ret = MAC_TX_OK;
              } else {
                /* Not an ack or ack not for us: collision */
                ret = MAC_TX_COLLISION;
              }
            }
          } else {
	    PRINTF("lprdc tx noack\n");
	  }
        }
        break;
      case RADIO_TX_COLLISION:
        ret = MAC_TX_COLLISION;
        break;
      default:
        ret = MAC_TX_ERR;
        break;
      }
    }

  }
  if(ret == MAC_TX_OK) {
    last_sent_ok = 1;
    shedule_power_off(RTIMER_NOW() + PCK_WAIT_TIME);    // shedule a timer to power down the radio after
    // if we received a ack, wait a bit more time for the possible data packet
  } else {  // if not receive a ack turn off now
      off();
  }
  mac_call_sent_callback(sent, ptr, ret, 1);
  return last_sent_ok;
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  send_one_packet(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
  while(buf_list != NULL) {
    /* We backup the next pointer, as it may be nullified by
     * mac_call_sent_callback() */
    struct rdc_buf_list *next = buf_list->next;
    int last_sent_ok;

    queuebuf_to_packetbuf(buf_list->buf);
    last_sent_ok = send_one_packet(sent, ptr);

    /* If packet transmission was not successful, we should back off and let
     * upper layers retransmit, rather than potentially sending out-of-order
     * packet fragments. */
    if(!last_sent_ok) {
      return;
    }
    buf_list = next;
  }
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
#if NULLRDC_SEND_802154_ACK
  int original_datalen;
  uint8_t *original_dataptr;

  original_datalen = packetbuf_datalen();
  original_dataptr = packetbuf_dataptr();
#endif



  if(packetbuf_datalen() == ACK_LEN) {
    /* Ignore ack packets */
    PRINTF("lprdc: ignored ack\n"); 
  } else if(NETSTACK_FRAMER.parse() < 0) {
    PRINTF("lprdc: failed to parse %u\n", packetbuf_datalen());
#if NULLRDC_ADDRESS_FILTER
  } else if(!linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                                         &linkaddr_node_addr) &&
            !packetbuf_holds_broadcast()) {
    PRINTF("lprdc: not for us\n");
#endif /* NULLRDC_ADDRESS_FILTER */
  } else {
    int duplicate = 0;

#if NULLRDC_802154_AUTOACK_HW
#if RDC_WITH_DUPLICATE_DETECTION
    /* Check for duplicate packet. */
    duplicate = mac_sequence_is_duplicate();
    if(duplicate) {
      /* Drop the packet. */
      PRINTF("lprdc: drop duplicate link layer packet %u\n",
             packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO));
    } else {
      mac_sequence_register_seqno();
    }
#endif /* RDC_WITH_DUPLICATE_DETECTION */
#endif /* NULLRDC_802154_AUTOACK_HW */

/* TODO We may want to acknowledge only authentic frames */ 
#if NULLRDC_SEND_802154_ACK
    {
      frame802154_t info154;
      frame802154_parse(original_dataptr, original_datalen, &info154);
      if(info154.fcf.frame_type == FRAME802154_DATAFRAME &&
         info154.fcf.ack_required != 0 &&
         linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
                      &linkaddr_node_addr)) {
        uint8_t ackdata[ACK_LEN] = {0, 0, 0};

        ackdata[0] = FRAME802154_ACKFRAME;
        ackdata[1] = 0;
        ackdata[2] = info154.seq;
        NETSTACK_RADIO.send(ackdata, ACK_LEN);
      }
    }
#endif /* NULLRDC_SEND_ACK */
    if(!duplicate) {
      NETSTACK_MAC.input();
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
on(void)
{
  if(!radio_is_on) {
      radio_is_on = 1;
      NETSTACK_RADIO.on();
      PRINTF("lprdc: radio on\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  PRINTF("lprdc:rad_on: %d; keep: %d\n",radio_is_on, lprdc_keep_radio_on);
  if(radio_is_on && !lprdc_keep_radio_on) {
    radio_is_on = 0;
    NETSTACK_RADIO.off();
    PRINTF("lprdc: radio off\n");
  }
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  lprdc_keep_radio_on = 1;
  radio_is_on = 0;
  on();
}
static int
turn_on(void)
{
  if(!radio_is_on) {
    radio_is_on = 1;
    NETSTACK_RADIO.on();
    PRINTF("lprdc: radiot on\n");
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  lprdc_keep_radio_on = keep_radio_on;
  if(keep_radio_on) {
    radio_is_on = 1;
    PRINTF("lprdc: radioo on\n");
    return NETSTACK_RADIO.on();
  } else {
    radio_is_on = 0;
    PRINTF("lprdc: radioo off\n");
    return NETSTACK_RADIO.off();
  }
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver lprdc_driver = {
  "lprdc",
  init,
  send_packet,
  send_list,
  packet_input,
  turn_on,
  turn_off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
