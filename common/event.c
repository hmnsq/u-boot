// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#ಅನ್ತರ್ಭಾವ <event.h>
#ಅನ್ತರ್ಭಾವ <event_internal.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <linker_lists.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>
#ಅನ್ತರ್ಭಾವ <linux/list.h>
#ಅನ್ತರ್ಭಾವ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#ಯದಿ CONFIG_IS_ENABLED(EVENT_DEBUG)
ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_ಎಣಿಕೆ, "event type_name size");
#endif

ಧ್ರುವ ವರ್ಣ *event_type_name(ಗಣನಾ event_t type)
{
#ಯದಿ CONFIG_IS_ENABLED(EVENT_DEBUG)
	ಯದಿ (type < ARRAY_SIZE(type_name))
		ಪ್ರತ್ಯಾಗಮ type_name[type];
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ "(unknown)";
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ "(unknown)";
#endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ notify_static(ಸಂರಚನಾ event *ev)
{
	ಸಂರಚನಾ evspy_info *start =
		ll_entry_start(ಸಂರಚನಾ evspy_info, evspy_info);
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ n_ents = ll_entry_ಎಣಿಕೆ(ಸಂರಚನಾ evspy_info, evspy_info);
	ಸಂರಚನಾ evspy_info *spy;

	ಪ್ರತಿ (spy = start; spy != start + n_ents; spy++) {
		ಯದಿ (spy->type == ev->type) {
			ಪೂರ್ಣಾಙ್ಕ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			ಯದಿ (spy->flags & EVSPYF_SIMPLE) {
				ಧ್ರುವ ಸಂರಚನಾ evspy_info_simple *simple;

				simple = (ಸಂರಚನಾ evspy_info_simple *)spy;
				ret = simple->func();
			} ಅನ್ಯಥಾ {
				ret = spy->func(ರಿಕ್ತ, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ಯದಿ (ret)
				ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("spy", ret);
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ notify_dynamic(ಸಂರಚನಾ event *ev)
{
	ಸಂರಚನಾ event_state *state = gd_event_state();
	ಸಂರಚನಾ event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_ಗಂಟು) {
		ಯದಿ (spy->type == ev->type) {
			ಪೂರ್ಣಾಙ್ಕ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ಯದಿ (ret)
				ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("spy", ret);
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ event_notify(ಗಣನಾ event_t type, ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಸಂರಚನಾ event event;
	ಪೂರ್ಣಾಙ್ಕ ret;

	event.type = type;
	ಯದಿ (size > ಪರಿಮಾಣ(event.data))
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("sta", ret);

	ಯದಿ (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("dyn", ret);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ event_notify_null(ಗಣನಾ event_t type)
{
	ಪ್ರತ್ಯಾಗಮ event_notify(type, ರಿಕ್ತ, 0);
}

ಶೂನ್ಯ event_show_spy_list(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ evspy_info *start =
		ll_entry_start(ಸಂರಚನಾ evspy_info, evspy_info);
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ n_ents = ll_entry_ಎಣಿಕೆ(ಸಂರಚನಾ evspy_info, evspy_info);
	ಸಂರಚನಾ evspy_info *spy;
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ size = ಪರಿಮಾಣ(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	ಪ್ರತಿ (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#ಯದಿ CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ಸ್ಥಿರ ಶೂನ್ಯ spy_free(ಸಂರಚನಾ event_spy *spy)
{
	list_del(&spy->sibling_ಗಂಟು);
}

ಪೂರ್ಣಾಙ್ಕ event_register(ಧ್ರುವ ವರ್ಣ *id, ಗಣನಾ event_t type, event_handler_t func, ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ event_state *state = gd_event_state();
	ಸಂರಚನಾ event_spy *spy;

	spy = malloc(ಪರಿಮಾಣ(*spy));
	ಯದಿ (!spy)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_ಗಂಟು, &state->spy_head);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ event_uninit(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ event_state *state = gd_event_state();
	ಸಂರಚನಾ event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_ಗಂಟು)
		spy_free(spy);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ event_init(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* EVENT_DYNAMIC */
