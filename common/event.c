// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#અન્તર્ભાવ <event.h>
#અન્તર્ભાવ <event_internal.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <linker_lists.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <linux/errno.h>
#અન્તર્ભાવ <linux/list.h>
#અન્તર્ભાવ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#જો CONFIG_IS_ENABLED(EVENT_DEBUG)
અચલ અક્ષર *અચલ type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_ગણતરી, "event type_name size");
#endif

અચલ અક્ષર *event_type_name(ગણના event_t type)
{
#જો CONFIG_IS_ENABLED(EVENT_DEBUG)
	જો (type < ARRAY_SIZE(type_name))
		પાછા type_name[type];
	નહીંતર
		પાછા "(unknown)";
#નહીંતર
	પાછા "(unknown)";
#endif
}

સ્થિર પૂર્ણાંક notify_static(સંરચના event *ev)
{
	સંરચના evspy_info *start =
		ll_entry_start(સંરચના evspy_info, evspy_info);
	અચલ પૂર્ણાંક n_ents = ll_entry_ગણતરી(સંરચના evspy_info, evspy_info);
	સંરચના evspy_info *spy;

	માટે (spy = start; spy != start + n_ents; spy++) {
		જો (spy->type == ev->type) {
			પૂર્ણાંક ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			જો (spy->flags & EVSPYF_SIMPLE) {
				અચલ સંરચના evspy_info_simple *simple;

				simple = (સંરચના evspy_info_simple *)spy;
				ret = simple->func();
			} નહીંતર {
				ret = spy->func(ખાલી, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			જો (ret)
				પાછા log_સંદેશ_ret("spy", ret);
		}
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક notify_dynamic(સંરચના event *ev)
{
	સંરચના event_state *state = gd_event_state();
	સંરચના event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_ગાંઠ) {
		જો (spy->type == ev->type) {
			પૂર્ણાંક ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			જો (ret)
				પાછા log_સંદેશ_ret("spy", ret);
		}
	}

	પાછા 0;
}

પૂર્ણાંક event_notify(ગણના event_t type, શૂન્ય *data, પૂર્ણાંક size)
{
	સંરચના event event;
	પૂર્ણાંક ret;

	event.type = type;
	જો (size > કદ(event.data))
		પાછા log_સંદેશ_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	જો (ret)
		પાછા log_સંદેશ_ret("sta", ret);

	જો (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		જો (ret)
			પાછા log_સંદેશ_ret("dyn", ret);
	}

	પાછા 0;
}

પૂર્ણાંક event_notify_null(ગણના event_t type)
{
	પાછા event_notify(type, ખાલી, 0);
}

શૂન્ય event_show_spy_list(શૂન્ય)
{
	સંરચના evspy_info *start =
		ll_entry_start(સંરચના evspy_info, evspy_info);
	અચલ પૂર્ણાંક n_ents = ll_entry_ગણતરી(સંરચના evspy_info, evspy_info);
	સંરચના evspy_info *spy;
	અચલ પૂર્ણાંક size = કદ(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	માટે (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#જો CONFIG_IS_ENABLED(EVENT_DYNAMIC)
સ્થિર શૂન્ય spy_free(સંરચના event_spy *spy)
{
	list_del(&spy->sibling_ગાંઠ);
}

પૂર્ણાંક event_register(અચલ અક્ષર *id, ગણના event_t type, event_handler_t func, શૂન્ય *ctx)
{
	સંરચના event_state *state = gd_event_state();
	સંરચના event_spy *spy;

	spy = malloc(કદ(*spy));
	જો (!spy)
		પાછા log_સંદેશ_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_ગાંઠ, &state->spy_head);

	પાછા 0;
}

પૂર્ણાંક event_uninit(શૂન્ય)
{
	સંરચના event_state *state = gd_event_state();
	સંરચના event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_ગાંઠ)
		spy_free(spy);

	પાછા 0;
}

પૂર્ણાંક event_init(શૂન્ય)
{
	સંરચના event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	પાછા 0;
}
#endif /* EVENT_DYNAMIC */
