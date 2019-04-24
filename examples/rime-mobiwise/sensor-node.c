/*
 * Copyright (c) 2018, University of Coimbra.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Sensor Node Primitives.
 * \author
 *         Joel Mollering
 */

#include "contiki.h"                   /* Contiki OS */
#include "net/rime/rime.h"             /* Rime Network Stack Protocol */
#include "dev/serial-line.h"           /* For Serial Input Commands */
#include "dev/sht11/sht11-sensor.h"    /* SHT11 Temperature and Humidity Sensor */
#include "net/rime/packetqueue.h"      /* Packet Queue Management */
#include "dev/battery-sensor.h"        /* Battery Sensor */
#include "lib/random.h"                /* Random Number Generation */
#include "neighbor.c"                  /* Neighbord List*/
#include <string.h>
#include <stdio.h>

#include "msg-struct.h"

#define MAX_PACKET_QUEUE  10
#define TIMESLOT 10 //seconds
#define NETWORK_STABILIZE 20 //seconds

#define DEBUG 1
#if DEBUG
#define DBG(...) printf(__VA_ARGS__);
#else
#define DBG(...)
#endif

static struct mesh_conn mesh;
static linkaddr_t sink_node_addr;
static linkaddr_t this_node_addr;
long double battery = 1000000.0;

/* Define a packet queue for sending */
PACKETQUEUE(sensor_packet_queue, MAX_PACKET_QUEUE);

/*---------------------------------------------------------------------------*/
PROCESS(sensor_node_process, "Sensor Node Process");
PROCESS(serial_input, "Serial Commands Process");
PROCESS(broadcast_process, "Broadcast process");
AUTOSTART_PROCESSES(&sensor_node_process, &serial_input, &broadcast_process);
/*---------------------------------------------------------------------------*/
/*
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from, uint8_t hops)
{
   printf("Data received from %d.%d: %.*s (%d)\n",
	 from->u8[0], from->u8[1],
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

  if (packetqueue_enqueue_packetbuf(&sensor_packet_queue, 0, c)) {
    printf("Received packet saved into packet queue.\n");
  } else {
    printf("Received packet could not be saved into packet queue.\n");
  }

}

//---------------------------------------------------------------------------/
static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(linkaddr_cmp(dest, &linkaddr_null)) {
    return;
  }
  printf("unicast message sent to %d.%d: status %d num_tx %d\n",
    dest->u8[0], dest->u8[1], status, num_tx);
}
//---------------------------------------------------------------------------/
static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};
static struct unicast_conn uc;
//---------------------------------------------------------------------------/
*/
// mesh callback triggered when a packet is succesfully sent by the multihop routines /
static void
sent(struct mesh_conn *c)
{
  printf("Packet sent!\n");
}

/* mesh callback triggered when a packet suffers a timeout */
static void
timedout(struct mesh_conn *c, uint8_t timeout_src)
{
  printf("Packet timedout!\n");

  if (timeout_src) {
    printf("Route not found, retrying.\n");
    route_discovery_discover(&(mesh.route_discovery_conn),
        &sink_node_addr, CLOCK_SECOND * 10);
  }
}

