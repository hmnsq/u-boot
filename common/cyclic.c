// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#ضمن <cyclic.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <time.h>
#ضمن <linux/errno.h>
#ضمن <linux/list.h>
#ضمن <asm/global_data.h>
#ضمن <u-boot/schedule.h>
#ضمن <uthread.h>

DECLARE_GLOBAL_داده_PTR;

فراغ hw_watchdog_reset(فراغ);

ساكن bool cyclic_is_registered(ثابت بنية cyclic_info *cyclic)
{
	ثابت بنية cyclic_info *c;

	hlist_for_each_مدخل(c, &gd->cyclic_list, list) {
		إذا (c == cyclic)
			أرجع true;
	}

	أرجع false;
}

فراغ cyclic_ثبات(بنية cyclic_info *cyclic, cyclic_func_t func,
		     u64 delay_us, ثابت حرف *name)
{
	cyclic_unregister(cyclic);

	memset(cyclic, 0, حجم(*cyclic));

	/* Store values in struct */
	cyclic->func = func;
	cyclic->name = name;
	cyclic->delay_us = delay_us;
	cyclic->شروع_زمان_us = get_timer_us(0);
	hlist_جمع_کن_سر(&cyclic->list, &gd->cyclic_list);
}

فراغ cyclic_unregister(بنية cyclic_info *cyclic)
{
	إذا (!cyclic_is_registered(cyclic))
		أرجع;

	hlist_del(&cyclic->list);
}

ساكن فراغ cyclic_پیاپی(فراغ)
{
	بنية cyclic_info *cyclic;
	بنية hlist_گره *tmp;
	u64 now, after, cpu_زمان;

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
	إذا (hlist_خالی(&gd->cyclic_list))
		أرجع;

	/* Prevent recursion */
	إذا (gd->flags & GD_FLG_CYCLIC_RUNNING)
		أرجع;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	now = get_timer_us(0);
	hlist_for_each_مدخل_امن(cyclic, tmp, &gd->cyclic_list, list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		إذا (زمان_after_eq64(now, cyclic->بعدی_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->بعدی_call = now + cyclic->delay_us;
			cyclic->func(cyclic);
			after = get_timer_us(0);
			cyclic->پیاپی_cnt++;
			cpu_زمان = after - now;
			cyclic->cpu_زمان_us += cpu_زمان;
			now = after;

			/* Check if cpu-time exceeds max allowed time */
			إذا ((cpu_زمان > CONFIG_CYCLIC_MAX_CPU_زمان_US) &&
			    (!cyclic->already_warned)) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max\n",
				       cyclic->name, cpu_زمان,
				       CONFIG_CYCLIC_MAX_CPU_زمان_US);

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

فراغ زمانبندی(فراغ)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	إذا (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	إذا (gd)
		cyclic_پیاپی();

	uthread_زمانبندی();
}

صحيح cyclic_unregister_all(فراغ)
{
	بنية cyclic_info *cyclic;
	بنية hlist_گره *tmp;

	hlist_for_each_مدخل_امن(cyclic, tmp, &gd->cyclic_list, list)
		cyclic_unregister(cyclic);

	أرجع 0;
}
