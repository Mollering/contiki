/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         Multihop forwarding
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup rime
 * @{
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/multihop.h"
#include "net/rime/route.h"

#include <string.h>

static const struct packetbuf_attrlist attributes[] =
  {
    MULTIHOP_ATTRIBUTES
    PACKETBUF_ATTR_LAST
  };

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void
data_packet_received(struct unicast_conn *uc, const linkaddr_t *from)
{
  struct multihop_conn *c = (struct multihop_conn *)uc;
  //linkaddr_t *nexthop;
  linkaddr_t sender, receiver;

  /* Copy the packet attributes to avoid them being overwritten or
     cleared by an application program that uses the packet buffer for
     its own needs. */
  linkaddr_copy(&sender, packetbuf_addr(PACKETBUF_ADDR_SENDER));
  linkaddr_copy(&receiver, packetbuf_addr(PACKETBUF_ADDR_ERECEIVER));

  PRINTF("data_packet_received from %d.%d towards %d.%d len %d\n",
	 from->u8[0], from->u8[1],
	 packetbuf_addr(PACKETBUF_ADDR_ERECEIVER)->u8[0],
	 packetbuf_addr(PACKETBUF_ADDR_ERECEIVER)->u8[1],
	 packetbuf_datalen());

  if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_ERECEIVER), &linkaddr_node_addr))
  { /* In case the packet reached the destination. */
    PRINTF("Packet is for us!\n");
    if(c->cb->recv)
    {
      c->cb->recv(c, &sender, from, packetbuf_attr(PACKETBUF_ATTR_HOPS));
    }
  }
  else /* In case the packet did not reached the destination. */
  {
    // nexthop = NULL;
    // if(c->cb->forward) /* If we have set a mesh forwarding callback, let's call it. */
    // {
    //   packetbuf_set_attr(PACKETBUF_ATTR_HOPS, packetbuf_attr(PACKETBUF_ATTR_HOPS) + 1);
    //   nexthop = c->cb->forward(c, &sender, &receiver, from, packetbuf_attr(PACKETBUF_ATTR_HOPS) - 1);
    // }

    // if(nexthop) /* We have found the next hop, let's send the packet. */I
    // {
    //   PRINTF("forwarding to %d.%d\n", nexthop->u8[0], nexthop->u8[1]);
    //   unicast_send(&c->c, nexthop);
    // }
    c->cb->recv(c, &sender, from, packetbuf_attr(PACKETBUF_ATTR_HOPS));
  }
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks data_callbacks = { data_packet_received };
/*---------------------------------------------------------------------------*/
void
multihop_open(struct multihop_conn *c, uint16_t channel,
	const struct multihop_callbacks *callbacks)
{
  unicast_open(&c->c, channel, &data_callbacks);
  channel_set_attributes(channel, attributes);
  c->cb = callbacks;
}
/*---------------------------------------------------------------------------*/
void
multihop_close(struct multihop_conn *c)
{
  unicast_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
multihop_send(struct multihop_conn *c, const linkaddr_t *to)
{
  //linkaddr_t *nexthop;

  /* If there is no registered multihop forward callback, 
   * then the packet can not be sent.
   */
  if(c->cb->forward == NULL) {
    return 0;
  }

  /* If it is possible to send, put the packet nice and tidy */
  packetbuf_compact();
  packetbuf_set_addr(PACKETBUF_ADDR_ERECEIVER, to);
  const packetbuf_attr_t hopscount = packetbuf_attr(PACKETBUF_ATTR_HOPS) + 1; 
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  packetbuf_set_attr(PACKETBUF_ATTR_HOPS, hopscount);
  /* Find where to send it by using the address returned by
   * the multihop forward callback.
   */
  //nexthop = c->cb->forward(c, &linkaddr_node_addr, to, NULL, 0);
  
  PRINTF("multihop_send: sending data towards %d.%d\n",
	   nexthop->u8[0], nexthop->u8[1]);
    unicast_send(&c->c, to);

  /*
  if(nexthop == NULL) {                 // If there is no address to send the packet to, /
    
    PRINTF("multihop_send: no route\n");// then we say that there is nothing to do. /
    return 0;

  } else { // We have a next hop, so let's proceed with a node-to-node communication. 
    
    PRINTF("multihop_send: sending data towards %d.%d\n",
	   nexthop->u8[0], nexthop->u8[1]);
    unicast_send(&c->c, nexthop);

    return 1;
  }
  */
}

/*-----------------------------------------t---------------------------------*/
void
multihop_resend(struct multihop_conn *c, const linkaddr_t *nexthop)
{
  unicast_send(&c->c, nexthop);
}
/*---------------------------------------------------------------------------*/
/** @} */
