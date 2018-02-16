/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <metal/event.h>
#include <metal/irq.h>
#include <metal/list.h>
#include <metal/log.h>
#include <metal/sys.h>
#include <string.h>


static struct metal_event *_events_table[METAL_MAX_EVENTS];
static struct metal_event_task *_tasks_table[METAL_MAX_EVENT_TASKS];

struct metal_event_stat {
	unsigned long reg_bitmap[METAL_EVENTS_ARRAY_SIZE];
	unsigned long enable_bitmap[METAL_EVENTS_ARRAY_SIZE];
	unsigned long pending_bitmap[METAL_EVENTS_ARRAY_SIZE];
};

static struct metal_event_stat _estat;
static unsigned long metal_tasks_bitmap[METAL_TASKS_ARRAY_SIZE];

int metal_event_init(struct metal_event *event)
{
	unsigned int flags;
	unsigned int nbit;

	if (event == NULL)
		return -EINVAL;

	event->id = METAL_EVENT_ANY_ID;
	memset(event->tasks, 0,  sizeof(event->tasks));

	flags = metal_irq_save_disable();
	nbit = metal_bitmap_next_clear_bit(_estat.reg_bitmap, 0,
					 METAL_MAX_EVENTS);
	if (nbit == METAL_MAX_EVENTS) {
		metal_irq_restore_enable(flags);
		return -EAGAIN;
	}
	metal_bitmap_set_bit(_estat.reg_bitmap, nbit);
	_events_table[nbit] = event;
	metal_irq_restore_enable(flags);
	event->id = nbit;
	return 0;
}

void metal_event_uninit(struct metal_event *event)
{
	unsigned int flags;
	unsigned int eid;

	if (event == NULL)
		return;
	eid = event->id;
	if (eid == METAL_EVENT_ANY_ID)
		return;
	flags = metal_irq_save_disable();
	metal_bitmap_clear_bit(_estat.reg_bitmap, eid);
	metal_bitmap_clear_bit(_estat.enable_bitmap, eid);
	metal_bitmap_clear_bit(_estat.pending_bitmap, eid);
	_events_table[eid] = NULL;
	metal_irq_restore_enable(flags);

	event->id = METAL_EVENT_ANY_ID;
}

int metal_event_set(struct metal_event *event)
{
	unsigned int eid;
	unsigned int flags;

	if (event == NULL)
		return -EINVAL;
	eid = event->id;
	if (event->id == METAL_EVENT_ANY_ID)
		return  -EINVAL;
	flags = metal_irq_save_disable();
	metal_bitmap_set_bit(_estat.pending_bitmap, eid);
	metal_irq_restore_enable(flags);
	return 0;
}

int metal_event_clear(struct metal_event *event)
{
	unsigned int eid;
	unsigned int flags;

	if (event == NULL)
		return -EINVAL;
	eid = event->id;
	if (event->id == METAL_EVENT_ANY_ID)
		return  -EINVAL;
	flags = metal_irq_save_disable();
	metal_bitmap_clear_bit(_estat.pending_bitmap, eid);
	metal_irq_restore_enable(flags);
	return 0;
}

void metal_event_enable(struct metal_event *event)
{
	unsigned int eid;
	unsigned int flags;

	if (event == NULL)
		return;
	eid = event->id;
	if (event->id == METAL_EVENT_ANY_ID)
		return;
	flags = metal_irq_save_disable();
	metal_bitmap_set_bit(_estat.enable_bitmap, eid);
	metal_irq_restore_enable(flags);
}

void metal_event_disable(struct metal_event *event)
{
	unsigned int eid;
	unsigned int flags;

	if (event == NULL)
		return;
	eid = event->id;
	if (event->id == METAL_EVENT_ANY_ID)
		return;
	flags = metal_irq_save_disable();
	metal_bitmap_clear_bit(_estat.enable_bitmap, eid);
	metal_irq_restore_enable(flags);
}

