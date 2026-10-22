// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <ansi.h>
#అన్తర్భావ <autoboot.h>
#అన్తర్భావ <bootretry.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fdtdec.h>
#అన్తర్భావ <hash.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <memalign.h>
#అన్తర్భావ <menu.h>
#అన్తర్భావ <post.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <linux/delay.h>
#అన్తర్భావ <u-boot/sha256.h>
#అన్తర్భావ <bootcount.h>
#అన్తర్భావ <crypt.h>
#అన్తర్భావ <dm/ofnode.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

#నిర్వచించు DELAY_STOP_STR_MAX_నిడివి 64

#నిర్వచితం_కాకపోతే DEBUG_BOOTKEYS
#నిర్వచించు DEBUG_BOOTKEYS 0
#అయితే_ముగింపు
#నిర్వచించు debug_bootkeys(fmt, వాదనలు...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
స్థిర పూర్ణాంకం stored_bootdelay;
స్థిర పూర్ణాంకం menukey;

#ఒకవేళ defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#నిర్వచించు AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#లేకపోతే
#నిర్వచించు AUTOBOOT_STOP_STR_CRYPT	""
#అయితే_ముగింపు
#ఒకవేళ defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#నిర్వచించు AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#లేకపోతే
#నిర్వచించు AUTOBOOT_STOP_STR_SHA256	""
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_AUTOBOOT_USE_MENUKEY
#నిర్వచించు AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#లేకపోతే
#నిర్వచించు AUTOBOOT_MENUKEY 0
#అయితే_ముగింపు

/**
 * passwd_abort_crypt() - check for a crypt-style hashed key sequence to abort booting
 *
 * This checks for the user entering a password within a given time.
 *
 * The entered password is hashed via one of the crypt-style hash methods
 * and compared to the pre-defined value from either
 *   the environment variable "bootstopkeycrypt"
 * or
 *   the config value CONFIG_AUTOBOOT_STOP_STR_CRYPT
 *
 * In case the config value CONFIG_AUTOBOOT_NEVER_TIMEOUT has been enabled
 * this function never times out if the user presses the <Enter> key
 * before starting to enter the password.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
స్థిర పూర్ణాంకం passwd_abort_crypt(uint64_t etime)
{
	స్థిరరాశి అక్షరం *crypt_env_str = env_get("bootstopkeycrypt");
	అక్షరం presskey[DELAY_STOP_STR_MAX_నిడివి];
	u_int presskey_len = 0;
	పూర్ణాంకం abort = 0;
	పూర్ణాంకం never_timeout = 0;
	పూర్ణాంకం err;

	ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	ఒకవేళ (!crypt_env_str)
		తిరిగి 0;

	/* We expect the stop-string to be newline-terminated */
	చేయి {
		ఒకవేళ (tstc()) {
			/* Check for input string overflow */
			ఒకవేళ (presskey_len >= పరిమాణం(presskey))
				తిరిగి 0;

			presskey[presskey_len] = getchar();

			ఒకవేళ ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					కొనసాగు;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				ఒకవేళ (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				ఆపు;
			} లేకపోతే {
				presskey_len++;
			}
		}
		udelay(10000);
	} అయితే (never_timeout || get_ticks() <= etime);

	తిరిగి abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
