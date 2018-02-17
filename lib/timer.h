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
 * @file	timer.h
 * @brief	Timer primitives for libmetal.
 */

#ifndef __METAL_TIMER__H__
#define __METAL_TIMER__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup timer Timer Handling Interfaces
 *  @{ */

/**
 * @brief user defined function to start a timer
 *
 * @param[in] timer pointer to the metal timer
 *            the timer pointer is expected to be used when the timer
 *            expires.
 * @param[in] priv private data will be used by the function
 * @param[in] expire timer expiration
 * @return    0 for success, negative value for failure
 */
typedef int (*start_timer_func)(struct metal_timer *timer, void *priv,
				unsigned long long expire);

/**
 * @brief user defined function to stop a timer
 *
 * @param[in] timer pointer to the metal timer
 * @param[in] priv private data will be used by the function
 */
typedef void (*stop_timer_func)(struct metal_timer *timer, void *priv);

/** @brief metal timer struct */
struct metal_timer {
	struct metal_event evt; /**< metal event */
	unsigned long long expire_val; /**< expiration value, unit is tick.
					    when the counter value is equal
					    to the expire value, timer expires
					*/
	start_timer_func start_timer; /**< user defined to start a timer.
					   When the timer expires
					   metal_timer_handler() function is
					   expected to be called.
				       */ 
	stop_timer_func stop_timer; /**< user defined to stop a timer. */
};

/**
 * @brief Initialize metal timer
 *
 * @param[in] timer pointer to metal timer
 * @param[in] expire_ns nano seconds on when timer expires
 * @param[in] hd_func timer event handler function
 * @param[in] timer_priv private data
 * @param[in] start_timer user defined function to start a timer
 * @param[in] stop_timer user defined function to stop a timer
 * @return    pointer to the created metal timer for success,
 *            NULL for failure
 */
struct metal_timer *metal_timer_init(struct metal_timer *timer,
				     unsigned long long expire_ns,
				     void *priv,
				     start_timer_func start_timer,
				     stop_timer_func stop_timer);

/**
 * @brief Deinitialize metal timer
 *
 * @param[in] timer pointer to metal timer
 */
struct metal_timer *metal_timer_deinit(struct metal_timer *timer);

/**
 * @brief Register metal timer event handler
 *
 * @param[in] timer pointer to metal timer
 * @param[in] hd_func timer handler function
 * @param[in] priv private data
 *
 * return 0 for success, negative value for failure
 */
int metal_timer_register(struct metal_timer *timer,
			 metal_event_hd_func hd_func, void *priv);

/**
 * @brief Unregister metal timer handler
 *
 * @param[in] timer pointer to metal timer
 * @param[in] hd_func timer handler function
 * @param[in] priv private data
 */
void metal_timer_unregister(struct metal_timer *timer,
			 metal_event_hd_func hd_func, void *priv);

/**
 * @brief Start metal timer
 *
 * @param[in] pointer to metal timer
 * @return    0 for success, negative value for failure
 */
int metal_timer_start(struct metal_timer *timer);

/**
 * @brief Stop metal timer
 *
 * @param[in] pointer to metal timer
 */
void metal_timer_stop(struct metal_timer *timer);

/**
 * @brief metal timer expries
 *
 * The registered timer handlers will be called in
 * this function.
 *
 * @param[in] pointer to metal timer
 */
void metal_timer_expire(struct metal_timer *timer);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_TIMER__H__ */
