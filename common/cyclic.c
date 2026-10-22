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

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಶೂನ್ಯ hw_watchdog_reset(ಶೂನ್ಯ);

ಸ್ಥಿರ bool cyclic_is_registered(ಸ್ಥಿರಾಂಕ ರಚನೆ cyclic_info *cyclic)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ cyclic_info *c;

	hlist_for_each_ನಮೂದು(c, &gd->cyclic_list, list) {
		ಆದರೆ (c == cyclic)
			ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}

ಶೂನ್ಯ cyclic_ರಿಜಿಸ್ಟರ್(ರಚನೆ cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, ಗಾತ್ರ(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->ಆರಂಭ_ಸಮಯ_us = get_timer_us(0);
	hlist_ಕೂಡಿಸು_ತಲೆ(&cyclic->list, &gd->cyclic_list);
}

ಶೂನ್ಯ cyclic_unregister(ರಚನೆ cyclic_info *cyclic)
{
	ಆದರೆ (!cyclic_is_registered(cyclic))
		ಹಿಂತಿರುಗಿ;

	hlist_del(&cyclic->list);
}

ಸ್ಥಿರ ಶೂನ್ಯ cyclic_ಸತತ(ಶೂನ್ಯ)
{
	ರಚನೆ cyclic_info *cyclic;
	ರಚನೆ hlist_ಗಂಟು *tmp;
	u64 now, after, cpu_ಸಮಯ;

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
	ಆದರೆ (hlist_ಖಾಲಿಗಳು(&gd->cyclic_list))
		ಹಿಂತಿರುಗಿ;

	/* Prevent recursion */
	ಆದರೆ (gd->flags & GD_FLG_CYCLIC_RUNNING)
		ಹಿಂತಿರುಗಿ;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		ಆದರೆ (ಸಮಯ_after_eq64(now, cyclic->ಮುಂದಿನ_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->ಮುಂದಿನ_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->ಸತತ_cnt++;
			cpu_ಸಮಯ = after - now;
			cyclic->cpu_ಸಮಯ_us += cpu_ಸಮಯ;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			ಆದರೆ ((cpu_ಸಮಯ > CONFIG_CYCLIC_MAX_CPU_ಸಮಯ_US) &&
			    (!cyclic->already_warned)) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max\n",
				       cyclic->name, cpu_ಸಮಯ,
				       CONFIG_CYCLIC_MAX_CPU_ಸಮಯ_US);

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

ಶೂನ್ಯ ವೇಳಾಪಟ್ಟಿ(ಶೂನ್ಯ)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	ಆದರೆ (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	ಆದರೆ (gd)
		cyclic_ಸತತ();

	uthread_ವೇಳಾಪಟ್ಟಿ();
}

ಪೂರ್ಣಾಂಕ cyclic_unregister_all(ಶೂನ್ಯ)
{
	ರಚನೆ cyclic_info *cyclic;
	ರಚನೆ hlist_ಗಂಟು *tmp;

	hlist_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	ಹಿಂತಿರುಗಿ 0;
}
