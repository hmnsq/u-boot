// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#شامل <event.h>
#شامل <event_internal.h>
#شامل <log.h>
#شامل <linker_lists.h>
#شامل <malloc.h>
#شامل <asm/global_data.h>
#شامل <linux/errno.h>
#شامل <linux/list.h>
#شامل <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#اگر CONFIG_IS_ENABLED(EVENT_DEBUG)
ثابت نویسه *ثابت type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_شمار, "event type_name size");
#endif

ثابت نویسه *event_type_name(شمارش event_t type)
{
#اگر CONFIG_IS_ENABLED(EVENT_DEBUG)
	اگر (type < ARRAY_SIZE(type_name))
		بازگشت type_name[type];
	وگرنه
		بازگشت "(unknown)";
#وگرنه
	بازگشت "(unknown)";
#endif
}

ایستا صحیح notify_static(ساختار event *ev)
{
	ساختار evspy_info *start =
		ll_entry_start(ساختار evspy_info, evspy_info);
	ثابت صحیح n_ents = ll_entry_شمار(ساختار evspy_info, evspy_info);
	ساختار evspy_info *spy;

	برای (spy = start; spy != start + n_ents; spy++) {
		اگر (spy->type == ev->type) {
			صحیح ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			اگر (spy->flags & EVSPYF_SIMPLE) {
				ثابت ساختار evspy_info_simple *simple;

				simple = (ساختار evspy_info_simple *)spy;
				ret = simple->func();
			} وگرنه {
				ret = spy->func(پوچ, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			اگر (ret)
				بازگشت log_پیام_ret("spy", ret);
		}
	}

	بازگشت 0;
}

ایستا صحیح notify_dynamic(ساختار event *ev)
{
	ساختار event_state *state = gd_event_state();
	ساختار event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_گره) {
		اگر (spy->type == ev->type) {
			صحیح ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			اگر (ret)
				بازگشت log_پیام_ret("spy", ret);
		}
	}

	بازگشت 0;
}

صحیح event_notify(شمارش event_t type, تهی *data, صحیح size)
{
	ساختار event event;
	صحیح ret;

	event.type = type;
	اگر (size > اندازه(event.data))
		بازگشت log_پیام_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	اگر (ret)
		بازگشت log_پیام_ret("sta", ret);

	اگر (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		اگر (ret)
			بازگشت log_پیام_ret("dyn", ret);
	}

	بازگشت 0;
}

صحیح event_notify_null(شمارش event_t type)
{
	بازگشت event_notify(type, پوچ, 0);
}

تهی event_show_spy_list(تهی)
{
	ساختار evspy_info *start =
		ll_entry_start(ساختار evspy_info, evspy_info);
	ثابت صحیح n_ents = ll_entry_شمار(ساختار evspy_info, evspy_info);
	ساختار evspy_info *spy;
	ثابت صحیح size = اندازه(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	برای (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#اگر CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ایستا تهی spy_free(ساختار event_spy *spy)
{
	list_del(&spy->sibling_گره);
}

صحیح event_register(ثابت نویسه *id, شمارش event_t type, event_handler_t func, تهی *ctx)
{
	ساختار event_state *state = gd_event_state();
	ساختار event_spy *spy;

	spy = malloc(اندازه(*spy));
	اگر (!spy)
		بازگشت log_پیام_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_گره, &state->spy_head);

	بازگشت 0;
}

صحیح event_uninit(تهی)
{
	ساختار event_state *state = gd_event_state();
	ساختار event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_گره)
		spy_free(spy);

	بازگشت 0;
}

صحیح event_init(تهی)
{
	ساختار event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	بازگشت 0;
}
#endif /* EVENT_DYNAMIC */
