// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#समावेश <cyclic.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <time.h>
#समावेश <linux/errno.h>
#समावेश <linux/list.h>
#समावेश <asm/global_data.h>
#समावेश <u-boot/schedule.h>
#समावेश <uthread.h>

DECLARE_GLOBAL_तथ्य_PTR;

व्योम hw_watchdog_reset(व्योम);

अटल bool cyclic_is_registered(स्थिर काष्ठा cyclic_info *cyclic)
{
	स्थिर काष्ठा cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		अगर (c == cyclic)
			वापस true;
	}

	वापस false;
}

व्योम cyclic_register(काष्ठा cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, स्थिर अक्षर *name)
{
	cyclic_unregister(cyclic);

	स_रखो(cyclic, 0, माप(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_शीर्ष(&cyclic->list, &gd->cyclic_list);
}

व्योम cyclic_unregister(काष्ठा cyclic_info *cyclic)
{
	अगर (!cyclic_is_registered(cyclic))
		वापस;

	hlist_del(&cyclic->list);
}

अटल व्योम cyclic_run(व्योम)
{
	काष्ठा cyclic_info *cyclic;
	काष्ठा hlist_गाँठ *tmp;
	u64 now, after, cpu_time;

	/*
	 * Nothing to do if the list is empty. Also, schedule() can be
	 * called before timer infrastructure is ready, in which case
	 * calling get_timer_us() before the (empty) loop could cause
	 * a divide-by-0 or otherwise crash the system. No clients
	 * should be registered before the timer infrastructure is up,
	 * so the check for the list being empty should be
	 * ok. Otherwise, we would need a new GD_FLG_TIMERS_READY
	 * flag.
	 */
	अगर (hlist_empty(&gd->cyclic_list))
		वापस;

	/* Prevent recursion */
	अगर (gd->flags & GD_FLG_CYCLIC_RUNNING)
		वापस;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		अगर (time_after_eq64(now, cyclic->अग्र_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->अग्र_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			अगर ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
			    (!cyclic->already_warned)) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max\n",
				       cyclic->name, cpu_time,
				       CONFIG_CYCLIC_MAX_CPU_TIME_US);

				/*
				 * Don't disable this function, just warn once
				 * about this exceeding CPU time usage
				 */
				cyclic->already_warned = true;
			}
		}
	}
	gd->flags &= ~GD_FLG_CYCLIC_RUNNING;
}

व्योम schedule(व्योम)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	अगर (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	अगर (gd)
		cyclic_run();

	uthread_schedule();
}

पूर्णांक cyclic_unregister_all(व्योम)
{
	काष्ठा cyclic_info *cyclic;
	काष्ठा hlist_गाँठ *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	वापस 0;
}
