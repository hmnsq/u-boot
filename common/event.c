// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EVENT

#अन्तर्भाव <event.h>
#अन्तर्भाव <event_internal.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <linker_lists.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/errno.h>
#अन्तर्भाव <linux/list.h>
#अन्तर्भाव <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

#यदि CONFIG_IS_ENABLED(EVENT_DEBUG)
ध्रुव वर्ण *ध्रुव type_name[] = {
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

_Static_assert(ARRAY_SIZE(type_name) == EVT_गिनती, "event type_name size");
#endif

ध्रुव वर्ण *event_type_name(गणना event_t type)
{
#यदि CONFIG_IS_ENABLED(EVENT_DEBUG)
	यदि (type < ARRAY_SIZE(type_name))
		प्रत्यागम type_name[type];
	अन्यथा
		प्रत्यागम "(unknown)";
#अन्यथा
	प्रत्यागम "(unknown)";
#endif
}

स्थिर पूर्णाङ्क notify_static(संरचना event *ev)
{
	संरचना evspy_info *start =
		ll_entry_start(संरचना evspy_info, evspy_info);
	ध्रुव पूर्णाङ्क n_ents = ll_entry_गिनती(संरचना evspy_info, evspy_info);
	संरचना evspy_info *spy;

	प्रति (spy = start; spy != start + n_ents; spy++) {
		यदि (spy->type == ev->type) {
			पूर्णाङ्क ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			यदि (spy->flags & EVSPYF_SIMPLE) {
				ध्रुव संरचना evspy_info_simple *simple;

				simple = (संरचना evspy_info_simple *)spy;
				ret = simple->func();
			} अन्यथा {
				ret = spy->func(रिक्त, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			यदि (ret)
				प्रत्यागम log_संदेश_ret("spy", ret);
		}
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क notify_dynamic(संरचना event *ev)
{
	संरचना event_state *state = gd_event_state();
	संरचना event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_गाँठ) {
		यदि (spy->type == ev->type) {
			पूर्णाङ्क ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			यदि (ret)
				प्रत्यागम log_संदेश_ret("spy", ret);
		}
	}

	प्रत्यागम 0;
}

पूर्णाङ्क event_notify(गणना event_t type, शून्य *data, पूर्णाङ्क size)
{
	संरचना event event;
	पूर्णाङ्क ret;

	event.type = type;
	यदि (size > परिमाण(event.data))
		प्रत्यागम log_संदेश_ret("size", -E2BIG);
	स_नकल(&event.data, data, size);

	ret = notify_static(&event);
	यदि (ret)
		प्रत्यागम log_संदेश_ret("sta", ret);

	यदि (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		यदि (ret)
			प्रत्यागम log_संदेश_ret("dyn", ret);
	}

	प्रत्यागम 0;
}

पूर्णाङ्क event_notify_null(गणना event_t type)
{
	प्रत्यागम event_notify(type, रिक्त, 0);
}

शून्य event_show_spy_list(शून्य)
{
	संरचना evspy_info *start =
		ll_entry_start(संरचना evspy_info, evspy_info);
	ध्रुव पूर्णाङ्क n_ents = ll_entry_गिनती(संरचना evspy_info, evspy_info);
	संरचना evspy_info *spy;
	ध्रुव पूर्णाङ्क size = परिमाण(ulong) * 2;

	म_लिखो("Seq  %-24s  %*s  %s\n", "Type", size, "Function", "ID");
	प्रति (spy = start; spy != start + n_ents; spy++) {
		म_लिखो("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), size, spy->func,
		       event_spy_id(spy));
	}
}

#यदि CONFIG_IS_ENABLED(EVENT_DYNAMIC)
स्थिर शून्य spy_free(संरचना event_spy *spy)
{
	list_del(&spy->sibling_गाँठ);
}

पूर्णाङ्क event_register(ध्रुव वर्ण *id, गणना event_t type, event_handler_t func, शून्य *ctx)
{
	संरचना event_state *state = gd_event_state();
	संरचना event_spy *spy;

	spy = दो_स्मृति(परिमाण(*spy));
	यदि (!spy)
		प्रत्यागम log_संदेश_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_tail(&spy->sibling_गाँठ, &state->spy_head);

	प्रत्यागम 0;
}

पूर्णाङ्क event_uninit(शून्य)
{
	संरचना event_state *state = gd_event_state();
	संरचना event_spy *spy, *next;

	list_for_each_entry_safe(spy, next, &state->spy_head, sibling_गाँठ)
		spy_free(spy);

	प्रत्यागम 0;
}

पूर्णाङ्क event_init(शून्य)
{
	संरचना event_state *state = gd_event_state();

	INIT_LIST_HEAD(&state->spy_head);

	प्रत्यागम 0;
}
#endif /* EVENT_DYNAMIC */
