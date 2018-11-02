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

#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/mesh.h"

#include "dev/button-sensor.h"

#include <stdio.h>
#include <string.h>

#define MESSAGE "Hello!"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__);
#else
#define PRINTF(...)
#endif

static struct mesh_conn mesh;
/*---------------------------------------------------------------------------*/
PROCESS(sensor_node_process, "Sensor Node Process");
AUTOSTART_PROCESSES(&sensor_node_process);
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

const static struct mesh_callbacks callbacks = {recv, sent, timedout};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_node_process, ev, data)
{
  static struct etimer et;
  linkaddr_t sink_node_addr, this_node_addr = linkaddr_node_addr;
  sink_node_addr.u8[0] = 1;
  sink_node_addr.u8[1] = 0;


  PROCESS_EXITHANDLER(mesh_close(&mesh);)
  PROCESS_BEGIN();
  PRINTF("[Sensor Node %d] Process begin.\n", this_node_addr.u8[0]);

  // Wait six seconds before starting
  etimer_set(&et, 6*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  mesh_open(&mesh, 132, &callbacks);
  PRINTF("[Sensor Node %d] Opened Mesh Connection.\n", this_node_addr.u8[0]);

  SENSORS_ACTIVATE(button_sensor);

  while(1) {
    

    /* Wait for button click before sending the first message. */
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);

    printf("[Sensor Node %d] Button clicked!\n", this_node_addr.u8[0]);

    /* Send a message to the sink node. */
    packetbuf_copyfrom(MESSAGE, strlen(MESSAGE));
    mesh_send(&mesh, &sink_node_addr);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
