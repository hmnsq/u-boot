// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#అన్తర్భావ <cyclic.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <linux/errno.h>
#అన్తర్భావ <linux/list.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <u-boot/schedule.h>
#అన్తర్భావ <uthread.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

శూన్య hw_watchdog_reset(శూన్య);

స్థిర bool cyclic_is_registered(స్థిరరాశి నిర్మాణం cyclic_info *cyclic)
{
	స్థిరరాశి నిర్మాణం cyclic_info *c;

	hlist_for_each_నమోదు(c, &gd->cyclic_list, list) {
		ఒకవేళ (c == cyclic)
			తిరిగి true;
	}

	తిరిగి false;
}

శూన్య cyclic_రిజిస్టరు(నిర్మాణం cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, స్థిరరాశి అక్షరం *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, పరిమాణం(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->ప్రారంభం_సమయం_us = get_timer_us(0);
	hlist_కూడు_తల(&cyclic->list, &gd->cyclic_list);
}

శూన్య cyclic_unregister(నిర్మాణం cyclic_info *cyclic)
{
	ఒకవేళ (!cyclic_is_registered(cyclic))
		తిరిగి;

	hlist_del(&cyclic->list);
}

స్థిర శూన్య cyclic_నిరంతరం(శూన్య)
{
	నిర్మాణం cyclic_info *cyclic;
	నిర్మాణం hlist_కణుపు *tmp;
	u64 now, after, cpu_సమయం;

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
	ఒకవేళ (hlist_ఖాళీలు(&gd->cyclic_list))
		తిరిగి;

	/* Prevent recursion */
	ఒకవేళ (gd->flags & GD_FLG_CYCLIC_RUNNING)
		తిరిగి;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_నమోదు_సురక్షిత(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		ఒకవేళ (సమయం_after_eq64(now, cyclic->తదుపరి_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->తదుపరి_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->నిరంతరం_cnt++;
			cpu_సమయం = after - now;
			cyclic->cpu_సమయం_us += cpu_సమయం;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			ఒకవేళ ((cpu_సమయం > CONFIG_CYCLIC_MAX_CPU_సమయం_US) &&
			    (!cyclic->already_warned)) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max\n",
				       cyclic->name, cpu_సమయం,
				       CONFIG_CYCLIC_MAX_CPU_సమయం_US);

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

శూన్య కాలపట్టిక(శూన్య)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	ఒకవేళ (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	ఒకవేళ (gd)
		cyclic_నిరంతరం();

	uthread_కాలపట్టిక();
}

పూర్ణాంకం cyclic_unregister_all(శూన్య)
{
	నిర్మాణం cyclic_info *cyclic;
	నిర్మాణం hlist_కణుపు *tmp;

	hlist_for_each_నమోదు_సురక్షిత(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	తిరిగి 0;
}
