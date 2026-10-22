// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#include <event.h>
#include <event_internal.h>
#include <log.h>
#include <linker_lists.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#ако CONFIG_IS_ENABLED(EVENT_DEBUG)
const char *const type_name[] = {
	"none",
	"test",

	/* Events related to driver model */
	"dm_post_init_f",
	"dm_post_init_r",
	"dm_pre_probe",
	"dm_post_probe",
	"dm_pre_remove",
	"dm_post_remove",

	/* init hooks */
	"misc_init_f",
	"fsp_init_r",
	"settings_r",
	"last_stage_init",

	/* Fpga load hook */
	"fpga_load",

	/* fdt hooks */
	"ft_fixup",

	/* main loop events */
	"main_loop",

	/* post preboot events */
	"post_preboot",

	/* livetree has been built */
	"of_live_init",
};

_Static_assert(ARRAY_SIZE(type_name) == EVT_COUNT, "event type_name size");
#endif

const char *event_type_name(enum event_t type)
{
#ако CONFIG_IS_ENABLED(EVENT_DEBUG)
	ако (type < ARRAY_SIZE(type_name))
		върни type_name[type];
	иначе
		върни "(unknown)";
#иначе
	върни "(unknown)";
#endif
}

static цяло notify_static(struct event *ev)
{
	struct evspy_info *start =
		ll_entry_start(struct evspy_info, evspy_info);
	const цяло n_ents = ll_entry_count(struct evspy_info, evspy_info);
	struct evspy_info *spy;

	за (spy = start; spy != start + n_ents; spy++) {
		ако (spy->type == ev->type) {
			цяло ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			ако (spy->flags & EVSPYF_SIMPLE) {
				const struct evspy_info_simple *simple;

				simple = (struct evspy_info_simple *)spy;
				ret = simple->func();
			} иначе {
				ret = spy->func(NULL, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ако (ret)
				върни log_msg_ret("spy", ret);
		}
	}

	върни 0;
}

static цяло notify_dynamic(struct event *ev)
{
	struct event_state *state = gd_event_state();
	struct event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node) {
		ако (spy->type == ev->type) {
			цяло ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ако (ret)
				върни log_msg_ret("spy", ret);
		}
	}

	върни 0;
}

цяло event_notify(enum event_t type, void *data, цяло size)
{
	struct event event;
	цяло ret;

	event.type = type;
	ако (size > sizeof(event.data))
		върни log_msg_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	ако (ret)
		върни log_msg_ret("sta", ret);

	ако (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		ако (ret)
			върни log_msg_ret("dyn", ret);
	}

	върни 0;
}

цяло event_notify_null(enum event_t type)
{
	върни event_notify(type, NULL, 0);
}

void event_show_spy_list(void)
{
	struct evspy_info *start =
		ll_entry_start(struct evspy_info, evspy_info);
	const цяло n_ents = ll_entry_count(struct evspy_info, evspy_info);
	struct evspy_info *spy;
	const цяло size = sizeof(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	за (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#ако CONFIG_IS_ENABLED(EVENT_DYNAMIC)
static void spy_free(struct event_spy *spy)
{
	list_del(&spy->sibling_node);
}

цяло event_register(const char *id, enum event_t type, event_handler_t func, void *ctx)
{
	struct event_state *state = gd_event_state();
	struct event_spy *spy;

	spy = malloc(sizeof(*spy));
	ако (!spy)
		върни log_msg_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_node, &state->spy_head);

	върни 0;
}

цяло event_uninit(void)
{
	struct event_state *state = gd_event_state();
	struct event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node)
		spy_free(spy);

	върни 0;
}

цяло event_init(void)
{
	struct event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	върни 0;
}
#endif /* EVENT_DYNAMIC */
