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

#include <stdio.h>
#include <string.h>

#define MESSAGE           "Hello!"
#define MAX_PACKET_QUEUE  10

#define DEBUG 1
#if DEBUG
#define DBG(...) printf(__VA_ARGS__);
#else
#define DBG(...)
#endif

static struct mesh_conn mesh;
static linkaddr_t sink_node_addr;
static linkaddr_t this_node_addr;

/* Define a packet queue for sending */
PACKETQUEUE(sensor_packet_queue, MAX_PACKET_QUEUE);


/*---------------------------------------------------------------------------*/
PROCESS(sensor_node_process, "Sensor Node Process");
PROCESS(serial_input, "Serial Commands Process");
AUTOSTART_PROCESSES(&sensor_node_process, &serial_input);
/*---------------------------------------------------------------------------*/
static void
sent(struct mesh_conn *c)
{
  printf("Packet sent!\n");
}

static void
timedout(struct mesh_conn *c)
{
  printf("Packet timedout!\n");
}

static void
recv(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
  printf("Data received from %d.%d: %.*s (%d)\n",
	 from->u8[0], from->u8[1],
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

  //packetbuf_copyfrom(MESSAGE, strlen(MESSAGE));
  //mesh_send(&mesh, from);
}

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

static void
route_start_discovery(void)
{
  printf("Route Discovery Started.\n");
  route_discovery_discover(&(mesh.route_discovery_conn),
        &sink_node_addr, CLOCK_SECOND * 10);
}

static void
send_msg_to_sink(void)
{
  printf("Message to Sink Started.\n");
  /* Send a message to the sink node. */

  // /* packetbuf before */
  // printf("packetbuf before\n");
  // printf("dataptr: %p\n", packetbuf_dataptr());
  // printf("hdrptr: %p\n", packetbuf_hdrptr());
  // printf("datalen: %u\n", packetbuf_datalen());
  // printf("hdrlen: %u\n", packetbuf_hdrlen());
  // printf("totlen: %u\n", packetbuf_totlen());
  // printf("remlen: %u\n", packetbuf_remaininglen());

  // packetbuf_copyfrom(MESSAGE, strlen(MESSAGE));

  // /* packetbuf after */
  // printf("packetbuf after\n");
  // printf("dataptr: %p\n", packetbuf_dataptr());
  // printf("hdrptr: %p\n", packetbuf_hdrptr());
  // printf("datalen: %u\n", packetbuf_datalen());
  // printf("hdrlen: %u\n", packetbuf_hdrlen());
  // printf("totlen: %u\n", packetbuf_totlen());
  // printf("remlen: %u\n", packetbuf_remaininglen());

  if (mesh_send(&mesh, &sink_node_addr)) {
    printf("Message to Sink sent.\n");
  } else {
    printf("Message to Sink NOT sent.\n");
  }
}

static void
sensor_perform_sensing(void)
{
  int val, dec;
  float frac, s = 0.0;

  printf("Sensor started sensing.\n");
  SENSORS_ACTIVATE(sht11_sensor);
  val = sht11_sensor.value(SHT11_SENSOR_TEMP);
  if (val != -1) {
    s = ((0.01*val) - 39.60);
    dec = s;
    frac = s - dec;
    printf("Temp: %d.%02u C (%d)\n", dec, (unsigned int)(frac*100), val);

    /* Insert the reading into a packet in packetbuf. */
    packetbuf_clear();
    packetbuf_set_datalen(sprintf(packetbuf_dataptr(), 
            "Temp: %d.%02u C", dec, (unsigned int)(frac*100)) + 1);

    if (packetqueue_enqueue_packetbuf(&sensor_packet_queue, 0, &mesh)) {
      printf("Sensing saved into packet queue.\n");
    } else {
      printf("Sensing could not be saved into packet queue.\n");
    }
  } else {
    printf("Error reading temperature.\n");
  }
  
  SENSORS_DEACTIVATE(sht11_sensor);
}

static void
queue_show(void)
{
  struct packetqueue_item *item = packetqueue_first(&sensor_packet_queue);
  int queue_length = packetqueue_len(&sensor_packet_queue);
  //struct queuebuf *buffer = NULL;
  int i;

  for (i = 0; i < queue_length; ++i) {
    //printf("Packet #%d: %s\n", i, item->buf.ram_ptr.data); <<< TODO
    item = item->next;
  }
}

const static struct mesh_callbacks callbacks = {recv, sent, timedout};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_node_process, ev, data)
{
  this_node_addr = linkaddr_node_addr;
  sink_node_addr.u8[0] = 1;
  sink_node_addr.u8[1] = 0;
  PROCESS_EXITHANDLER(mesh_close(&mesh);)


  PROCESS_BEGIN();
  DBG("[Sensor Node %d] Process begin.\n", this_node_addr.u8[0]);

  mesh_open(&mesh, 132, &callbacks);
  DBG("[Sensor Node %d] Opened Mesh Connection.\n", this_node_addr.u8[0]);

  packetqueue_init(&sensor_packet_queue);
  DBG("[Sensor Node %d] Initialized Packet Queue.\n", this_node_addr.u8[0]);

  while(1) {
    PROCESS_YIELD();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
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
      send_msg_to_sink();
     } else if (!strcmp((char *)data, "sense")) {
      sensor_perform_sensing();
     } else if (!strcmp((char *)data, "queue")) {
      queue_show();
     } else {
      printf("Unknown command.\n");
     }
   }
 }
 PROCESS_END();
}
/*---------------------------------------------------------------------------*/