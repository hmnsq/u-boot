// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#অন্তর্ভাব <event.h>
#অন্তর্ভাব <event_internal.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <linker_lists.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <linux/errno.h>
#অন্তর্ভাব <linux/list.h>
#অন্তর্ভাব <relocate.h>

DECLARE_GLOBAL_তথ্য_PTR;

#যদি CONFIG_IS_ENABLED(EVENT_DEBUG)
ধ্রুবক অক্ষর *ধ্রুবক type_name[] = {
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

_Static_assert(ARRAY_আকার(type_name) == EVT_গুনতি, "event type_name size");
#endif

ধ্রুবক অক্ষর *event_type_name(গণনা event_t type)
{
#যদি CONFIG_IS_ENABLED(EVENT_DEBUG)
	যদি (type < ARRAY_আকার(type_name))
		ফেরত type_name[type];
	অন্যথা
		ফেরত "(unknown)";
#অন্যথা
	ফেরত "(unknown)";
#endif
}

স্থির পূর্ণসংখ্যা notify_static(কাঠামো event *ev)
{
	কাঠামো evspy_info *start =
		ll_entry_start(কাঠামো evspy_info, evspy_info);
	ধ্রুবক পূর্ণসংখ্যা n_ents = ll_entry_গুনতি(কাঠামো evspy_info, evspy_info);
	কাঠামো evspy_info *spy;

	জন্য (spy = start; spy != start + n_ents; spy++) {
		যদি (spy->type == ev->type) {
			পূর্ণসংখ্যা ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			যদি (spy->flags & EVSPYF_SIMPLE) {
				ধ্রুবক কাঠামো evspy_info_simple *simple;

				simple = (কাঠামো evspy_info_simple *)spy;
				ret = simple->func();
			} অন্যথা {
				ret = spy->func(ফাঁকা, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			যদি (ret)
				ফেরত log_বার্তা_ret("spy", ret);
		}
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা notify_dynamic(কাঠামো event *ev)
{
	কাঠামো event_state *state = gd_event_state();
	কাঠামো event_spy *spy, *পরবর্তী;

	list_for_each_entry_safe(spy, পরবর্তী, &state->spy_শীর্ষ, sibling_গ্রন্থি) {
		যদি (spy->type == ev->type) {
			পূর্ণসংখ্যা ret;

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
				ফেরত log_বার্তা_ret("spy", ret);
		}
	}

	ফেরত 0;
}

পূর্ণসংখ্যা event_notify(গণনা event_t type, শূন্য *তথ্য, পূর্ণসংখ্যা আকার_2)
{
	কাঠামো event event;
	পূর্ণসংখ্যা ret;

	event.type = type;
	যদি (আকার_2 > আকার(event.তথ্য))
		ফেরত log_বার্তা_ret("size", -E2BIG);
	memcpy(&event.তথ্য, তথ্য, আকার_2);

	ret = notify_static(&event);
	যদি (ret)
		ফেরত log_বার্তা_ret("sta", ret);

	যদি (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		যদি (ret)
			ফেরত log_বার্তা_ret("dyn", ret);
	}

	ফেরত 0;
}

পূর্ণসংখ্যা event_notify_null(গণনা event_t type)
{
	ফেরত event_notify(type, ফাঁকা, 0);
}

শূন্য event_show_spy_list(শূন্য)
{
	কাঠামো evspy_info *start =
		ll_entry_start(কাঠামো evspy_info, evspy_info);
	ধ্রুবক পূর্ণসংখ্যা n_ents = ll_entry_গুনতি(কাঠামো evspy_info, evspy_info);
	কাঠামো evspy_info *spy;
	ধ্রুবক পূর্ণসংখ্যা আকার_2 = আকার(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", আকার_2, "Function", "ID");
	জন্য (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), আকার_2, spy->func,
		       event_spy_id(spy));
	}
}

#যদি CONFIG_IS_ENABLED(EVENT_DYNAMIC)
স্থির শূন্য spy_free(কাঠামো event_spy *spy)
{
	list_del(&spy->sibling_গ্রন্থি);
}

পূর্ণসংখ্যা event_register(ধ্রুবক অক্ষর *id, গণনা event_t type, event_handler_t func, শূন্য *ctx)
{
	কাঠামো event_state *state = gd_event_state();
	কাঠামো event_spy *spy;

	spy = malloc(আকার(*spy));
	যদি (!spy)
		ফেরত log_বার্তা_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_লেজ(&spy->sibling_গ্রন্থি, &state->spy_শীর্ষ);

	ফেরত 0;
}

পূর্ণসংখ্যা event_uninit(শূন্য)
{
	কাঠামো event_state *state = gd_event_state();
	কাঠামো event_spy *spy, *পরবর্তী;

	list_for_each_entry_safe(spy, পরবর্তী, &state->spy_শীর্ষ, sibling_গ্রন্থি)
		spy_free(spy);

	ফেরত 0;
}

পূর্ণসংখ্যা event_init(শূন্য)
{
	কাঠামো event_state *state = gd_event_state();

	INIT_LIST_শীর্ষ(&state->spy_শীর্ষ);

	ফেরত 0;
}
#endif /* EVENT_DYNAMIC */
