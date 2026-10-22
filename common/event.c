// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#ضمن <event.h>
#ضمن <event_internal.h>
#ضمن <log.h>
#ضمن <linker_lists.h>
#ضمن <malloc.h>
#ضمن <asm/global_data.h>
#ضمن <linux/errno.h>
#ضمن <linux/list.h>
#ضمن <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#אם CONFIG_IS_ENABLED(EVENT_DEBUG)
ثابت حرف *ثابت type_name[] = {
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

ثابت حرف *event_type_name(تعداد event_t type)
{
#אם CONFIG_IS_ENABLED(EVENT_DEBUG)
	אם (type < ARRAY_SIZE(type_name))
		החזר type_name[type];
	אחרת
		החזר "(unknown)";
#אחרת
	החזר "(unknown)";
#endif
}

ساكن صحيح notify_static(بنية event *ev)
{
	بنية evspy_info *start =
		ll_entry_start(بنية evspy_info, evspy_info);
	ثابت صحيح n_ents = ll_entry_count(بنية evspy_info, evspy_info);
	بنية evspy_info *spy;

	עבור (spy = start; spy != start + n_ents; spy++) {
		אם (spy->type == ev->type) {
			صحيح ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			אם (spy->flags & EVSPYF_SIMPLE) {
				ثابت بنية evspy_info_simple *simple;

				simple = (بنية evspy_info_simple *)spy;
				ret = simple->func();
			} אחרת {
				ret = spy->func(عدم, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			אם (ret)
				החזר log_msg_ret("spy", ret);
		}
	}

	החזר 0;
}

ساكن صحيح notify_dynamic(بنية event *ev)
{
	بنية event_state *state = gd_event_state();
	بنية event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node) {
		אם (spy->type == ev->type) {
			صحيح ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			אם (ret)
				החזר log_msg_ret("spy", ret);
		}
	}

	החזר 0;
}

صحيح event_notify(تعداد event_t type, فراغ *data, صحيح size)
{
	بنية event event;
	صحيح ret;

	event.type = type;
	אם (size > حجم(event.data))
		החזר log_msg_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	אם (ret)
		החזר log_msg_ret("sta", ret);

	אם (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		אם (ret)
			החזר log_msg_ret("dyn", ret);
	}

	החזר 0;
}

صحيح event_notify_null(تعداد event_t type)
{
	החזר event_notify(type, عدم, 0);
}

فراغ event_show_spy_list(فراغ)
{
	بنية evspy_info *start =
		ll_entry_start(بنية evspy_info, evspy_info);
	ثابت صحيح n_ents = ll_entry_count(بنية evspy_info, evspy_info);
	بنية evspy_info *spy;
	ثابت صحيح size = حجم(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	עבור (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#אם CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ساكن فراغ spy_free(بنية event_spy *spy)
{
	list_del(&spy->sibling_node);
}

صحيح event_register(ثابت حرف *id, تعداد event_t type, event_handler_t func, فراغ *ctx)
{
	بنية event_state *state = gd_event_state();
	بنية event_spy *spy;

	spy = malloc(حجم(*spy));
	אם (!spy)
		החזר log_msg_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_node, &state->spy_head);

	החזר 0;
}

صحيح event_uninit(فراغ)
{
	بنية event_state *state = gd_event_state();
	بنية event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node)
		spy_free(spy);

	החזר 0;
}

صحيح event_init(فراغ)
{
	بنية event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	החזר 0;
}
#endif /* EVENT_DYNAMIC */
