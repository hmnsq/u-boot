// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#ଅନ୍ତର୍ଭାଵ <cyclic.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>
#ଅନ୍ତର୍ଭାଵ <linux/list.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/schedule.h>
#ଅନ୍ତର୍ଭାଵ <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

ଶୂନ୍ୟ hw_watchdog_reset(ଶୂନ୍ୟ);

ସ୍ଥିର bool cyclic_is_registered(ଅଚଳ ସଂରଚନା cyclic_info *cyclic)
{
	ଅଚଳ ସଂରଚନା cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		ଯଦି (c == cyclic)
			ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
}

ଶୂନ୍ୟ cyclic_register(ସଂରଚନା cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, ଅଚଳ ଅକ୍ଷର *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, ଆକାର(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, &gd->cyclic_list);
}

ଶୂନ୍ୟ cyclic_unregister(ସଂରଚନା cyclic_info *cyclic)
{
	ଯଦି (!cyclic_is_registered(cyclic))
		ଫେରନ୍ତୁ;

	hlist_del(&cyclic->list);
}

ସ୍ଥିର ଶୂନ୍ୟ cyclic_run(ଶୂନ୍ୟ)
{
	ସଂରଚନା cyclic_info *cyclic;
	ସଂରଚନା hlist_ଗଣ୍ଠି *tmp;
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
	ଯଦି (hlist_empty(&gd->cyclic_list))
		ଫେରନ୍ତୁ;

	/* Prevent recursion */
	ଯଦି (gd->flags & GD_FLG_CYCLIC_RUNNING)
		ଫେରନ୍ତୁ;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		ଯଦି (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			ଯଦି ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
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

ଶୂନ୍ୟ schedule(ଶୂନ୍ୟ)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	ଯଦି (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	ଯଦି (gd)
		cyclic_run();

	uthread_schedule();
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cyclic_unregister_all(ଶୂନ୍ୟ)
{
	ସଂରଚନା cyclic_info *cyclic;
	ସଂରଚନା hlist_ଗଣ୍ଠି *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	ଫେରନ୍ତୁ 0;
}
