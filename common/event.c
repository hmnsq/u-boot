// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#నిర్వచించు LOG_CATEGORY	LOGC_EVENT

#అన్తర్భావ <event.h>
#అన్తర్భావ <event_internal.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <linker_lists.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <linux/errno.h>
#అన్తర్భావ <linux/list.h>
#అన్తర్భావ <relocate.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

#ఒకవేళ CONFIG_IS_ENABLED(EVENT_DEBUG)
స్థిరరాశి అక్షరం *స్థిరరాశి type_name[] = {
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

_Static_assert(ARRAY_కొలత(type_name) == EVT_లెక్క, "event type_name size");
#అయితే_ముగింపు

స్థిరరాశి అక్షరం *event_type_name(గణన event_t type)
{
#ఒకవేళ CONFIG_IS_ENABLED(EVENT_DEBUG)
	ఒకవేళ (type < ARRAY_కొలత(type_name))
		తిరిగి type_name[type];
	లేకపోతే
		తిరిగి "(unknown)";
#లేకపోతే
	తిరిగి "(unknown)";
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం notify_static(నిర్మాణం event *ev)
{
	నిర్మాణం evspy_info *ప్రారంభం =
		ll_నమోదు_ప్రారంభం(నిర్మాణం evspy_info, evspy_info);
	స్థిరరాశి పూర్ణాంకం n_ents = ll_నమోదు_లెక్క(నిర్మాణం evspy_info, evspy_info);
	నిర్మాణం evspy_info *spy;

	కొరకు (spy = ప్రారంభం; spy != ప్రారంభం + n_ents; spy++) {
		ఒకవేళ (spy->type == ev->type) {
			పూర్ణాంకం ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			ఒకవేళ (spy->flags & EVSPYF_SIMPLE) {
				స్థిరరాశి నిర్మాణం evspy_info_simple *simple;

				simple = (నిర్మాణం evspy_info_simple *)spy;
				ret = simple->func();
			} లేకపోతే {
				ret = spy->func(ఖాళీ, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ఒకవేళ (ret)
				తిరిగి log_సందేశం_ret("spy", ret);
		}
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం notify_dynamic(నిర్మాణం event *ev)
{
	నిర్మాణం event_state *state = gd_event_state();
	నిర్మాణం event_spy *spy, *తదుపరి;

	list_for_each_నమోదు_సురక్షిత(spy, తదుపరి, &state->spy_తల, sibling_కణుపు) {
		ఒకవేళ (spy->type == ev->type) {
			పూర్ణాంకం ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ఒకవేళ (ret)
				తిరిగి log_సందేశం_ret("spy", ret);
		}
	}

	తిరిగి 0;
}

పూర్ణాంకం event_notify(గణన event_t type, శూన్య *దత్తాంశం, పూర్ణాంకం కొలత)
{
	నిర్మాణం event event;
	పూర్ణాంకం ret;

	event.type = type;
	ఒకవేళ (కొలత > పరిమాణం(event.దత్తాంశం))
		తిరిగి log_సందేశం_ret("size", -E2BIG);
	జ్ఞాపకం_నకలు(&event.దత్తాంశం, దత్తాంశం, కొలత);

	ret = notify_static(&event);
	ఒకవేళ (ret)
		తిరిగి log_సందేశం_ret("sta", ret);

	ఒకవేళ (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		ఒకవేళ (ret)
			తిరిగి log_సందేశం_ret("dyn", ret);
	}

	తిరిగి 0;
}

పూర్ణాంకం event_notify_null(గణన event_t type)
{
	తిరిగి event_notify(type, ఖాళీ, 0);
}

శూన్య event_show_spy_list(శూన్య)
{
	నిర్మాణం evspy_info *ప్రారంభం =
		ll_నమోదు_ప్రారంభం(నిర్మాణం evspy_info, evspy_info);
	స్థిరరాశి పూర్ణాంకం n_ents = ll_నమోదు_లెక్క(నిర్మాణం evspy_info, evspy_info);
	నిర్మాణం evspy_info *spy;
	స్థిరరాశి పూర్ణాంకం కొలత = పరిమాణం(ulong) * 2;

	రాయి("Seq  %-24s  %*s  %s\n", "Type", కొలత, "Function", "ID");
	కొరకు (spy = ప్రారంభం; spy != ప్రారంభం + n_ents; spy++) {
		రాయి("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - ప్రారంభం),
		       spy->type, event_type_name(spy->type), కొలత, spy->func,
		       event_spy_id(spy));
	}
}

#ఒకవేళ CONFIG_IS_ENABLED(EVENT_DYNAMIC)
స్థిర శూన్య spy_free(నిర్మాణం event_spy *spy)
{
	list_del(&spy->sibling_కణుపు);
}

పూర్ణాంకం event_రిజిస్టరు(స్థిరరాశి అక్షరం *id, గణన event_t type, event_handler_t func, శూన్య *ctx)
{
	నిర్మాణం event_state *state = gd_event_state();
	నిర్మాణం event_spy *spy;

	spy = జ్ఞాపకం_ఇవ్వు(పరిమాణం(*spy));
	ఒకవేళ (!spy)
		తిరిగి log_సందేశం_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_కూడు_తోక(&spy->sibling_కణుపు, &state->spy_తల);

	తిరిగి 0;
}

పూర్ణాంకం event_uninit(శూన్య)
{
	నిర్మాణం event_state *state = gd_event_state();
	నిర్మాణం event_spy *spy, *తదుపరి;

	list_for_each_నమోదు_సురక్షిత(spy, తదుపరి, &state->spy_తల, sibling_కణుపు)
		spy_free(spy);

	తిరిగి 0;
}

పూర్ణాంకం event_init(శూన్య)
{
	నిర్మాణం event_state *state = gd_event_state();

	INIT_LIST_తల(&state->spy_తల);

	తిరిగి 0;
}
#అయితే_ముగింపు /* EVENT_DYNAMIC */
