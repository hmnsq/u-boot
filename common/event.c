// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#include <event.h>
#include <event_internal.h>
#include <log.h>
#include <linker_lists.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#ຖ້າ CONFIG_IS_ENABLED(EVENT_DEBUG)
const var_na *const type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_COUNT, "event type_name size");
#endif

const var_na *event_type_name(ga_nanaa event_t type)
{
#ຖ້າ CONFIG_IS_ENABLED(EVENT_DEBUG)
	ຖ້າ (type < ARRAY_SIZE(type_name))
		ສົ່ງຄືນ type_name[type];
	ບໍ່ດັ່ງນັ້ນ
		ສົ່ງຄືນ "(unknown)";
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ "(unknown)";
#endif
}

static ຈຳນວນເຕັມ notify_static(sa_nracanaa event *ev)
{
	sa_nracanaa evspy_info *start =
		ll_entry_start(sa_nracanaa evspy_info, evspy_info);
	const ຈຳນວນເຕັມ n_ents = ll_entry_count(sa_nracanaa evspy_info, evspy_info);
	sa_nracanaa evspy_info *spy;

	ສຳລັບ (spy = start; spy != start + n_ents; spy++) {
		ຖ້າ (spy->type == ev->type) {
			ຈຳນວນເຕັມ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			ຖ້າ (spy->flags & EVSPYF_SIMPLE) {
				const sa_nracanaa evspy_info_simple *simple;

				simple = (sa_nracanaa evspy_info_simple *)spy;
				ret = simple->func();
			} ບໍ່ດັ່ງນັ້ນ {
				ret = spy->func(NULL, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ຖ້າ (ret)
				ສົ່ງຄືນ log_msg_ret("spy", ret);
		}
	}

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ notify_dynamic(sa_nracanaa event *ev)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node) {
		ຖ້າ (spy->type == ev->type) {
			ຈຳນວນເຕັມ ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			ຖ້າ (ret)
				ສົ່ງຄືນ log_msg_ret("spy", ret);
		}
	}

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ event_notify(ga_nanaa event_t type, void *data, ຈຳນວນເຕັມ size)
{
	sa_nracanaa event event;
	ຈຳນວນເຕັມ ret;

	event.type = type;
	ຖ້າ (size > parimaa_na(event.data))
		ສົ່ງຄືນ log_msg_ret("size", -E2BIG);
	memcpy(&event.data, data, size);

	ret = notify_static(&event);
	ຖ້າ (ret)
		ສົ່ງຄືນ log_msg_ret("sta", ret);

	ຖ້າ (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		ຖ້າ (ret)
			ສົ່ງຄືນ log_msg_ret("dyn", ret);
	}

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ event_notify_null(ga_nanaa event_t type)
{
	ສົ່ງຄືນ event_notify(type, NULL, 0);
}

void event_show_spy_list(void)
{
	sa_nracanaa evspy_info *start =
		ll_entry_start(sa_nracanaa evspy_info, evspy_info);
	const ຈຳນວນເຕັມ n_ents = ll_entry_count(sa_nracanaa evspy_info, evspy_info);
	sa_nracanaa evspy_info *spy;
	const ຈຳນວນເຕັມ size = parimaa_na(ulong) * 2;

	printf("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	ສຳລັບ (spy = start; spy != start + n_ents; spy++) {
		printf("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#ຖ້າ CONFIG_IS_ENABLED(EVENT_DYNAMIC)
static void spy_free(sa_nracanaa event_spy *spy)
{
	list_del(&spy->sibling_node);
}

ຈຳນວນເຕັມ event_register(const var_na *id, ga_nanaa event_t type, event_handler_t func, void *ctx)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy;

	spy = malloc(parimaa_na(*spy));
	ຖ້າ (!spy)
		ສົ່ງຄືນ log_msg_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_node, &state->spy_head);

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ event_uninit(void)
{
	sa_nracanaa event_state *state = gd_event_state();
	sa_nracanaa event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_node)
		spy_free(spy);

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ event_init(void)
{
	sa_nracanaa event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	ສົ່ງຄືນ 0;
}
#endif /* EVENT_DYNAMIC */