int metal_event_task_init(struct metal_event_task *task,
			  metal_event_hd_func func,
			  void *arg)
{
	unsigned int flags;
	unsigned int nbit;

	if (task  == NULL)
		return -EINVAL;

	task->id = METAL_EVENT_ANY_ID;
	memset(task->events, 0,  sizeof(task->events));
	metal_list_init(&task->node);

	flags = metal_irq_save_disable();
	nbit = metal_bitmap_next_clear_bit(metal_tasks_bitmap, 0,
					 METAL_MAX_EVENT_TASKS);
	if (nbit == METAL_MAX_EVENT_TASKS) {
		metal_irq_restore_enable(flags);
		return -EAGAIN;
	}
	metal_bitmap_set_bit(metal_tasks_bitmap, nbit);
	_tasks_table[nbit] = task;
	metal_irq_restore_enable(flags);
	task->id = nbit;
	task->func = func;
	task->arg = arg;
	task->priority = 0;
	return 0;
}

void metal_event_task_uninit(struct metal_event_task *task)
{
	unsigned int flags;
	unsigned int tid;

	if (task == NULL)
		return;

	tid = task->id;
	if (tid >= METAL_MAX_EVENT_TASKS)
		return;
	flags = metal_irq_save_disable();
	metal_bitmap_clear_bit(metal_tasks_bitmap, tid);
	_tasks_table[tid] = NULL;
	metal_irq_restore_enable(flags);
	task->id = METAL_EVENT_ANY_ID;
}

int metal_event_register_handler(struct metal_event_task *task,
				 struct metal_event *event)
{
	unsigned int flags;
	unsigned int eid, tid;

	if (task == NULL || event == NULL)
		return -EINVAL;

	eid = event->id;
	tid = task->id;
	if (eid >= METAL_MAX_EVENTS || tid >= METAL_MAX_EVENT_TASKS)
		return -EINVAL;
	flags = metal_irq_save_disable();
	metal_bitmap_set_bit(task->events, eid);
	metal_bitmap_set_bit(event->tasks, tid);
	metal_irq_restore_enable(flags);
	return 0;
}

void metal_event_unregister_handler(struct metal_event_task *task,
				    struct metal_event *event)
{
	unsigned int flags;
	unsigned int eid, tid;

	if (!task && !event)
		return;

	eid = event->id;
	tid = task->id;
	if (eid >= METAL_MAX_EVENTS || tid >= METAL_MAX_EVENT_TASKS)
		return;
	flags = metal_irq_save_disable();
	metal_bitmap_clear_bit(task->events, eid);
	metal_bitmap_clear_bit(event->tasks, tid);
	metal_irq_restore_enable(flags);
}

void metal_event_task_dispatch(void)
{
	unsigned int flags;
	unsigned long tmp_ebitmap[METAL_EVENTS_ARRAY_SIZE];
	struct metal_list tasks;
	unsigned ebit, tbit;
	struct metal_list *node;
	struct metal_event_task *t;
	unsigned int i;

	metal_list_init(&tasks);
	flags = metal_irq_save_disable();
	for (i = 0; i < METAL_EVENTS_ARRAY_SIZE; i++) {
		tmp_ebitmap[i] = _estat.enable_bitmap[i] &
				 _estat.pending_bitmap[i];
	}
	metal_bitmap_for_each_set_bit(tmp_ebitmap, ebit,
				      METAL_MAX_EVENTS) {
		struct metal_event *e;

		e = _events_table[ebit];
		if (e == NULL) {
			continue;
		}
		metal_bitmap_for_each_set_bit(e->tasks, tbit,
					      METAL_MAX_EVENT_TASKS) {
			t = _tasks_table[tbit];
			if (t == NULL) {
				continue;
			}
			metal_list_add_after(&tasks, &t->node);
		}
		metal_bitmap_clear_bit(_estat.pending_bitmap, ebit);
	}
	metal_irq_restore_enable(flags);

	metal_list_for_each(&tasks, node) {
		t = metal_container_of(node, struct metal_event_task,
				       node);
		if (t->func == NULL) {
			metal_log(METAL_LOG_ERROR,
				  "Task 0x%x has no function.\n",
				  t->id);
			continue;
		}
		(void)t->func(t, t->arg);
	}
}
