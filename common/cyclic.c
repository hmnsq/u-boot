// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#شامل <cyclic.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <time.h>
#شامل <linux/errno.h>
#شامل <linux/list.h>
#شامل <asm/global_data.h>
#شامل <u-boot/schedule.h>
#شامل <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

تهی hw_watchdog_reset(تهی);

ایستا bool cyclic_is_registered(ثابت ساختار cyclic_info *cyclic)
{
	ثابت ساختار cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		اگر (c == cyclic)
			بازگشت true;
	}

	بازگشت false;
}

تهی cyclic_register(ساختار cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, ثابت نویسه *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, اندازه(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, &gd->cyclic_list);
}

تهی cyclic_unregister(ساختار cyclic_info *cyclic)
{
	اگر (!cyclic_is_registered(cyclic))
		بازگشت;

	hlist_del(&cyclic->list);
}

ایستا تهی cyclic_run(تهی)
{
	ساختار cyclic_info *cyclic;
	ساختار hlist_گره *tmp;
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
	اگر (hlist_empty(&gd->cyclic_list))
		بازگشت;

	/* Prevent recursion */
	اگر (gd->flags & GD_FLG_CYCLIC_RUNNING)
		بازگشت;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		اگر (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			اگر ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
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

تهی schedule(تهی)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	اگر (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	اگر (gd)
		cyclic_run();

	uthread_schedule();
}

صحیح cyclic_unregister_all(تهی)
{
	ساختار cyclic_info *cyclic;
	ساختار hlist_گره *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	بازگشت 0;
}