/* mesh callback triggered when the node receives a packet */
static void
recv(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
  printf("Data received from %d.%d: %.*s (%d)\n",
	 from->u8[0], from->u8[1],
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

  if (packetqueue_enqueue_packetbuf(&sensor_packet_queue, 0, c)) {
    printf("Received packet saved into packet queue.\n");
  } else {
    printf("Received packet could not be saved into packet queue.\n");
  }
  update_battery(0,0,1,1);
}

/* Shows the current routing table of the node */
static void
route_print_table(void)
{
  struct route_entry *e = NULL;
  int i = 0;

  printf("Showing all entries from routing table.\n");
  do {
    e = route_get(i);
    if (e == NULL) {
      break;
    }
    printf("Route to %d.%d with nexthop %d.%d and cost %d.\n",
      e->dest.u8[0], e->dest.u8[1],
      e->nexthop.u8[0], e->nexthop.u8[1],
      e->cost);
    ++i;
  } while (1);
}

/* Starts the process of discover a route to the Sink Node */
static void
route_start_discovery(void)
{
  printf("Route Discovery Started.\n");
  route_discovery_discover(&(mesh.route_discovery_conn),
        &sink_node_addr, CLOCK_SECOND * 10);
}

/* Triggers the proess of sending a message to the Sink Node */
static void
send_msg(void)
{
  printf("Send message Started.\n");

  /* Check queue first. */
  int queue_length = packetqueue_len(&sensor_packet_queue);
  if (queue_length == 0) {
   printf("No packets in queue. There is nothing to send.\n");
    return;
  }
  /* In case there is something to send. */
  struct packetqueue_item *item; 
  struct queuebuf *queue;
  int i;
  struct neighbor *n;
  linkaddr_t to = { { 0, 0 } };
  int hops_aux = 200;
  long double batt_aux = 1;
  
  //Choose destination
  for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n))
  {
       if(linkaddr_cmp(&n->addr, &sink_node_addr))
       {
         linkaddr_copy(&to, &n->addr);
         hops_aux = -1;
       }
       else
       if(n->hops <= hops_aux)
       {
          if(n->hops < hops_aux)
          {
               linkaddr_copy(&to, &n->addr);
               hops_aux = n->hops;
               batt_aux = n->batt;
          }
           if(n->hops == hops_aux)
           {
              if(n->batt > batt_aux)
              {
               linkaddr_copy(&to, &n->addr);
               hops_aux = n->hops;
               batt_aux = n->batt;                
              }
           }
       }
  }
 
  /* Run through the queue. */
  for (i = 0; i < queue_length; ++i) {
    /* We should send the first packet from the queue. */
    item = packetqueue_first(&sensor_packet_queue);
    queue = packetqueue_queuebuf(item);

    /* Place the queued packet into the packetbuf. */
    queuebuf_to_packetbuf(queue);    
   
    /* Send the packet to the Node. */
    printf("Sending mensage to %d.%d\n", to.u8[0],to.u8[1]);
    if (mesh_send(&mesh, &to)) {
      packetqueue_dequeue(&sensor_packet_queue);
      printf("Message to Sink sent.\n");
      //return;
    } else {
      printf("Message to Sink NOT sent.\n");
      //return;
    }
   update_battery(1,1,0,0);
  }
   
}

/* Triggers the process of performing a sensing */
float
floor(float x)
{
  if(x >= 0.0f) {
    return (float)((int)x);
  } else {
    return (float)((int)x - 1);
  }
}

static void
sensor_perform_sensing(void)
{
  int val, dec;
  uint16_t bat = 0;
  float frac, s = 0.0;
  unsigned int timestamp = 0;
  
  printf("Sensors started.\n");
  SENSORS_ACTIVATE(sht11_sensor);
  SENSORS_ACTIVATE(battery_sensor);

  bat = battery_sensor.value(0);
  val = sht11_sensor.value(SHT11_SENSOR_TEMP);
    
  float mv = (bat * 2.500 * 2)/4096;

  if (val != -1) {
    s = ((0.01*val) - 39.60);
    dec = s;
    frac = s - dec;

    #if TIMESYNCH_CONF_ENABLED
        timestamp = timesynch_time();
    #else    
        timestamp = clock_time();
      
    #endif

    printf("Temp: %d.%02u C (%d) | Bat: %i (%ld.%03d mV) | Timestamp: %u (clock ticks)\n", 
      dec, (unsigned int)(frac*100), val,
      bat, (long)mv, (unsigned)((mv - floor(mv)) * 1000),
      timestamp);

    // Insert the reading into a packet in packetbuf. 
    packetbuf_clear();
    packetbuf_set_datalen(sprintf(packetbuf_dataptr(), 
            "Temp: %d.%02u C | Bat: %d mV | Timestamp: %u", 
              dec,
              (unsigned int)(frac*100), 
              ((bat*5*1000)/4095) + 1,
              (timestamp)));
    //packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE, PACKETBUF_ATTR_PACKET_TYPE_TIMESTAMP);
    packetbuf_set_addr(PACKETBUF_ADDR_ESENDER, &this_node_addr);

    if (packetqueue_enqueue_packetbuf(&sensor_packet_queue, 0, &mesh)) {
      printf("Sensing saved into packet queue.\n");
    } else {
      printf("Sensing could not be saved into packet queue.\n");
    }
  } else {
    printf("Error reading temperature.\n");
  }
  SENSORS_DEACTIVATE(sht11_sensor);
  SENSORS_DEACTIVATE(battery_sensor);
}


