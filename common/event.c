// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کن LOG_CATEGORY	LOGC_EVENT

#ضمن <event.h>
#ضمن <event_internal.h>
#ضمن <log.h>
#ضمن <linker_lists.h>
#ضمن <malloc.h>
#ضمن <asm/global_data.h>
#ضمن <linux/errno.h>
#ضمن <linux/list.h>
#ضمن <relocate.h>

DECLARE_GLOBAL_داده_PTR;

#إذا CONFIG_IS_ENABLED(EVENT_DEBUG)
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

_Static_assert(ARRAY_اندازه(type_name) == EVT_شمار, "event type_name size");
#پایان_اگر

ثابت حرف *event_type_name(تعداد event_t type)
{
#إذا CONFIG_IS_ENABLED(EVENT_DEBUG)
	إذا (type < ARRAY_اندازه(type_name))
		أرجع type_name[type];
	وإلا
		أرجع "(unknown)";
#وإلا
	أرجع "(unknown)";
#پایان_اگر
}

ساكن صحيح notify_static(بنية event *ev)
{
	بنية evspy_info *شروع =
		ll_مدخل_شروع(بنية evspy_info, evspy_info);
	ثابت صحيح n_ents = ll_مدخل_شمار(بنية evspy_info, evspy_info);
	بنية evspy_info *spy;

	لكل (spy = شروع; spy != شروع + n_ents; spy++) {
		إذا (spy->type == ev->type) {
			صحيح ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			إذا (spy->flags & EVSPYF_SIMPLE) {
				ثابت بنية evspy_info_simple *simple;

				simple = (بنية evspy_info_simple *)spy;
				ret = simple->func();
			} وإلا {
				ret = spy->func(عدم, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			إذا (ret)
				أرجع log_پیام_ret("spy", ret);
		}
	}

	أرجع 0;
}

ساكن صحيح notify_dynamic(بنية event *ev)
{
	بنية event_state *state = gd_event_state();
	بنية event_spy *spy, *بعدی;

	list_for_each_مدخل_امن(spy, بعدی, &state->spy_سر, sibling_گره) {
		إذا (spy->type == ev->type) {
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
			إذا (ret)
				أرجع log_پیام_ret("spy", ret);
		}
	}

	أرجع 0;
}

صحيح event_notify(تعداد event_t type, فراغ *داده, صحيح اندازه)
{
	بنية event event;
	صحيح ret;

	event.type = type;
	إذا (اندازه > حجم(event.داده))
		أرجع log_پیام_ret("size", -E2BIG);
	کپی_حافظه(&event.داده, داده, اندازه);

	ret = notify_static(&event);
	إذا (ret)
		أرجع log_پیام_ret("sta", ret);

	إذا (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		إذا (ret)
			أرجع log_پیام_ret("dyn", ret);
	}

	أرجع 0;
}

صحيح event_notify_null(تعداد event_t type)
{
	أرجع event_notify(type, عدم, 0);
}

فراغ event_show_spy_list(فراغ)
{
	بنية evspy_info *شروع =
		ll_مدخل_شروع(بنية evspy_info, evspy_info);
	ثابت صحيح n_ents = ll_مدخل_شمار(بنية evspy_info, evspy_info);
	بنية evspy_info *spy;
	ثابت صحيح اندازه = حجم(ulong) * 2;

	چاپ_کن("Seq  %-24s  %*s  %s\n", "Type", اندازه, "Function", "ID");
	لكل (spy = شروع; spy != شروع + n_ents; spy++) {
		چاپ_کن("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - شروع),
		       spy->type, event_type_name(spy->type), اندازه, spy->func,
		       event_spy_id(spy));
	}
}

#إذا CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ساكن فراغ spy_free(بنية event_spy *spy)
{
	list_del(&spy->sibling_گره);
}

صحيح event_ثبات(ثابت حرف *id, تعداد event_t type, event_handler_t func, فراغ *ctx)
{
	بنية event_state *state = gd_event_state();
	بنية event_spy *spy;

	spy = حافظه_بده(حجم(*spy));
	إذا (!spy)
		أرجع log_پیام_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_جمع_کن_دم(&spy->sibling_گره, &state->spy_سر);

	أرجع 0;
}

صحيح event_uninit(فراغ)
{
	بنية event_state *state = gd_event_state();
	بنية event_spy *spy, *بعدی;

	list_for_each_مدخل_امن(spy, بعدی, &state->spy_سر, sibling_گره)
		spy_free(spy);

	أرجع 0;
}

صحيح event_init(فراغ)
{
	بنية event_state *state = gd_event_state();

	INIT_LIST_سر(&state->spy_سر);

	أرجع 0;
}
#پایان_اگر /* EVENT_DYNAMIC */
