/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

#include <metal/compiler.h>
#include <metal/config.h>
#include <metal/list.h>
#include <metal/utilities.h>

/** Event any ID */
#define METAL_EVENT_ANY_ID     ((unsigned int)-1)

/** Event handled status */
#define METAL_EVENT_NOT_HANDLED 0x0UL
#define METAL_EVENT_HANDLED     0x1UL

/** Maximum number of events */
#define METAL_MAX_EVENTS        CONFIG_METAL_MAX_EVENTS

/** Maximum number of tasks */
#define METAL_MAX_EVENT_TASKS   CONFIG_METAL_MAX_EVENT_TASKS

#define METAL_EVENTS_ARRAY_SIZE (METAL_MAX_EVENTS/METAL_BITS_PER_ULONG)
#define METAL_TASKS_ARRAY_SIZE (METAL_MAX_EVENT_TASKS/METAL_BITS_PER_ULONG)

struct metal_event;
struct metal_event_task;

/**
 * @brief	type of metal event handler
 * @param[in]   task pointer to the event task
 * @param[in]	arg argument to event handler
 * @return      event handled status
 */
typedef int (*metal_event_hd_func) (struct metal_event_task *task, void *arg);

/** Libmetal event handler structure */
struct metal_event_task {
	metal_event_hd_func func; /**< pointer to event handler funcion */
	unsigned int id; /**< metal event task id */
	unsigned int priority; /**< event priority. Reserved. It can be used
				    when posting the event. The default event
				    posting implementation from libmetal will
				    not consider the priority. But the user
				    can overwrite the event posting function to
				    make use of the priority property. */
	unsigned long events[METAL_EVENTS_ARRAY_SIZE]; /**< events which has
						             triggered task */
	struct metal_list node; /**< task node */
	void *arg; /**< argement of the handler function */
};

/** Libmetal event structure */
struct metal_event {
	unsigned int id; /**< metal event id */
	unsigned long tasks[METAL_TASKS_ARRAY_SIZE]; /**< tasks which will be
						          triggered when event
							  has occured */
};

/**
 * @brief     initialize event
 *
 * @param[in] event pointer to the event
 * @return    0 for success, negative value for failure
 */
int metal_event_init(struct metal_event *event);

/**
 * @brief     uninitialize event
 *
 * @param[in] event pointer to the event
 */
void metal_event_uninit(struct metal_event *event);

/**
 * @brief     event set
 *
 * To indicate an event has occurs.
 *
 * @param[in] event pointer to the event
 *
 * @return    0 if event is successfully posted, otherwise, negative
 *            value for failure.
 */
int metal_event_set(struct metal_event *event);

/**
 * @brief     event clear
 *
 * Clear the event
 *
 * @param[in] event pointer to the event
 *
 * @return    0 if event is successfully posted, otherwise, negative
 *            value for failure.
 */
int metal_event_clear(struct metal_event *event);

/**
 * @brief         enable event
 *
 * enable event handling to enable posting the event if the event
 * has happened.
 * If the event is pending, post it.
 *
 * @param[in]     event pointer to the event
 */
void metal_event_enable(struct metal_event *event);

/**
 * @brief         disable event
 *
 * disable event from posting it.
 *
 * @param[in]     event pointer to the event
 */
void metal_event_disable(struct metal_event *event);

/**
 * @brief     initialize event task
 *
 * @param[in] task pointer to the task
 * @param[in] func event handler function
 * @param[in] arg event handler argument
 * @return    0 for success, negative value for failure
 */
int metal_event_task_init(struct metal_event_task *task,
			  metal_event_hd_func func,
			  void *arg);

/**
 * @brief     uninitialize event task
 *
 * @param[in] task pointer to the task
 */
void metal_event_task_uninit(struct metal_event_task *task);

/**
 * @brief      Register event handler
 *
 * @param[in]  event pointer to the event
 * @param[in]  func pointer to the event handler
 * @return     0 for success, negative value for failure
 */
int metal_event_register_handler(struct metal_event_task *task,
				 struct metal_event *event);

/**
 * @brief      Unregister event handler
 *
 * @param[in]  event pointer to the event
 * @param[in]  func pointer to the event handler
 */
void metal_event_unregister_handler(struct metal_event_task *task,
				    struct metal_event *event);

/**
 * @brief        event dispatcher
 *
 * This function will check which events have occured.
 * If yes, the tasks registered for the events will
 * be called.
 */
void metal_event_task_dispatch(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_EVENT__H__ */