/* Shows the packet queue contents of the node */
static void
queue_show(void)
{
  int queue_length = packetqueue_len(&sensor_packet_queue);
  if (queue_length == 0) {
    printf("No packets in queue.\n");
    return;
  }

  struct packetqueue_item *item = packetqueue_first(&sensor_packet_queue);
  int i;
  for (i = 0; i < queue_length; ++i) {
    struct queuebuf *buf = packetqueue_queuebuf(item);
    linkaddr_t *esender = queuebuf_addr(buf, PACKETBUF_ADDR_ESENDER);
    linkaddr_t *sender  = queuebuf_addr(buf, PACKETBUF_ADDR_SENDER);

    printf("Packet #%d | From %d.0 | Prev %d.0 | Hops %d | Payload: \"%s\"\n", i+1, 
       esender->u8[0],
       sender->u8[0],
       (int)queuebuf_attr(buf, PACKETBUF_ATTR_HOPS),
       (char *)queuebuf_dataptr(packetqueue_queuebuf(item)));

    item = item->next;
  }
}

const static struct mesh_callbacks callbacks = {recv, sent, timedout};
/*---------------------------------------------------------------------------*/
/* Main Process */
PROCESS_THREAD(sensor_node_process, ev, data)
{
  

  this_node_addr = linkaddr_node_addr;
  sink_node_addr.u8[0] = 1;
  sink_node_addr.u8[1] = 0;
  
  PROCESS_EXITHANDLER(mesh_close(&mesh);)
  //PROCESS_EXITHANDLER(unicast_close(&uc);)
  //unicast_open(&uc, 146, &unicast_callbacks);

  unsigned seconds = 20;
  double fixed_perc_energy = 1.0;
  unsigned variation = 1;



  PROCESS_BEGIN();
  DBG("[Sensor Node %d] Process begin.\n", this_node_addr.u8[0]);

  powertrace_start(CLOCK_SECOND * seconds, seconds, fixed_perc_energy, variation);
  DBG("[Sensor Node %d] Powertrace started.\n", this_node_addr.u8[0]);

  mesh_open(&mesh, 132, &callbacks);
  DBG("[Sensor Node %d] Opened Unicast Connection.\n", this_node_addr.u8[0]);

  packetqueue_init(&sensor_packet_queue);
  DBG("[Sensor Node %d] Initialized Packet Queue.\n", this_node_addr.u8[0]);

  static struct etimer et, timeslot_wait;
  random_init(this_node_addr.u8[0]); 
  etimer_set(&et, random_rand() % (TIMESLOT*CLOCK_SECOND));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  route_start_discovery();
  DBG("[Sensor Node %d] Started Looking for Sink.\n", this_node_addr.u8[0]);

  //static struct etimer sinc;
 

   /* Allow some time for the network to settle. */
  etimer_set(&et, NETWORK_STABILIZE * CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));

  while(1) {  
  
    // Send a packet every TIMESLOT seconds. 
    etimer_set(&timeslot_wait,(CLOCK_SECOND * TIMESLOT));
    {
     sensor_perform_sensing();
     send_msg();
    }
    PROCESS_WAIT_UNTIL(etimer_expired(&timeslot_wait));
  
  //PROCESS_YIELD();
  //etimer_set(&timeslot_wait,TIMESLOT*CLOCK_SECOND);
  //sensor_perform_sensing();
  //send_msg();
  //PROCESS_YIELD();
  //etimer_set(&sinc,5*CLOCK_SECOND);
  //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sinc));
  //DBG("Node Clock Time: %d (seconds)\n", timesynch_time());
  //DBG("Node Clock Time: %d (clock ticks)\n", clock_time());
  
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Process that handles serial commands */
PROCESS_THREAD(serial_input, ev, data)
{
 PROCESS_BEGIN();

 for(;;) {
   PROCESS_YIELD();
   if(ev == serial_line_event_message) {
     DBG("Received Serial Input: %s\n", (char *)data);
     if (!strcmp((char *)data, "routes")) {
      route_print_table();
     } else if (!strcmp((char *)data, "discover")) {
      route_start_discovery();
     } else if (!strcmp((char *)data, "send")) {
      send_msg();
     } else if (!strcmp((char *)data, "sense")) {
      sensor_perform_sensing();
     }else if(!strcmp((char *)data,"test")){
       printf("time test %d", (int)clock_time());
     } else if (!strcmp((char *)data, "queue")) {
      queue_show();
     }else if (!strcmp((char *)data, "battery")) {
      printf("Battery charge: %lu\n", (unsigned long)get_battery_charge());
     } else if (!strcmp((char *)data, "neighbors")){
      print_neighbors();
     } else {
      printf("Unknown command.\n");
     }
   }
 }
 PROCESS_END();
}
/*---------------------------------------------------------------------------*/