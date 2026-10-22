// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#અન્તર્ભાવ <cyclic.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <linux/errno.h>
#અન્તર્ભાવ <linux/list.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <u-boot/schedule.h>
#અન્તર્ભાવ <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

શૂન્ય hw_watchdog_reset(શૂન્ય);

સ્થિર bool cyclic_is_registered(અચલ સંરચના cyclic_info *cyclic)
{
	અચલ સંરચના cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		જો (c == cyclic)
			પાછા true;
	}

	પાછા false;
}

શૂન્ય cyclic_register(સંરચના cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, અચલ અક્ષર *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, કદ(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, &gd->cyclic_list);
}

શૂન્ય cyclic_unregister(સંરચના cyclic_info *cyclic)
{
	જો (!cyclic_is_registered(cyclic))
		પાછા;

	hlist_del(&cyclic->list);
}

સ્થિર શૂન્ય cyclic_run(શૂન્ય)
{
	સંરચના cyclic_info *cyclic;
	સંરચના hlist_ગાંઠ *tmp;
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
	જો (hlist_empty(&gd->cyclic_list))
		પાછા;

	/* Prevent recursion */
	જો (gd->flags & GD_FLG_CYCLIC_RUNNING)
		પાછા;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		જો (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			જો ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
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

શૂન્ય schedule(શૂન્ય)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	જો (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	જો (gd)
		cyclic_run();

	uthread_schedule();
}

પૂર્ણાંક cyclic_unregister_all(શૂન્ય)
{
	સંરચના cyclic_info *cyclic;
	સંરચના hlist_ગાંઠ *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	પાછા 0;
}
