// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <cyclic.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/list.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/schedule.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <uthread.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਖਾਲੀ hw_watchdog_reset(ਖਾਲੀ);

ਸਥਿਰ bool cyclic_is_registered(ਅਚਲ ਬਣਤਰ cyclic_info *cyclic)
{
	ਅਚਲ ਬਣਤਰ cyclic_info *c;

	hlist_for_each_ਇੰਦਰਾਜ(c, &gd->cyclic_list, list) {
		ਜੇ (c == cyclic)
			ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}

ਖਾਲੀ cyclic_ਰਜਿਸਟਰ(ਬਣਤਰ cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, ਅਚਲ ਅੱਖਰ *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, ਆਕਾਰ(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->ਸ਼ੁਰੂ_ਸਮਾਂ_us = get_timer_us(0);
	hlist_ਜੋੜੋ_ਸਿਰ(&cyclic->list, &gd->cyclic_list);
}

ਖਾਲੀ cyclic_unregister(ਬਣਤਰ cyclic_info *cyclic)
{
	ਜੇ (!cyclic_is_registered(cyclic))
		ਵਾਪਸ;

	hlist_del(&cyclic->list);
}

ਸਥਿਰ ਖਾਲੀ cyclic_ਲੜੀਵਾਰ(ਖਾਲੀ)
{
	ਬਣਤਰ cyclic_info *cyclic;
	ਬਣਤਰ hlist_ਗੰਢ *tmp;
	u64 now, after, cpu_ਸਮਾਂ;

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
	ਜੇ (hlist_ਖਾਲੀ_ਥਾਵਾਂ(&gd->cyclic_list))
		ਵਾਪਸ;

	/* Prevent recursion */
	ਜੇ (gd->flags & GD_FLG_CYCLIC_RUNNING)
		ਵਾਪਸ;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		ਜੇ (ਸਮਾਂ_after_eq64(now, cyclic->ਅਗਲਾ_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->ਅਗਲਾ_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->ਲੜੀਵਾਰ_cnt++;
			cpu_ਸਮਾਂ = after - now;
			cyclic->cpu_ਸਮਾਂ_us += cpu_ਸਮਾਂ;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			ਜੇ ((cpu_ਸਮਾਂ > CONFIG_CYCLIC_MAX_CPU_ਸਮਾਂ_US) &&
			    (!cyclic->already_warned)) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max\n",
				       cyclic->name, cpu_ਸਮਾਂ,
				       CONFIG_CYCLIC_MAX_CPU_ਸਮਾਂ_US);

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

ਖਾਲੀ ਸਮਾਂ_ਸਾਰਣੀ(ਖਾਲੀ)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	ਜੇ (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	ਜੇ (gd)
		cyclic_ਲੜੀਵਾਰ();

	uthread_ਸਮਾਂ_ਸਾਰਣੀ();
}

ਪੂਰਨ cyclic_unregister_all(ਖਾਲੀ)
{
	ਬਣਤਰ cyclic_info *cyclic;
	ਬਣਤਰ hlist_ਗੰਢ *tmp;

	hlist_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	ਵਾਪਸ 0;
}
