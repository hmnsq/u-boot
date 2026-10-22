// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_CATEGORY	LOGC_EVENT

#ਅਨ੍ਤਰ੍ਭਾਵ <event.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <event_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linker_lists.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/list.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <relocate.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

#ਜੇ CONFIG_IS_ENABLED(EVENT_DEBUG)
ਅਚਲ ਅੱਖਰ *ਅਚਲ type_name[] = {
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

_Static_assert(ARRAY_ਮਾਪ(type_name) == EVT_ਗਿਣਤੀ, "event type_name size");
#ਜੇ_ਅੰਤ

ਅਚਲ ਅੱਖਰ *event_type_name(ਗਿਣਤੀ event_t type)
{
#ਜੇ CONFIG_IS_ENABLED(EVENT_DEBUG)
	ਜੇ (type < ARRAY_ਮਾਪ(type_name))
		ਵਾਪਸ type_name[type];
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ "(unknown)";
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ "(unknown)";
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ notify_static(ਬਣਤਰ event *ev)
{
	ਬਣਤਰ evspy_info *ਸ਼ੁਰੂ =
		ll_ਇੰਦਰਾਜ_ਸ਼ੁਰੂ(ਬਣਤਰ evspy_info, evspy_info);
	ਅਚਲ ਪੂਰਨ n_ents = ll_ਇੰਦਰਾਜ_ਗਿਣਤੀ(ਬਣਤਰ evspy_info, evspy_info);
	ਬਣਤਰ evspy_info *spy;

	ਲਈ (spy = ਸ਼ੁਰੂ; spy != ਸ਼ੁਰੂ + n_ents; spy++) {
		ਜੇ (spy->type == ev->type) {
			ਪੂਰਨ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			ਜੇ (spy->flags & EVSPYF_SIMPLE) {
				ਅਚਲ ਬਣਤਰ evspy_info_simple *simple;

				simple = (ਬਣਤਰ evspy_info_simple *)spy;
				ret = simple->func();
			} ਨਹੀਂਤਾਂ {
				ret = spy->func(ਨਿਰਾ, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ਜੇ (ret)
				ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("spy", ret);
		}
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ notify_dynamic(ਬਣਤਰ event *ev)
{
	ਬਣਤਰ event_state *state = gd_event_state();
	ਬਣਤਰ event_spy *spy, *ਅਗਲਾ;

	list_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(spy, ਅਗਲਾ, &state->spy_ਸਿਰ, sibling_ਗੰਢ) {
		ਜੇ (spy->type == ev->type) {
			ਪੂਰਨ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ਜੇ (ret)
				ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("spy", ret);
		}
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ event_notify(ਗਿਣਤੀ event_t type, ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ ਮਾਪ)
{
	ਬਣਤਰ event event;
	ਪੂਰਨ ret;

	event.type = type;
	ਜੇ (ਮਾਪ > ਆਕਾਰ(event.ਅੰਕੜੇ))
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("size", -E2BIG);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(&event.ਅੰਕੜੇ, ਅੰਕੜੇ, ਮਾਪ);

	ret = notify_static(&event);
	ਜੇ (ret)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("sta", ret);

	ਜੇ (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		ਜੇ (ret)
			ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("dyn", ret);
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ event_notify_null(ਗਿਣਤੀ event_t type)
{
	ਵਾਪਸ event_notify(type, ਨਿਰਾ, 0);
}

ਖਾਲੀ event_show_spy_list(ਖਾਲੀ)
{
	ਬਣਤਰ evspy_info *ਸ਼ੁਰੂ =
		ll_ਇੰਦਰਾਜ_ਸ਼ੁਰੂ(ਬਣਤਰ evspy_info, evspy_info);
	ਅਚਲ ਪੂਰਨ n_ents = ll_ਇੰਦਰਾਜ_ਗਿਣਤੀ(ਬਣਤਰ evspy_info, evspy_info);
	ਬਣਤਰ evspy_info *spy;
	ਅਚਲ ਪੂਰਨ ਮਾਪ = ਆਕਾਰ(ulong) * 2;

	ਲਿਖੋ("Seq  %-24s  %*s  %s\n", "Type", ਮਾਪ, "Function", "ID");
	ਲਈ (spy = ਸ਼ੁਰੂ; spy != ਸ਼ੁਰੂ + n_ents; spy++) {
		ਲਿਖੋ("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - ਸ਼ੁਰੂ),
		       spy->type, event_type_name(spy->type), ਮਾਪ, spy->func,
		       event_spy_id(spy));
	}
}

#ਜੇ CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ਸਥਿਰ ਖਾਲੀ spy_free(ਬਣਤਰ event_spy *spy)
{
	list_del(&spy->sibling_ਗੰਢ);
}

ਪੂਰਨ event_ਰਜਿਸਟਰ(ਅਚਲ ਅੱਖਰ *id, ਗਿਣਤੀ event_t type, event_handler_t func, ਖਾਲੀ *ctx)
{
	ਬਣਤਰ event_state *state = gd_event_state();
	ਬਣਤਰ event_spy *spy;

	spy = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(*spy));
	ਜੇ (!spy)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_ਜੋੜੋ_ਪੂਛ(&spy->sibling_ਗੰਢ, &state->spy_ਸਿਰ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ event_uninit(ਖਾਲੀ)
{
	ਬਣਤਰ event_state *state = gd_event_state();
	ਬਣਤਰ event_spy *spy, *ਅਗਲਾ;

	list_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(spy, ਅਗਲਾ, &state->spy_ਸਿਰ, sibling_ਗੰਢ)
		spy_free(spy);

	ਵਾਪਸ 0;
}

ਪੂਰਨ event_init(ਖਾਲੀ)
{
	ਬਣਤਰ event_state *state = gd_event_state();

	INIT_LIST_ਸਿਰ(&state->spy_ਸਿਰ);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* EVENT_DYNAMIC */
