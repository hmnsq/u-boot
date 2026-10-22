// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#অন্তর্ভাৱ <event.h>
#অন্তর্ভাৱ <event_internal.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <linker_lists.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <linux/errno.h>
#অন্তর্ভাৱ <linux/list.h>
#অন্তর্ভাৱ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#যদি CONFIG_IS_ENABLED(EVENT_DEBUG)
ধ্রুৱ ৱর্ণ *ধ্রুৱ type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_গুনতি, "event type_name size");
#endif

ধ্রুৱ ৱর্ণ *event_type_name(গণনা event_t type)
{
#যদি CONFIG_IS_ENABLED(EVENT_DEBUG)
	যদি (type < ARRAY_SIZE(type_name))
		ঘূৰাই type_name[type];
	অন্যথা
		ঘূৰাই "(unknown)";
#অন্যথা
	ঘূৰাই "(unknown)";
#endif
}

স্থির পূর্ণাঙ্ক notify_static(সংরচনা event *ev)
{
	সংরচনা evspy_info *start =
		ll_entry_start(সংরচনা evspy_info, evspy_info);
	ধ্রুৱ পূর্ণাঙ্ক n_ents = ll_entry_গুনতি(সংরচনা evspy_info, evspy_info);
	সংরচনা evspy_info *spy;

	বাবে (spy = start; spy != start + n_ents; spy++) {
		যদি (spy->type == ev->type) {
			পূর্ণাঙ্ক ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			যদি (spy->flags & EVSPYF_SIMPLE) {
				ধ্রুৱ সংরচনা evspy_info_simple *simple;

				simple = (সংরচনা evspy_info_simple *)spy;
				ret = simple->func();
			} অন্যথা {
				ret = spy->func(রিক্ত, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			যদি (ret)
				ঘূৰাই log_বাৰ্তা_ret("spy", ret);
		}
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক notify_dynamic(সংরচনা event *ev)
{
	সংরচনা event_state *state = gd_event_state();
	সংরচনা event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_গ্ৰন্থি) {
		যদি (spy->type == ev->type) {
			পূর্ণাঙ্ক ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			যদি (ret)
				ঘূৰাই log_বাৰ্তা_ret("spy", ret);
		}
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক event_notify(গণনা event_t type, শূন্য *data, পূর্ণাঙ্ক size)
{
	সংরচনা event event;
	পূর্ণাঙ্ক ret;

	event.type = type;
	যদি (size > পরিমাণ(event.data))
		ঘূৰাই log_বাৰ্তা_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	যদি (ret)
		ঘূৰাই log_বাৰ্তা_ret("sta", ret);

	যদি (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		যদি (ret)
			ঘূৰাই log_বাৰ্তা_ret("dyn", ret);
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক event_notify_null(গণনা event_t type)
{
	ঘূৰাই event_notify(type, রিক্ত, 0);
}

শূন্য event_show_spy_list(শূন্য)
{
	সংরচনা evspy_info *start =
		ll_entry_start(সংরচনা evspy_info, evspy_info);
	ধ্রুৱ পূর্ণাঙ্ক n_ents = ll_entry_গুনতি(সংরচনা evspy_info, evspy_info);
	সংরচনা evspy_info *spy;
	ধ্রুৱ পূর্ণাঙ্ক size = পরিমাণ(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	বাবে (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#যদি CONFIG_IS_ENABLED(EVENT_DYNAMIC)
স্থির শূন্য spy_free(সংরচনা event_spy *spy)
{
	list_del(&spy->sibling_গ্ৰন্থি);
}

পূর্ণাঙ্ক event_register(ধ্রুৱ ৱর্ণ *id, গণনা event_t type, event_handler_t func, শূন্য *ctx)
{
	সংরচনা event_state *state = gd_event_state();
	সংরচনা event_spy *spy;

	spy = malloc(পরিমাণ(*spy));
	যদি (!spy)
		ঘূৰাই log_বাৰ্তা_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_গ্ৰন্থি, &state->spy_head);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক event_uninit(শূন্য)
{
	সংরচনা event_state *state = gd_event_state();
	সংরচনা event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_গ্ৰন্থি)
		spy_free(spy);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক event_init(শূন্য)
{
	সংরচনা event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	ঘূৰাই 0;
}
#endif /* EVENT_DYNAMIC */
