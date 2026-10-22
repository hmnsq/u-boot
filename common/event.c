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

#නම් CONFIG_IS_ENABLED(EVENT_DEBUG)
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
#නම් CONFIG_IS_ENABLED(EVENT_DEBUG)
	නම් (type < ARRAY_SIZE(type_name))
		ආපසු type_name[type];
	නැතහොත්
		ආපසු "(unknown)";
#නැතහොත්
	ආපසු "(unknown)";
#endif
}

static පූර්ණ_සංඛ්‍යා notify_static(sa_nracanaa event *ev)
{
	sa_nracanaa evspy_info *start =
		ll_entry_start(sa_nracanaa evspy_info, evspy_info);
	const පූර්ණ_සංඛ්‍යා n_ents = ll_entry_count(sa_nracanaa evspy_info, evspy_info);
	sa_nracanaa evspy_info *spy;

	සඳහා (spy = start; spy != start + n_ents; spy++) {
		නම් (spy->type == ev->type) {
			පූර්ණ_සංඛ්‍යා ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			නම් (spy->flags & EVSPYF_SIMPLE) {
				const sa_nracanaa evspy_info_simple *simple;

				simple = (sa_nracanaa evspy_info_simple *)spy;
				ret = simple->func();
			} නැතහොත් {
				ret = spy->func(NULL, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			නම් (ret)
				ආපසු log_msg_ret("spy", ret);
		}
	}

	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා notify_dynamic(sa_nracanaa event *ev)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node) {
		නම් (spy->type == ev->type) {
			පූර්ණ_සංඛ්‍යා ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			නම් (ret)
				ආපසු log_msg_ret("spy", ret);
		}
	}

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා event_notify(ga_nanaa event_t type, void *data, පූර්ණ_සංඛ්‍යා size)
{
	sa_nracanaa event event;
	පූර්ණ_සංඛ්‍යා ret;

	event.type = type;
	නම් (size > parimaa_na(event.data))
		ආපසු log_msg_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	නම් (ret)
		ආපසු log_msg_ret("sta", ret);

	නම් (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		නම් (ret)
			ආපසු log_msg_ret("dyn", ret);
	}

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා event_notify_null(ga_nanaa event_t type)
{
	ආපසු event_notify(type, NULL, 0);
}

void event_show_spy_list(void)
{
	sa_nracanaa evspy_info *start =
		ll_entry_start(sa_nracanaa evspy_info, evspy_info);
	const පූර්ණ_සංඛ්‍යා n_ents = ll_entry_count(sa_nracanaa evspy_info, evspy_info);
	sa_nracanaa evspy_info *spy;
	const පූර්ණ_සංඛ්‍යා size = parimaa_na(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	සඳහා (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#නම් CONFIG_IS_ENABLED(EVENT_DYNAMIC)
static void spy_free(sa_nracanaa event_spy *spy)
{
	list_del(&spy->sibling_node);
}

පූර්ණ_සංඛ්‍යා event_register(const var_na *id, ga_nanaa event_t type, event_handler_t func, void *ctx)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy;

	spy = malloc(parimaa_na(*spy));
	නම් (!spy)
		ආපසු log_msg_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_node, &state->spy_head);

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා event_uninit(void)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node)
		spy_free(spy);

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා event_init(void)
{
	sa_nracanaa event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	ආපසු 0;
}
#endif /* EVENT_DYNAMIC */
