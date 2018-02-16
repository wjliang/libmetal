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
 * @file	poll.h
 * @brief	Poll event handling primitives for libmetal.
 */

#ifndef __METAL_POLL__H__
#define __METAL_POLL__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup poll Poll Event Handling Interfaces
 *  @{ */

typedef int (*poll_func)(void *priv);

/** metal poll struct */
struct metal_poll_event {
	struct metal_event event; /**< event */
	poll_func poll; /**< poll function */
};

/**
 * @brief      Initialize poll event
 * @param[in]  poll_event pointer to poll event, user is expected to have
 *             allocated memory for the poll event.
 * @param[in]  pfunc poll function
 * @param[in]  poll_id private data of the poll event
 * @return     pointer to poll event if initialized successfully, NULL on
 *             failure
 */
struct metal_poll_event *metal_poll_event_init(struct metal_poll_event *poll,
					       poll_func pfunc, void *poll_id);

/**
 * @brief      Deinitialize poll event
 * @param[in]  poll_event pointer to poll event, user is expected to have
 *             allocated memory for the poll event.
 */
void metal_poll_event_deinit(struct metal_poll_event *poll);

/**
 * @brief      Register poll event handler
 *
 * @param[in]  event pointer to the event
 * @param[in]  hd_func event handler function
 * @param[in]  priv private data to the event handler
 * @return     registered event handle pointer for success, NULL on failure
 */
int metal_poll_event_register(struct metal_poll_event *event,
			      metal_event_hd_func hd_func,
			      void *priv);

/**
 * @brief     Unregister poll event handler
 *
 * @param[in]  event pointer to the event
 * @param[in]  hd_func event handler function
 * @param[in]  priv private data to the event handler
 */
void metal_poll_event_unregister(struct metal_poll_event *event,
				 struct metal_event_hd *hd, void *priv);

/**
 * @brief metal poll event handling
 *
 * @event pointer to the poll event
 * @timeout poll timeout
 *
 * @return 0 for success, negative value on failure
 */
int metal_poll(struct metal_poll_event *event, unsigned long long timeout);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_POLL__H__ */
