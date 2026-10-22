// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#அந்தர்பாவ <cyclic.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <linux/errno.h>
#அந்தர்பாவ <linux/list.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <u-boot/schedule.h>
#அந்தர்பாவ <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

வெறுமை hw_watchdog_reset(வெறுமை);

நிலையான bool cyclic_is_registered(மாறிலி கட்டமைப்பு cyclic_info *cyclic)
{
	மாறிலி கட்டமைப்பு cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		எனில் (c == cyclic)
			திரும்பு true;
	}

	திரும்பு false;
}

வெறுமை cyclic_register(கட்டமைப்பு cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, மாறிலி எழுத்து *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, அளவு(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, &gd->cyclic_list);
}

வெறுமை cyclic_unregister(கட்டமைப்பு cyclic_info *cyclic)
{
	எனில் (!cyclic_is_registered(cyclic))
		திரும்பு;

	hlist_del(&cyclic->list);
}

நிலையான வெறுமை cyclic_run(வெறுமை)
{
	கட்டமைப்பு cyclic_info *cyclic;
	கட்டமைப்பு hlist_முனை *tmp;
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
	எனில் (hlist_empty(&gd->cyclic_list))
		திரும்பு;

	/* Prevent recursion */
	எனில் (gd->flags & GD_FLG_CYCLIC_RUNNING)
		திரும்பு;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		எனில் (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			எனில் ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
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

வெறுமை schedule(வெறுமை)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	எனில் (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	எனில் (gd)
		cyclic_run();

	uthread_schedule();
}

முழு cyclic_unregister_all(வெறுமை)
{
	கட்டமைப்பு cyclic_info *cyclic;
	கட்டமைப்பு hlist_முனை *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	திரும்பு 0;
}
