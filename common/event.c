// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_CATEGORY	LOGC_EVENT

#ಅನ್ತರ್ಭಾವ <event.h>
#ಅನ್ತರ್ಭಾವ <event_internal.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <linker_lists.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>
#ಅನ್ತರ್ಭಾವ <linux/list.h>
#ಅನ್ತರ್ಭಾವ <relocate.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

#ಆದರೆ CONFIG_IS_ENABLED(EVENT_DEBUG)
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ type_name[] = {
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

_Static_assert(ARRAY_ಗಾತ್ರ(type_name) == EVT_ಎಣಿಕೆ, "event type_name size");
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *event_type_name(ಎಣಿಕೆ event_t type)
{
#ಆದರೆ CONFIG_IS_ENABLED(EVENT_DEBUG)
	ಆದರೆ (type < ARRAY_ಗಾತ್ರ(type_name))
		ಹಿಂತಿರುಗಿ type_name[type];
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ "(unknown)";
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ "(unknown)";
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ notify_static(ರಚನೆ event *ev)
{
	ರಚನೆ evspy_info *ಆರಂಭ =
		ll_ನಮೂದು_ಆರಂಭ(ರಚನೆ evspy_info, evspy_info);
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ents = ll_ನಮೂದು_ಎಣಿಕೆ(ರಚನೆ evspy_info, evspy_info);
	ರಚನೆ evspy_info *spy;

	ಗಾಗಿ (spy = ಆರಂಭ; spy != ಆರಂಭ + n_ents; spy++) {
		ಆದರೆ (spy->type == ev->type) {
			ಪೂರ್ಣಾಂಕ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			ಆದರೆ (spy->flags & EVSPYF_SIMPLE) {
				ಸ್ಥಿರಾಂಕ ರಚನೆ evspy_info_simple *simple;

				simple = (ರಚನೆ evspy_info_simple *)spy;
				ret = simple->func();
			} ಇಲ್ಲದಿದ್ದರೆ {
				ret = spy->func(ಖಾಲಿ, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ಆದರೆ (ret)
				ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("spy", ret);
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ notify_dynamic(ರಚನೆ event *ev)
{
	ರಚನೆ event_state *state = gd_event_state();
	ರಚನೆ event_spy *spy, *ಮುಂದಿನ;

	list_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(spy, ಮುಂದಿನ, &state->spy_ತಲೆ, sibling_ಗಂಟು) {
		ಆದರೆ (spy->type == ev->type) {
			ಪೂರ್ಣಾಂಕ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ಆದರೆ (ret)
				ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("spy", ret);
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ event_notify(ಎಣಿಕೆ event_t type, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ರಚನೆ event event;
	ಪೂರ್ಣಾಂಕ ret;

	event.type = type;
	ಆದರೆ (ಗಾತ್ರ_2 > ಗಾತ್ರ(event.ದತ್ತಾಂಶ))
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("size", -E2BIG);
	ಸ್ಮರಣೆ_ನಕಲು(&event.ದತ್ತಾಂಶ, ದತ್ತಾಂಶ, ಗಾತ್ರ_2);

	ret = notify_static(&event);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("sta", ret);

	ಆದರೆ (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("dyn", ret);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ event_notify_null(ಎಣಿಕೆ event_t type)
{
	ಹಿಂತಿರುಗಿ event_notify(type, ಖಾಲಿ, 0);
}

ಶೂನ್ಯ event_show_spy_list(ಶೂನ್ಯ)
{
	ರಚನೆ evspy_info *ಆರಂಭ =
		ll_ನಮೂದು_ಆರಂಭ(ರಚನೆ evspy_info, evspy_info);
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ents = ll_ನಮೂದು_ಎಣಿಕೆ(ರಚನೆ evspy_info, evspy_info);
	ರಚನೆ evspy_info *spy;
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ಗಾತ್ರ(ulong) * 2;

	ಬರೆ("Seq  %-24s  %*s  %s\n", "Type", ಗಾತ್ರ_2, "Function", "ID");
	ಗಾಗಿ (spy = ಆರಂಭ; spy != ಆರಂಭ + n_ents; spy++) {
		ಬರೆ("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - ಆರಂಭ),
		       spy->type, event_type_name(spy->type), ಗಾತ್ರ_2, spy->func,
		       event_spy_id(spy));
	}
}

#ಆದರೆ CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ಸ್ಥಿರ ಶೂನ್ಯ spy_free(ರಚನೆ event_spy *spy)
{
	list_del(&spy->sibling_ಗಂಟು);
}

ಪೂರ್ಣಾಂಕ event_ರಿಜಿಸ್ಟರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *id, ಎಣಿಕೆ event_t type, event_handler_t func, ಶೂನ್ಯ *ctx)
{
	ರಚನೆ event_state *state = gd_event_state();
	ರಚನೆ event_spy *spy;

	spy = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(*spy));
	ಆದರೆ (!spy)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_ಕೂಡಿಸು_ಬಾಲ(&spy->sibling_ಗಂಟು, &state->spy_ತಲೆ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ event_uninit(ಶೂನ್ಯ)
{
	ರಚನೆ event_state *state = gd_event_state();
	ರಚನೆ event_spy *spy, *ಮುಂದಿನ;

	list_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(spy, ಮುಂದಿನ, &state->spy_ತಲೆ, sibling_ಗಂಟು)
		spy_free(spy);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ event_init(ಶೂನ್ಯ)
{
	ರಚನೆ event_state *state = gd_event_state();

	INIT_LIST_ತಲೆ(&state->spy_ತಲೆ);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* EVENT_DYNAMIC */
