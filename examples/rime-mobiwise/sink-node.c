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
 *         Sink Node Primitives.
 * \author
 *         Joel Mollering
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/mesh.h"

#include <stdio.h>
#include <string.h>

#define DEBUG 1
#if DEBUG
#define DBG(...) DBG(__VA_ARGS__);
#else
#define DBG(...)
#endif

static struct mesh_conn mesh;
/*---------------------------------------------------------------------------*/
PROCESS(sink_node_process, "Sink Node Process");
AUTOSTART_PROCESSES(&sink_node_process);
/*---------------------------------------------------------------------------*/

static void
recv(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
  DBG("[Sink Node] Data received from %d.%d (%d hops): %.*s (%d)\n",
	 from->u8[0], from->u8[1], hops,
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

}

const static struct mesh_callbacks callbacks = {recv};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sink_node_process, ev, data)
{
  PROCESS_EXITHANDLER(mesh_close(&mesh);)
  static struct etimer periodic, et;

  PROCESS_BEGIN();
  DBG("[Sink Node] Process begin.\n");

  etimer_set(&periodic, CLOCK_SECOND/10UL);

  /* Wait two seconds before starting */
  etimer_set(&et, 2*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  mesh_open(&mesh, 132, &callbacks);
  DBG("[Sink Node] Opened Mesh Connection.\n");


  while(1) {
	  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic));
	  etimer_reset(&periodic);
  }

  DBG("[Sink Node] Process End.\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/