// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#അന്തര്ഭാവ <cyclic.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <linux/errno.h>
#അന്തര്ഭാവ <linux/list.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <u-boot/schedule.h>
#അന്തര്ഭാവ <uthread.h>

DECLARE_GLOBAL_DATA_PTR;

ശൂന്യം hw_watchdog_reset(ശൂന്യം);

സ്ഥിതം bool cyclic_is_registered(സ്ഥിരം ഘടന cyclic_info *cyclic)
{
	സ്ഥിരം ഘടന cyclic_info *c;

	hlist_for_each_entry(c, &gd->cyclic_list, list) {
		എങ്കിൽ (c == cyclic)
			തിരികെ true;
	}

	തിരികെ false;
}

ശൂന്യം cyclic_register(ഘടന cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, സ്ഥിരം അക്ഷരം *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, വലുപ്പം(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = get_timer_us(0);
	hlist_add_head(&cyclic->list, &gd->cyclic_list);
}

ശൂന്യം cyclic_unregister(ഘടന cyclic_info *cyclic)
{
	എങ്കിൽ (!cyclic_is_registered(cyclic))
		തിരികെ;

	hlist_del(&cyclic->list);
}

സ്ഥിതം ശൂന്യം cyclic_run(ശൂന്യം)
{
	ഘടന cyclic_info *cyclic;
	ഘടന hlist_കണ്ണി *tmp;
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
	എങ്കിൽ (hlist_empty(&gd->cyclic_list))
		തിരികെ;

	/* Prevent recursion */
	എങ്കിൽ (gd->flags & GD_FLG_CYCLIC_RUNNING)
		തിരികെ;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		എങ്കിൽ (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->run_cnt++;
			cpu_time = after - now;
			cyclic->cpu_time_us += cpu_time;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			എങ്കിൽ ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
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

ശൂന്യം schedule(ശൂന്യം)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	എങ്കിൽ (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	എങ്കിൽ (gd)
		cyclic_run();

	uthread_schedule();
}

പൂർണ്ണസംഖ്യ cyclic_unregister_all(ശൂന്യം)
{
	ഘടന cyclic_info *cyclic;
	ഘടന hlist_കണ്ണി *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	തിരികെ 0;
}
