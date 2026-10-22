// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <ansi.h>
#अन्तर्भाव <autoboot.h>
#अन्तर्भाव <bootretry.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <hash.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <memalign.h>
#अन्तर्भाव <menu.h>
#अन्तर्भाव <post.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/delay.h>
#अन्तर्भाव <u-boot/sha256.h>
#अन्तर्भाव <bootcount.h>
#अन्तर्भाव <crypt.h>
#अन्तर्भाव <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
स्थिर पूर्णाङ्क stored_bootdelay;
स्थिर पूर्णाङ्क menukey;

#यदि defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#अन्यथा
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#यदि defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#अन्यथा
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#अन्यथा
#define AUTOBOOT_MENUKEY 0
#endif

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
स्थिर पूर्णाङ्क passwd_abort_crypt(uint64_t etime)
{
	ध्रुव वर्ण *crypt_env_str = env_get("bootstopkeycrypt");
	वर्ण presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	पूर्णाङ्क abort = 0;
	पूर्णाङ्क never_timeout = 0;
	पूर्णाङ्क err;

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	यदि (!crypt_env_str)
		प्रत्यागम 0;

	/* We expect the stop-string to be newline-terminated */
	कुरु {
		यदि (tstc()) {
			/* Check for input string overflow */
			यदि (presskey_len >= परिमाण(presskey))
				प्रत्यागम 0;

			presskey[presskey_len] = getchar();

			यदि ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				यदि (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					अनुवर्त;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				यदि (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				भङ्ग;
			} अन्यथा {
				presskey_len++;
			}
		}
		udelay(10000);
	} यावत् (never_timeout || get_ticks() <= etime);

	प्रत्यागम abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
स्थिर पूर्णाङ्क slow_equals(u8 *a, u8 *b, पूर्णाङ्क len)
{
	पूर्णाङ्क diff = 0;
	पूर्णाङ्क i;

	प्रति (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	प्रत्यागम diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
स्थिर पूर्णाङ्क passwd_abort_sha256(uint64_t etime)
{
	ध्रुव वर्ण *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	वर्ण *presskey;
	वर्ण *c;
	ध्रुव वर्ण *algo_name = "sha256";
	u_int presskey_len = 0;
	पूर्णाङ्क abort = 0;
	पूर्णाङ्क size = परिमाण(sha);
	पूर्णाङ्क ret;

	यदि (sha_env_str == रिक्त)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	यदि (!presskey)
		प्रत्यागम -ENOMEM;

	c = strstr(sha_env_str, ":");
	यदि (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
		/* preload presskey with salt */
		memcpy(presskey, sha_env_str, c - sha_env_str);
		presskey_len = c - sha_env_str;
		sha_env_str = c + 1;
	}
	/*
	 * Generate the binary value from the environment hash value
	 * so that we can compare this value with the computed hash
	 * from the user input
	 */
	ret = hash_parse_string(algo_name, sha_env_str, sha_env);
	यदि (ret) {
		printf("Hash %s not supported!\n", algo_name);
		मुक्त(presskey);
		प्रत्यागम 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	यदि (!sha) {
		मुक्त(presskey);
		प्रत्यागम -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	कुरु {
		यदि (tstc()) {
			/* Check for input string overflow */
			यदि (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				मुक्त(presskey);
				मुक्त(sha);
				प्रत्यागम 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (ध्रुव शून्य *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			यदि (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} यावत् (!abort && get_ticks() <= etime);

	मुक्त(presskey);
	मुक्त(sha);
	प्रत्यागम abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
स्थिर पूर्णाङ्क passwd_abort_किल्ली(uint64_t etime)
{
	पूर्णाङ्क abort = 0;
	संरचना {
		वर्ण *str;
		u_int len;
		पूर्णाङ्क retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	वर्ण presskey[DELAY_STOP_STR_MAX_LENGTH];
	पूर्णाङ्क presskey_len = 0;
	पूर्णाङ्क presskey_max = 0;
	पूर्णाङ्क i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	यदि (delaykey[0].str == रिक्त)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	यदि (delaykey[1].str == रिक्त)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	प्रति (i = 0; i < परिमाण(delaykey) / परिमाण(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == रिक्त ?
				    0 : strlen(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_LENGTH ?
				    DELAY_STOP_STR_MAX_LENGTH : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	कुरु {
		यदि (tstc()) {
			यदि (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} अन्यथा {
				प्रति (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		प्रति (i = 0; i < परिमाण(delaykey) / परिमाण(delaykey[0]); i++) {
			यदि (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				यदि (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} यावत् (!abort && get_ticks() <= etime);

	प्रत्यागम abort;
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
स्थिर bool fallback_to_sha256(शून्य)
{
	यदि (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		प्रत्यागम env_get_yesno("bootstopusesha256") == 1;
	अन्यथा यदि (IS_ENABLED(CONFIG_CRYPT_PW))
		प्रत्यागम false;
	अन्यथा
		प्रत्यागम true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
स्थिर पूर्णाङ्क abortboot_किल्ली_sequence(पूर्णाङ्क bootdelay)
{
	पूर्णाङ्क abort;
	uint64_t etime = endtick(bootdelay);

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		यदि (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		अन्यथा
			abort = passwd_abort_sha256(etime);
	} अन्यथा {
		abort = passwd_abort_किल्ली(etime);
	}
	यदि (!abort)
		debug_bootkeys("key timeout\n");

	प्रत्यागम abort;
}

स्थिर शून्य print_boot_delay(पूर्णाङ्क bootdelay)
{
	printf(ANSI_CLEAR_ओळ "\rHit any key to stop autoboot: %d", bootdelay);
}

स्थिर पूर्णाङ्क abortboot_single_किल्ली(पूर्णाङ्क bootdelay)
{
	पूर्णाङ्क abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	यदि (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	यावत् ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		कुरु {
			यदि (tstc()) {	/* we got a key press	*/
				पूर्णाङ्क किल्ली;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				किल्ली = getchar();/* consume input	*/
				यदि (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = किल्ली;
				भङ्ग;
			}
			udelay(10000);
		} यावत् (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	प्रत्यागम abort;
}

स्थिर पूर्णाङ्क abortboot(पूर्णाङ्क bootdelay)
{
	पूर्णाङ्क abort = 0;

	यदि (bootdelay >= 0) {
		यदि (autoboot_keyed())
			abort = abortboot_किल्ली_sequence(bootdelay);
		अन्यथा
			abort = abortboot_single_किल्ली(bootdelay);
	}

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	प्रत्यागम abort;
}

स्थिर शून्य process_fdt_options(शून्य)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	यदि (addr)
		env_set_addr("kernaddr", (शून्य *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	यदि (addr)
		env_set_addr("rootaddr", (शून्य *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

ध्रुव वर्ण *bootdelay_process(शून्य)
{
	वर्ण *s;
	पूर्णाङ्क bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (पूर्णाङ्क)simple_strtol(s, रिक्त, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	यदि (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	यदि (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} अन्यथा
#endif /* CONFIG_POST */
	यदि (bootcount_error())
		s = env_get("altbootcmd");
	अन्यथा
		s = env_get("bootcmd");

	यदि (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	प्रत्यागम s;
}

शून्य autoboot_command(ध्रुव वर्ण *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	यदि (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		पूर्णाङ्क prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		यदि (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		यदि (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		यदि (s)
			run_command_list(s, -1, 0);
	}
}
