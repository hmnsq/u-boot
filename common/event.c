// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#ଅନ୍ତର୍ଭାଵ <event.h>
#ଅନ୍ତର୍ଭାଵ <event_internal.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <linker_lists.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>
#ଅନ୍ତର୍ଭାଵ <linux/list.h>
#ଅନ୍ତର୍ଭାଵ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#ଯଦି CONFIG_IS_ENABLED(EVENT_DEBUG)
ଅଚଳ ଅକ୍ଷର *ଅଚଳ type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_ଗଣନା, "event type_name size");
#endif

ଅଚଳ ଅକ୍ଷର *event_type_name(ଗଣନା event_t type)
{
#ଯଦି CONFIG_IS_ENABLED(EVENT_DEBUG)
	ଯଦି (type < ARRAY_SIZE(type_name))
		ଫେରନ୍ତୁ type_name[type];
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ "(unknown)";
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ "(unknown)";
#endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା notify_static(ସଂରଚନା event *ev)
{
	ସଂରଚନା evspy_info *start =
		ll_entry_start(ସଂରଚନା evspy_info, evspy_info);
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ents = ll_entry_ଗଣନା(ସଂରଚନା evspy_info, evspy_info);
	ସଂରଚନା evspy_info *spy;

	ପାଇଁ (spy = start; spy != start + n_ents; spy++) {
		ଯଦି (spy->type == ev->type) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			ଯଦି (spy->flags & EVSPYF_SIMPLE) {
				ଅଚଳ ସଂରଚନା evspy_info_simple *simple;

				simple = (ସଂରଚନା evspy_info_simple *)spy;
				ret = simple->func();
			} ଅନ୍ୟଥା {
				ret = spy->func(ଖାଲି, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ଯଦି (ret)
				ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("spy", ret);
		}
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା notify_dynamic(ସଂରଚନା event *ev)
{
	ସଂରଚନା event_state *state = gd_event_state();
	ସଂରଚନା event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_ଗଣ୍ଠି) {
		ଯଦି (spy->type == ev->type) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ଯଦି (ret)
				ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("spy", ret);
		}
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_notify(ଗଣନା event_t type, ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ସଂରଚନା event event;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	event.type = type;
	ଯଦି (size > ଆକାର(event.data))
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	ଯଦି (ret)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("sta", ret);

	ଯଦି (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		ଯଦି (ret)
			ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("dyn", ret);
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_notify_null(ଗଣନା event_t type)
{
	ଫେରନ୍ତୁ event_notify(type, ଖାଲି, 0);
}

ଶୂନ୍ୟ event_show_spy_list(ଶୂନ୍ୟ)
{
	ସଂରଚନା evspy_info *start =
		ll_entry_start(ସଂରଚନା evspy_info, evspy_info);
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ents = ll_entry_ଗଣନା(ସଂରଚନା evspy_info, evspy_info);
	ସଂରଚନା evspy_info *spy;
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = ଆକାର(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	ପାଇଁ (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#ଯଦି CONFIG_IS_ENABLED(EVENT_DYNAMIC)
ସ୍ଥିର ଶୂନ୍ୟ spy_free(ସଂରଚନା event_spy *spy)
{
	list_del(&spy->sibling_ଗଣ୍ଠି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_register(ଅଚଳ ଅକ୍ଷର *id, ଗଣନା event_t type, event_handler_t func, ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା event_state *state = gd_event_state();
	ସଂରଚନା event_spy *spy;

	spy = malloc(ଆକାର(*spy));
	ଯଦି (!spy)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_ଗଣ୍ଠି, &state->spy_head);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_uninit(ଶୂନ୍ୟ)
{
	ସଂରଚନା event_state *state = gd_event_state();
	ସଂରଚନା event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_ଗଣ୍ଠି)
		spy_free(spy);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_init(ଶୂନ୍ୟ)
{
	ସଂରଚନା event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	ଫେରନ୍ତୁ 0;
}
#endif /* EVENT_DYNAMIC */
