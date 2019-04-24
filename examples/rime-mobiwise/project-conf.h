/*
 * Copyright (c) 2016, Inria.
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
 *         Project config file
 * \author
 *         Marco Silva
 *
 */

#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__


#define NETSTACK_CONF_NETWORK rime_driver // Define the network driver to use
#define NETSTACK_CONF_MAC     csma_driver // Define the MAC driver to use
//#define NETSTACK_CONF_RDC     nullrdc_driver // Define the RDC driver to use. 
//#define NETSTACK_CONF_FRAMER  framer_802154 // Define the framer driver to use
#define NETSTACK_CONF_RADIO   cc2420_driver // Define the radio driver to use. 

/* Netstack layers */
#ifdef NETSTACK_CONF_WITH_IPV6
#undef NETSTACK_CONF_WITH_IPV6
#endif /* NETSTACK_CONF_WITH_IPV6 */

 
#undef TIMESYNCH_CONF_ENABLED  // TO ENABLE THE Implicit Network Time Synchronization
#define TIMESYNCH_CONF_ENABLED 1 // TO ENABLE THE Implicit Network Time Synchronization
 
//Define the channel to be used
#define RF_CHANNEL 20

#endif /* __PROJECT_CONF_H__ */
