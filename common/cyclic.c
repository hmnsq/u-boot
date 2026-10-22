// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#অন্তর্ভাব <cyclic.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <linux/errno.h>
#অন্তর্ভাব <linux/list.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <u-boot/schedule.h>
#অন্তর্ভাব <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

শূন্য hw_watchdog_reset(শূন্য);

স্থির bool cyclic_is_registered(ধ্রুব সংরচনা cyclic_info *cyclic)
{
	ধ্রুব সংরচনা cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		যদি (c == cyclic)
			প্রত্যাগম true;
	}

	প্রত্যাগম false;
}

শূন্য cyclic_register(সংরচনা cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, ধ্রুব বর্ণ *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, পরিমাণ(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, &gd->cyclic_list);
}

শূন্য cyclic_unregister(সংরচনা cyclic_info *cyclic)
{
	যদি (!cyclic_is_registered(cyclic))
		প্রত্যাগম;

	hlist_del(&cyclic->list);
}

স্থির শূন্য cyclic_run(শূন্য)
{
	সংরচনা cyclic_info *cyclic;
	সংরচনা hlist_গ্রন্থি *tmp;
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
	যদি (hlist_empty(&gd->cyclic_list))
		প্রত্যাগম;

	/* Prevent recursion */
	যদি (gd->flags & GD_FLG_CYCLIC_RUNNING)
		প্রত্যাগম;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		যদি (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			যদি ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
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

শূন্য schedule(শূন্য)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	যদি (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	যদি (gd)
		cyclic_run();

	uthread_schedule();
}

পূর্ণাঙ্ক cyclic_unregister_all(শূন্য)
{
	সংরচনা cyclic_info *cyclic;
	সংরচনা hlist_গ্রন্থি *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	প্রত্যাগম 0;
}
