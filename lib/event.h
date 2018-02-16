/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * @file	event.h
 * @brief	Event handling primitives for libmetal.
 */

#ifndef __METAL_EVENT__H__
#define __METAL_EVENT__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup event Event Handling Interfaces
 *  @{ */

#include <metal/list.h>

/** IRQ handled status */
#define METAL_EVENT_NOT_HANDLED 0
#define METAL_EVENT_HANDLED     1

/**
 * @brief	type of metal event handler
 * @param[in]   eid event id
 * @param[in]	priv private data
 * @return      event handled status
 */
typedef int (*metal_event_hd_func) (void *eid, void *priv);

/** Libmetal event handler struct */
struct metal_event_hd {
	metal_event_hd_func hd_func; /**< handler function */
	void *priv; /**< pointer to the handler private data */
	struct metal_event *event; /**< pointer to the event */
	struct metal_list node; /**< list node */
};


/** Libmetal event structure */
struct metal_event {
	void *eid; /**<	event id */
	struct metal_list hds; /**< event handlers */
};

/**
 * @brief      Register event handler
 *
 * @param[in]  pointer to the event
 * @param[in]  hd_func event handler function
 * @param[in]  priv private data
 * @return     registered event handle pointer for success, NULL on failure
 */
struct metal_event_hd *metal_event_register(struct metal_event *event,
					    metal_event_hd_func hd_func,
					    void *priv);

/**
 * @brief     Unregister event handler
 *
 * @param[in] event event handler to be unregistered
 */
void metal_event_unregister(struct metal_event_hd *hd);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_EVENT__H__ */
