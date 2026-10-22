// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#அந்தர்பாவ <event.h>
#அந்தர்பாவ <event_internal.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <linker_lists.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <linux/errno.h>
#அந்தர்பாவ <linux/list.h>
#அந்தர்பாவ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#எனில் CONFIG_IS_ENABLED(EVENT_DEBUG)
மாறிலி எழுத்து *மாறிலி type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_மொத்தம், "event type_name size");
#endif

மாறிலி எழுத்து *event_type_name(எண்ணிக்கை event_t type)
{
#எனில் CONFIG_IS_ENABLED(EVENT_DEBUG)
	எனில் (type < ARRAY_SIZE(type_name))
		திரும்பு type_name[type];
	இல்லையெனில்
		திரும்பு "(unknown)";
#இல்லையெனில்
	திரும்பு "(unknown)";
#endif
}

நிலையான முழு notify_static(கட்டமைப்பு event *ev)
{
	கட்டமைப்பு evspy_info *start =
		ll_entry_start(கட்டமைப்பு evspy_info, evspy_info);
	மாறிலி முழு n_ents = ll_entry_மொத்தம்(கட்டமைப்பு evspy_info, evspy_info);
	கட்டமைப்பு evspy_info *spy;

	க்கு (spy = start; spy != start + n_ents; spy++) {
		எனில் (spy->type == ev->type) {
			முழு ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			எனில் (spy->flags & EVSPYF_SIMPLE) {
				மாறிலி கட்டமைப்பு evspy_info_simple *simple;

				simple = (கட்டமைப்பு evspy_info_simple *)spy;
				ret = simple->func();
			} இல்லையெனில் {
				ret = spy->func(வெற்று, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			எனில் (ret)
				திரும்பு log_செய்தி_ret("spy", ret);
		}
	}

	திரும்பு 0;
}

நிலையான முழு notify_dynamic(கட்டமைப்பு event *ev)
{
	கட்டமைப்பு event_state *state = gd_event_state();
	கட்டமைப்பு event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_முனை) {
		எனில் (spy->type == ev->type) {
			முழு ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			எனில் (ret)
				திரும்பு log_செய்தி_ret("spy", ret);
		}
	}

	திரும்பு 0;
}

முழு event_notify(எண்ணிக்கை event_t type, வெறுமை *data, முழு size)
{
	கட்டமைப்பு event event;
	முழு ret;

	event.type = type;
	எனில் (size > அளவு(event.data))
		திரும்பு log_செய்தி_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	எனில் (ret)
		திரும்பு log_செய்தி_ret("sta", ret);

	எனில் (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		எனில் (ret)
			திரும்பு log_செய்தி_ret("dyn", ret);
	}

	திரும்பு 0;
}

முழு event_notify_null(எண்ணிக்கை event_t type)
{
	திரும்பு event_notify(type, வெற்று, 0);
}

வெறுமை event_show_spy_list(வெறுமை)
{
	கட்டமைப்பு evspy_info *start =
		ll_entry_start(கட்டமைப்பு evspy_info, evspy_info);
	மாறிலி முழு n_ents = ll_entry_மொத்தம்(கட்டமைப்பு evspy_info, evspy_info);
	கட்டமைப்பு evspy_info *spy;
	மாறிலி முழு size = அளவு(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	க்கு (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#எனில் CONFIG_IS_ENABLED(EVENT_DYNAMIC)
நிலையான வெறுமை spy_free(கட்டமைப்பு event_spy *spy)
{
	list_del(&spy->sibling_முனை);
}

முழு event_register(மாறிலி எழுத்து *id, எண்ணிக்கை event_t type, event_handler_t func, வெறுமை *ctx)
{
	கட்டமைப்பு event_state *state = gd_event_state();
	கட்டமைப்பு event_spy *spy;

	spy = malloc(அளவு(*spy));
	எனில் (!spy)
		திரும்பு log_செய்தி_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_முனை, &state->spy_head);

	திரும்பு 0;
}

முழு event_uninit(வெறுமை)
{
	கட்டமைப்பு event_state *state = gd_event_state();
	கட்டமைப்பு event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_முனை)
		spy_free(spy);

	திரும்பு 0;
}

முழு event_init(வெறுமை)
{
	கட்டமைப்பு event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	திரும்பு 0;
}
#endif /* EVENT_DYNAMIC */
