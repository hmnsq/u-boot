// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کرو LOG_CATEGORY	LOGC_EVENT

#شامل <event.h>
#شامل <event_internal.h>
#شامل <log.h>
#شامل <linker_lists.h>
#شامل <malloc.h>
#شامل <asm/global_data.h>
#شامل <linux/errno.h>
#شامل <linux/list.h>
#شامل <relocate.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

#اگر CONFIG_IS_ENABLED(EVENT_DEBUG)
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

_Static_assert(ARRAY_سائز(type_name) == EVT_گنتی, "event type_name size");
#اگر_ختم

ثابت حرف *event_type_name(شمار event_t type)
{
#اگر CONFIG_IS_ENABLED(EVENT_DEBUG)
	اگر (type < ARRAY_سائز(type_name))
		واپس type_name[type];
	ورنہ
		واپس "(unknown)";
#ورنہ
	واپس "(unknown)";
#اگر_ختم
}

ساکن صحیح notify_static(ساخت event *ev)
{
	ساخت evspy_info *آغاز =
		ll_اندراج_آغاز(ساخت evspy_info, evspy_info);
	ثابت صحیح n_ents = ll_اندراج_گنتی(ساخت evspy_info, evspy_info);
	ساخت evspy_info *spy;

	برائے (spy = آغاز; spy != آغاز + n_ents; spy++) {
		اگر (spy->type == ev->type) {
			صحیح ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			اگر (spy->flags & EVSPYF_SIMPLE) {
				ثابت ساخت evspy_info_simple *simple;

				simple = (ساخت evspy_info_simple *)spy;
				ret = simple->func();
			} ورنہ {
				ret = spy->func(تہی, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			اگر (ret)
				واپس log_پیغام_ret("spy", ret);
		}
	}

	واپس 0;
}

ساکن صحیح notify_dynamic(ساخت event *ev)
{
	ساخت event_state *state = gd_event_state();
	ساخت event_spy *spy, *اگلا;

	list_for_each_اندراج_محفوظ(spy, اگلا, &state->spy_سر, sibling_گرہ) {
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
				واپس log_پیغام_ret("spy", ret);
		}
	}

	واپس 0;
}

صحیح event_notify(شمار event_t type, خالی *ڈیٹا, صحیح سائز)
{
	ساخت event event;
	صحیح ret;

	event.type = type;
	اگر (سائز > حجم(event.ڈیٹا))
		واپس log_پیغام_ret("size", -E2BIG);
	حافظہ_نقل(&event.ڈیٹا, ڈیٹا, سائز);

	ret = notify_static(&event);
	اگر (ret)
		واپس log_پیغام_ret("sta", ret);

	اگر (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		اگر (ret)
			واپس log_پیغام_ret("dyn", ret);
	}

	واپس 0;
}

صحیح event_notify_null(شمار event_t type)
{
	واپس event_notify(type, تہی, 0);
}

خالی event_show_spy_list(خالی)
{
	ساخت evspy_info *آغاز =
		ll_اندراج_آغاز(ساخت evspy_info, evspy_info);
	ثابت صحیح n_ents = ll_اندراج_گنتی(ساخت evspy_info, evspy_info);
	ساخت evspy_info *spy;
	ثابت صحیح سائز = حجم(ulong) * 2;

	لکھو("Seq  %-24s  %*s  %s\n", "Type", سائز, "Function", "ID");
	برائے (spy = آغاز; spy != آغاز + n_ents; spy++) {
		لکھو("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - آغاز),
		       spy->type, event_type_name(spy->type), سائز, spy->func,
		       event_spy_id(spy));
	}
}

#اگر CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ساکن خالی spy_free(ساخت event_spy *spy)
{
	list_del(&spy->sibling_گرہ);
}

صحیح event_رجسٹر(ثابت حرف *id, شمار event_t type, event_handler_t func, خالی *ctx)
{
	ساخت event_state *state = gd_event_state();
	ساخت event_spy *spy;

	spy = حافظہ_دو(حجم(*spy));
	اگر (!spy)
		واپس log_پیغام_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_جوڑو_دم(&spy->sibling_گرہ, &state->spy_سر);

	واپس 0;
}

صحیح event_uninit(خالی)
{
	ساخت event_state *state = gd_event_state();
	ساخت event_spy *spy, *اگلا;

	list_for_each_اندراج_محفوظ(spy, اگلا, &state->spy_سر, sibling_گرہ)
		spy_free(spy);

	واپس 0;
}

صحیح event_init(خالی)
{
	ساخت event_state *state = gd_event_state();

	INIT_LIST_سر(&state->spy_سر);

	واپس 0;
}
#اگر_ختم /* EVENT_DYNAMIC */
