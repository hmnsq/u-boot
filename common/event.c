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

#အကယ်၍ CONFIG_IS_ENABLED(EVENT_DEBUG)
const var_na *const type_name[] = {
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

const var_na *event_type_name(ga_nanaa event_t type)
{
#အကယ်၍ CONFIG_IS_ENABLED(EVENT_DEBUG)
	အကယ်၍ (type < ARRAY_SIZE(type_name))
		ပြန်ပေး type_name[type];
	မဟုတ်ပါက
		ပြန်ပေး "(unknown)";
#မဟုတ်ပါက
	ပြန်ပေး "(unknown)";
#endif
}

static ကိန်းပြည့် notify_static(sa_nracanaa event *ev)
{
	sa_nracanaa evspy_info *start =
		ll_entry_start(sa_nracanaa evspy_info, evspy_info);
	const ကိန်းပြည့် n_ents = ll_entry_count(sa_nracanaa evspy_info, evspy_info);
	sa_nracanaa evspy_info *spy;

	အတွက် (spy = start; spy != start + n_ents; spy++) {
		အကယ်၍ (spy->type == ev->type) {
			ကိန်းပြည့် ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			အကယ်၍ (spy->flags & EVSPYF_SIMPLE) {
				const sa_nracanaa evspy_info_simple *simple;

				simple = (sa_nracanaa evspy_info_simple *)spy;
				ret = simple->func();
			} မဟုတ်ပါက {
				ret = spy->func(NULL, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			အကယ်၍ (ret)
				ပြန်ပေး log_msg_ret("spy", ret);
		}
	}

	ပြန်ပေး 0;
}

static ကိန်းပြည့် notify_dynamic(sa_nracanaa event *ev)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node) {
		အကယ်၍ (spy->type == ev->type) {
			ကိန်းပြည့် ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			အကယ်၍ (ret)
				ပြန်ပေး log_msg_ret("spy", ret);
		}
	}

	ပြန်ပေး 0;
}

ကိန်းပြည့် event_notify(ga_nanaa event_t type, void *data, ကိန်းပြည့် size)
{
	sa_nracanaa event event;
	ကိန်းပြည့် ret;

	event.type = type;
	အကယ်၍ (size > parimaa_na(event.data))
		ပြန်ပေး log_msg_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	အကယ်၍ (ret)
		ပြန်ပေး log_msg_ret("sta", ret);

	အကယ်၍ (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		အကယ်၍ (ret)
			ပြန်ပေး log_msg_ret("dyn", ret);
	}

	ပြန်ပေး 0;
}

ကိန်းပြည့် event_notify_null(ga_nanaa event_t type)
{
	ပြန်ပေး event_notify(type, NULL, 0);
}

void event_show_spy_list(void)
{
	sa_nracanaa evspy_info *start =
		ll_entry_start(sa_nracanaa evspy_info, evspy_info);
	const ကိန်းပြည့် n_ents = ll_entry_count(sa_nracanaa evspy_info, evspy_info);
	sa_nracanaa evspy_info *spy;
	const ကိန်းပြည့် size = parimaa_na(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	အတွက် (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#အကယ်၍ CONFIG_IS_ENABLED(EVENT_DYNAMIC)
static void spy_free(sa_nracanaa event_spy *spy)
{
	list_del(&spy->sibling_node);
}

ကိန်းပြည့် event_register(const var_na *id, ga_nanaa event_t type, event_handler_t func, void *ctx)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy;

	spy = malloc(parimaa_na(*spy));
	အကယ်၍ (!spy)
		ပြန်ပေး log_msg_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_node, &state->spy_head);

	ပြန်ပေး 0;
}

ကိန်းပြည့် event_uninit(void)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node)
		spy_free(spy);

	ပြန်ပေး 0;
}

ကိန်းပြည့် event_init(void)
{
	sa_nracanaa event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	ပြန်ပေး 0;
}
#endif /* EVENT_DYNAMIC */
