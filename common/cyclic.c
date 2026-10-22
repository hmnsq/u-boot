// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#ಅನ್ತರ್ಭಾವ <cyclic.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>
#ಅನ್ತರ್ಭಾವ <linux/list.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <u-boot/schedule.h>
#ಅನ್ತರ್ಭಾವ <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

ಶೂನ್ಯ hw_watchdog_reset(ಶೂನ್ಯ);

ಸ್ಥಿರ bool cyclic_is_registered(ಧ್ರುವ ಸಂರಚನಾ cyclic_info *cyclic)
{
	ಧ್ರುವ ಸಂರಚನಾ cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		ಯದಿ (c == cyclic)
			ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

ಶೂನ್ಯ cyclic_register(ಸಂರಚನಾ cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, ಧ್ರುವ ವರ್ಣ *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, ಪರಿಮಾಣ(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, &gd->cyclic_list);
}

ಶೂನ್ಯ cyclic_unregister(ಸಂರಚನಾ cyclic_info *cyclic)
{
	ಯದಿ (!cyclic_is_registered(cyclic))
		ಪ್ರತ್ಯಾಗಮ;

	hlist_del(&cyclic->list);
}

ಸ್ಥಿರ ಶೂನ್ಯ cyclic_run(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ cyclic_info *cyclic;
	ಸಂರಚನಾ hlist_ಗಂಟು *tmp;
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
	ಯದಿ (hlist_empty(&gd->cyclic_list))
		ಪ್ರತ್ಯಾಗಮ;

	/* Prevent recursion */
	ಯದಿ (gd->flags & GD_FLG_CYCLIC_RUNNING)
		ಪ್ರತ್ಯಾಗಮ;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		ಯದಿ (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			ಯದಿ ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
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

ಶೂನ್ಯ schedule(ಶೂನ್ಯ)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	ಯದಿ (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	ಯದಿ (gd)
		cyclic_run();

	uthread_schedule();
}

ಪೂರ್ಣಾಙ್ಕ cyclic_unregister_all(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ cyclic_info *cyclic;
	ಸಂರಚನಾ hlist_ಗಂಟು *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	ಪ್ರತ್ಯಾಗಮ 0;
}