స్థిర పూర్ణాంకం slow_equals(u8 *a, u8 *b, పూర్ణాంకం len)
{
	పూర్ణాంకం diff = 0;
	పూర్ణాంకం i;

	కొరకు (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	తిరిగి diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
స్థిర పూర్ణాంకం passwd_abort_sha256(uint64_t etime)
{
	స్థిరరాశి అక్షరం *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_కూడిక_LEN];
	u8 *sha;
	అక్షరం *presskey;
	అక్షరం *c;
	స్థిరరాశి అక్షరం *algo_name = "sha256";
	u_int presskey_len = 0;
	పూర్ణాంకం abort = 0;
	పూర్ణాంకం కొలత = పరిమాణం(sha);
	పూర్ణాంకం ret;

	ఒకవేళ (sha_env_str == ఖాళీ)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_నిడివి);
	ఒకవేళ (!presskey)
		తిరిగి -ENOMEM;

	c = అక్షరశ్రేణి_శోధన(sha_env_str, ":");
	ఒకవేళ (c && (c - sha_env_str < DELAY_STOP_STR_MAX_నిడివి)) {
		/* preload presskey with salt */
		జ్ఞాపకం_నకలు(presskey, sha_env_str, c - sha_env_str);
		presskey_len = c - sha_env_str;
		sha_env_str = c + 1;
	}
	/*
	 * Generate the binary value from the environment hash value
	 * so that we can compare this value with the computed hash
	 * from the user input
	 */
	ret = hash_parse_string(algo_name, sha_env_str, sha_env);
	ఒకవేళ (ret) {
		రాయి("Hash %s not supported!\n", algo_name);
		విడుదల(presskey);
		తిరిగి 0;
	}

	sha = malloc_cache_aligned(SHA256_కూడిక_LEN);
	ఒకవేళ (!sha) {
		విడుదల(presskey);
		తిరిగి -ENOMEM;
	}
	కొలత = SHA256_కూడిక_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	చేయి {
		ఒకవేళ (tstc()) {
			/* Check for input string overflow */
			ఒకవేళ (presskey_len >= DELAY_STOP_STR_MAX_నిడివి) {
				విడుదల(presskey);
				విడుదల(sha);
				తిరిగి 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_ఖండం(algo_name, (స్థిరరాశి శూన్య *)presskey,
				   presskey_len, sha, &కొలత);

			/* And check if sha matches saved value in env */
			ఒకవేళ (slow_equals(sha, sha_env, SHA256_కూడిక_LEN))
				abort = 1;
		}
		udelay(10000);
	} అయితే (!abort && get_ticks() <= etime);

	విడుదల(presskey);
	విడుదల(sha);
	తిరిగి abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
స్థిర పూర్ణాంకం passwd_abort_కీలకం(uint64_t etime)
{
	పూర్ణాంకం abort = 0;
	నిర్మాణం {
		అక్షరం *str;
		u_int len;
		పూర్ణాంకం retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	అక్షరం presskey[DELAY_STOP_STR_MAX_నిడివి];
	పూర్ణాంకం presskey_len = 0;
	పూర్ణాంకం presskey_max = 0;
	పూర్ణాంకం i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	ఒకవేళ (delaykey[0].str == ఖాళీ)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	ఒకవేళ (delaykey[1].str == ఖాళీ)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	కొరకు (i = 0; i < పరిమాణం(delaykey) / పరిమాణం(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ఖాళీ ?
				    0 : అక్షరశ్రేణి_పొడవు(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_నిడివి ?
				    DELAY_STOP_STR_MAX_నిడివి : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	చేయి {
		ఒకవేళ (tstc()) {
			ఒకవేళ (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} లేకపోతే {
				కొరకు (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		కొరకు (i = 0; i < పరిమాణం(delaykey) / పరిమాణం(delaykey[0]); i++) {
			ఒకవేళ (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				ఒకవేళ (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} అయితే (!abort && get_ticks() <= etime);

	తిరిగి abort;
}

/**
 * fallback_to_sha256() - check whether we should fall back to sha256
 *                        password checking
 *
 * This checks for the environment variable `bootstopusesha256` in case
 * sha256-fallback has been enabled via the config setting
 * `AUTOBOOT_SHA256_FALLBACK`.
 *
 * Return: `false` if we must not fall-back, `true` if plain sha256 should be tried
 */
స్థిర bool fallback_గమ్యం_sha256(శూన్య)
{
	ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		తిరిగి env_get_yesno("bootstopusesha256") == 1;
	లేకపోతే ఒకవేళ (IS_ENABLED(CONFIG_CRYPT_PW))
		తిరిగి false;
	లేకపోతే
		తిరిగి true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
స్థిర పూర్ణాంకం abortboot_కీలకం_sequence(పూర్ణాంకం bootdelay)
{
	పూర్ణాంకం abort;
	uint64_t etime = endtick(bootdelay);

	ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	రాయి(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		ఒకవేళ (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_గమ్యం_sha256())
			abort = passwd_abort_crypt(etime);
		లేకపోతే
			abort = passwd_abort_sha256(etime);
	} లేకపోతే {
		abort = passwd_abort_కీలకం(etime);
	}
	ఒకవేళ (!abort)
		debug_bootkeys("key timeout\n");

	తిరిగి abort;
}

స్థిర శూన్య print_boot_delay(పూర్ణాంకం bootdelay)
{
	రాయి(ANSI_CLEAR_పంక్తి "\rHit any key to stop autoboot: %d", bootdelay);
}

స్థిర పూర్ణాంకం abortboot_single_కీలకం(పూర్ణాంకం bootdelay)
{
	పూర్ణాంకం abort = 0;
	గుర్తులేని దీర్ఘ ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	ఒకవేళ (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	అయితే ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		చేయి {
			ఒకవేళ (tstc()) {	/* we got a key press	*/
				పూర్ణాంకం కీలకం;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				కీలకం = getchar();/* consume input	*/
				ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = కీలకం;
				ఆపు;
			}
			udelay(10000);
		} అయితే (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	తిరిగి abort;
}

స్థిర పూర్ణాంకం abortboot(పూర్ణాంకం bootdelay)
{
	పూర్ణాంకం abort = 0;

	ఒకవేళ (bootdelay >= 0) {
		ఒకవేళ (autoboot_keyed())
			abort = abortboot_కీలకం_sequence(bootdelay);
		లేకపోతే
			abort = abortboot_single_కీలకం(bootdelay);
	}

	ఒకవేళ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	తిరిగి abort;
}

స్థిర శూన్య process_fdt_options(శూన్య)
{
#నిర్వచితమైతే CONFIG_పాఠ్యం_పునాది
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	ఒకవేళ (addr)
		env_సమితి_addr("kernaddr", (శూన్య *)(CONFIG_పాఠ్యం_పునాది + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	ఒకవేళ (addr)
		env_సమితి_addr("rootaddr", (శూన్య *)(CONFIG_పాఠ్యం_పునాది + addr));
#అయితే_ముగింపు /* CONFIG_TEXT_BASE */
}

స్థిరరాశి అక్షరం *bootdelay_process(శూన్య)
{
	అక్షరం *s;
	పూర్ణాంకం bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (పూర్ణాంకం)simple_strtol(s, ఖాళీ, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_యొక్క_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#నిర్వచితమైతే CONFIG_POST
	ఒకవేళ (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} లేకపోతే
#అయితే_ముగింపు /* CONFIG_POST */
	ఒకవేళ (bootcount_error())
		s = env_get("altbootcmd");
	లేకపోతే
		s = env_get("bootcmd");

	ఒకవేళ (IS_ENABLED(CONFIG_యొక్క_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	తిరిగి s;
}

శూన్య autoboot_command(స్థిరరాశి అక్షరం *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	ఒకవేళ (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool తాళం;
		పూర్ణాంకం prev;

		తాళం = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		ఒకవేళ (తాళం)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		నిరంతరం_command_list(s, -1, 0);

		ఒకవేళ (తాళం)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		ఒకవేళ (s)
			నిరంతరం_command_list(s, -1, 0);
	}
}
