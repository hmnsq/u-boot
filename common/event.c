// SPDX-License-Identifier: GPL-2.0+
/*
 * Events provide a general-purpose way to react to / subscribe to changes
 * within U-Boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#घोषणा LOG_CATEGORY	LOGC_EVENT

#समावेश <event.h>
#समावेश <event_internal.h>
#समावेश <log.h>
#समावेश <linker_lists.h>
#समावेश <malloc.h>
#समावेश <asm/global_data.h>
#समावेश <linux/errno.h>
#समावेश <linux/list.h>
#समावेश <relocate.h>

DECLARE_GLOBAL_तथ्य_PTR;

#अगर CONFIG_IS_ENABLED(EVENT_DEBUG)
स्थिर अक्षर *स्थिर type_name[] = {
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

_Static_assert(ARRAY_आकार(type_name) == EVT_गिनती, "event type_name size");
#पूर्ण_अगर

स्थिर अक्षर *event_type_name(क्रमागत event_t type)
{
#अगर CONFIG_IS_ENABLED(EVENT_DEBUG)
	अगर (type < ARRAY_आकार(type_name))
		वापस type_name[type];
	अन्यथा
		वापस "(unknown)";
#अन्यथा
	वापस "(unknown)";
#पूर्ण_अगर
}

अटल पूर्णांक notify_static(काष्ठा event *ev)
{
	काष्ठा evspy_info *start =
		ll_entry_start(काष्ठा evspy_info, evspy_info);
	स्थिर पूर्णांक n_ents = ll_entry_गिनती(काष्ठा evspy_info, evspy_info);
	काष्ठा evspy_info *spy;

	क्रम (spy = start; spy != start + n_ents; spy++) {
		अगर (spy->type == ev->type) {
			पूर्णांक ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), event_spy_id(spy));
			अगर (spy->flags & EVSPYF_SIMPLE) {
				स्थिर काष्ठा evspy_info_simple *simple;

				simple = (काष्ठा evspy_info_simple *)spy;
				ret = simple->func();
			} अन्यथा {
				ret = spy->func(शून्य, ev);
			}

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			अगर (ret)
				वापस log_संदेश_ret("spy", ret);
		}
	}

	वापस 0;
}

अटल पूर्णांक notify_dynamic(काष्ठा event *ev)
{
	काष्ठा event_state *state = gd_event_state();
	काष्ठा event_spy *spy, *अग्र;

	list_for_each_entry_safe(spy, अग्र, &state->spy_शीर्ष, sibling_गाँठ) {
		अगर (spy->type == ev->type) {
			पूर्णांक ret;

			log_debug("Sending event %x/%s to spy '%s'\n", ev->type,
				  event_type_name(ev->type), spy->id);
			ret = spy->func(spy->ctx, ev);

			/*
			 * TODO: Handle various return codes to
			 *
			 * - claim an event (no others will see it)
			 * - return an error from the event
			 */
			अगर (ret)
				वापस log_संदेश_ret("spy", ret);
		}
	}

	वापस 0;
}

पूर्णांक event_notify(क्रमागत event_t type, व्योम *तथ्य, पूर्णांक आकार)
{
	काष्ठा event event;
	पूर्णांक ret;

	event.type = type;
	अगर (आकार > माप(event.तथ्य))
		वापस log_संदेश_ret("size", -E2BIG);
	स_नकल(&event.तथ्य, तथ्य, आकार);

	ret = notify_static(&event);
	अगर (ret)
		वापस log_संदेश_ret("sta", ret);

	अगर (CONFIG_IS_ENABLED(EVENT_DYNAMIC)) {
		ret = notify_dynamic(&event);
		अगर (ret)
			वापस log_संदेश_ret("dyn", ret);
	}

	वापस 0;
}

पूर्णांक event_notify_null(क्रमागत event_t type)
{
	वापस event_notify(type, शून्य, 0);
}

व्योम event_show_spy_list(व्योम)
{
	काष्ठा evspy_info *start =
		ll_entry_start(काष्ठा evspy_info, evspy_info);
	स्थिर पूर्णांक n_ents = ll_entry_गिनती(काष्ठा evspy_info, evspy_info);
	काष्ठा evspy_info *spy;
	स्थिर पूर्णांक आकार = माप(ulong) * 2;

	म_लिखो("Seq  %-24s  %*s  %s\n", "Type", आकार, "Function", "ID");
	क्रम (spy = start; spy != start + n_ents; spy++) {
		म_लिखो("%3x  %-3x %-20s  %*p  %s\n", (uint)(spy - start),
		       spy->type, event_type_name(spy->type), आकार, spy->func,
		       event_spy_id(spy));
	}
}

#अगर CONFIG_IS_ENABLED(EVENT_DYNAMIC)
अटल व्योम spy_free(काष्ठा event_spy *spy)
{
	list_del(&spy->sibling_गाँठ);
}

पूर्णांक event_register(स्थिर अक्षर *id, क्रमागत event_t type, event_handler_t func, व्योम *ctx)
{
	काष्ठा event_state *state = gd_event_state();
	काष्ठा event_spy *spy;

	spy = दो_स्मृति(माप(*spy));
	अगर (!spy)
		वापस log_संदेश_ret("alloc", -ENOMEM);

	spy->id = id;
	spy->type = type;
	spy->func = func;
	spy->ctx = ctx;
	list_add_पुच्छ(&spy->sibling_गाँठ, &state->spy_शीर्ष);

	वापस 0;
}

पूर्णांक event_uninit(व्योम)
{
	काष्ठा event_state *state = gd_event_state();
	काष्ठा event_spy *spy, *अग्र;

	list_for_each_entry_safe(spy, अग्र, &state->spy_शीर्ष, sibling_गाँठ)
		spy_free(spy);

	वापस 0;
}

पूर्णांक event_init(व्योम)
{
	काष्ठा event_state *state = gd_event_state();

	INIT_LIST_शीर्ष(&state->spy_शीर्ष);

	वापस 0;
}
#पूर्ण_अगर /* EVENT_DYNAMIC */
