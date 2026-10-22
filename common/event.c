// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#അന്തര്ഭാവ <event.h>
#അന്തര്ഭാവ <event_internal.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <linker_lists.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <linux/errno.h>
#അന്തര്ഭാവ <linux/list.h>
#അന്തര്ഭാവ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#എങ്കിൽ CONFIG_IS_ENABLED(EVENT_DEBUG)
സ്ഥിരം അക്ഷരം *സ്ഥിരം type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_എണ്ണം, "event type_name size");
#endif

സ്ഥിരം അക്ഷരം *event_type_name(എണ്ണൽ event_t type)
{
#എങ്കിൽ CONFIG_IS_ENABLED(EVENT_DEBUG)
	എങ്കിൽ (type < ARRAY_SIZE(type_name))
		തിരികെ type_name[type];
	അല്ലെങ്കിൽ
		തിരികെ "(unknown)";
#അല്ലെങ്കിൽ
	തിരികെ "(unknown)";
#endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ notify_static(ഘടന event *ev)
{
	ഘടന evspy_info *start =
		ll_entry_start(ഘടന evspy_info, evspy_info);
	സ്ഥിരം പൂർണ്ണസംഖ്യ n_ents = ll_entry_എണ്ണം(ഘടന evspy_info, evspy_info);
	ഘടന evspy_info *spy;

	വേണ്ടി (spy = start; spy != start + n_ents; spy++) {
		എങ്കിൽ (spy->type == ev->type) {
			പൂർണ്ണസംഖ്യ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			എങ്കിൽ (spy->flags & EVSPYF_SIMPLE) {
				സ്ഥിരം ഘടന evspy_info_simple *simple;

				simple = (ഘടന evspy_info_simple *)spy;
				ret = simple->func();
			} അല്ലെങ്കിൽ {
				ret = spy->func(ശൂന്യ, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			എങ്കിൽ (ret)
				തിരികെ log_സന്ദേശം_ret("spy", ret);
		}
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ notify_dynamic(ഘടന event *ev)
{
	ഘടന event_state *state = gd_event_state();
	ഘടന event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_കണ്ണി) {
		എങ്കിൽ (spy->type == ev->type) {
			പൂർണ്ണസംഖ്യ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			എങ്കിൽ (ret)
				തിരികെ log_സന്ദേശം_ret("spy", ret);
		}
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ event_notify(എണ്ണൽ event_t type, ശൂന്യം *data, പൂർണ്ണസംഖ്യ size)
{
	ഘടന event event;
	പൂർണ്ണസംഖ്യ ret;

	event.type = type;
	എങ്കിൽ (size > വലുപ്പം(event.data))
		തിരികെ log_സന്ദേശം_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	എങ്കിൽ (ret)
		തിരികെ log_സന്ദേശം_ret("sta", ret);

	എങ്കിൽ (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		എങ്കിൽ (ret)
			തിരികെ log_സന്ദേശം_ret("dyn", ret);
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ event_notify_null(എണ്ണൽ event_t type)
{
	തിരികെ event_notify(type, ശൂന്യ, 0);
}

ശൂന്യം event_show_spy_list(ശൂന്യം)
{
	ഘടന evspy_info *start =
		ll_entry_start(ഘടന evspy_info, evspy_info);
	സ്ഥിരം പൂർണ്ണസംഖ്യ n_ents = ll_entry_എണ്ണം(ഘടന evspy_info, evspy_info);
	ഘടന evspy_info *spy;
	സ്ഥിരം പൂർണ്ണസംഖ്യ size = വലുപ്പം(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	വേണ്ടി (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#എങ്കിൽ CONFIG_IS_ENABLED(EVENT_DYNAMIC)
സ്ഥിതം ശൂന്യം spy_free(ഘടന event_spy *spy)
{
	list_del(&spy->sibling_കണ്ണി);
}

പൂർണ്ണസംഖ്യ event_register(സ്ഥിരം അക്ഷരം *id, എണ്ണൽ event_t type, event_handler_t func, ശൂന്യം *ctx)
{
	ഘടന event_state *state = gd_event_state();
	ഘടന event_spy *spy;

	spy = malloc(വലുപ്പം(*spy));
	എങ്കിൽ (!spy)
		തിരികെ log_സന്ദേശം_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_കണ്ണി, &state->spy_head);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ event_uninit(ശൂന്യം)
{
	ഘടന event_state *state = gd_event_state();
	ഘടന event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_കണ്ണി)
		spy_free(spy);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ event_init(ശൂന്യം)
{
	ഘടന event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	തിരികെ 0;
}
#endif /* EVENT_DYNAMIC */
