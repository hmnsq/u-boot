// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#অন্তর্ভাব <config.h>
#অন্তর্ভাব <ansi.h>
#অন্তর্ভাব <autoboot.h>
#অন্তর্ভাব <bootretry.h>
#অন্তর্ভাব <cli.h>
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <console.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fdtdec.h>
#অন্তর্ভাব <hash.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <memalign.h>
#অন্তর্ভাব <menu.h>
#অন্তর্ভাব <post.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <linux/delay.h>
#অন্তর্ভাব <u-boot/sha256.h>
#অন্তর্ভাব <bootcount.h>
#অন্তর্ভাব <crypt.h>
#অন্তর্ভাব <dm/ofnode.h>

DECLARE_GLOBAL_তথ্য_PTR;

#define DELAY_STOP_STR_MAX_দৈর্ঘ্য 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
স্থির পূর্ণসংখ্যা stored_bootdelay;
স্থির পূর্ণসংখ্যা menukey;

#যদি defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#অন্যথা
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#যদি defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#অন্যথা
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#অন্যথা
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
স্থির পূর্ণসংখ্যা passwd_abort_crypt(uint64_t etime)
{
	ধ্রুবক অক্ষর *crypt_env_str = env_get("bootstopkeycrypt");
	অক্ষর presskey[DELAY_STOP_STR_MAX_দৈর্ঘ্য];
	u_int presskey_len = 0;
	পূর্ণসংখ্যা abort = 0;
	পূর্ণসংখ্যা never_timeout = 0;
	পূর্ণসংখ্যা err;

	যদি (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	যদি (!crypt_env_str)
		ফেরত 0;

	/* We expect the stop-string to be newline-terminated */
	করো {
		যদি (tstc()) {
			/* Check for input string overflow */
			যদি (presskey_len >= আকার(presskey))
				ফেরত 0;

			presskey[presskey_len] = getchar();

			যদি ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				যদি (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					চালু;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				যদি (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				বিরতি;
			} অন্যথা {
				presskey_len++;
			}
		}
		udelay(10000);
	} যখন (never_timeout || get_ticks() <= etime);

	ফেরত abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
স্থির পূর্ণসংখ্যা slow_equals(u8 *a, u8 *b, পূর্ণসংখ্যা len)
{
	পূর্ণসংখ্যা diff = 0;
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	ফেরত diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
স্থির পূর্ণসংখ্যা passwd_abort_sha256(uint64_t etime)
{
	ধ্রুবক অক্ষর *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_যোগ_LEN];
	u8 *sha;
	অক্ষর *presskey;
	অক্ষর *c;
	ধ্রুবক অক্ষর *algo_name = "sha256";
	u_int presskey_len = 0;
	পূর্ণসংখ্যা abort = 0;
	পূর্ণসংখ্যা আকার_2 = আকার(sha);
	পূর্ণসংখ্যা ret;

	যদি (sha_env_str == ফাঁকা)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_দৈর্ঘ্য);
	যদি (!presskey)
		ফেরত -ENOMEM;

	c = strstr(sha_env_str, ":");
	যদি (c && (c - sha_env_str < DELAY_STOP_STR_MAX_দৈর্ঘ্য)) {
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
	যদি (ret) {
		printf("Hash %s not supported!\n", algo_name);
		মুক্ত(presskey);
		ফেরত 0;
	}

	sha = malloc_cache_aligned(SHA256_যোগ_LEN);
	যদি (!sha) {
		মুক্ত(presskey);
		ফেরত -ENOMEM;
	}
	আকার_2 = SHA256_যোগ_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	করো {
		যদি (tstc()) {
			/* Check for input string overflow */
			যদি (presskey_len >= DELAY_STOP_STR_MAX_দৈর্ঘ্য) {
				মুক্ত(presskey);
				মুক্ত(sha);
				ফেরত 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (ধ্রুবক শূন্য *)presskey,
				   presskey_len, sha, &আকার_2);

			/* And check if sha matches saved value in env */
			যদি (slow_equals(sha, sha_env, SHA256_যোগ_LEN))
				abort = 1;
		}
		udelay(10000);
	} যখন (!abort && get_ticks() <= etime);

	মুক্ত(presskey);
	মুক্ত(sha);
	ফেরত abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
স্থির পূর্ণসংখ্যা passwd_abort_চাবি(uint64_t etime)
{
	পূর্ণসংখ্যা abort = 0;
	কাঠামো {
		অক্ষর *str;
		u_int len;
		পূর্ণসংখ্যা retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	অক্ষর presskey[DELAY_STOP_STR_MAX_দৈর্ঘ্য];
	পূর্ণসংখ্যা presskey_len = 0;
	পূর্ণসংখ্যা presskey_max = 0;
	পূর্ণসংখ্যা i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	যদি (delaykey[0].str == ফাঁকা)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	যদি (delaykey[1].str == ফাঁকা)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	জন্য (i = 0; i < আকার(delaykey) / আকার(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ফাঁকা ?
				    0 : strlen(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_দৈর্ঘ্য ?
				    DELAY_STOP_STR_MAX_দৈর্ঘ্য : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	করো {
		যদি (tstc()) {
			যদি (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} অন্যথা {
				জন্য (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		জন্য (i = 0; i < আকার(delaykey) / আকার(delaykey[0]); i++) {
			যদি (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				যদি (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} যখন (!abort && get_ticks() <= etime);

	ফেরত abort;
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
স্থির bool fallback_to_sha256(শূন্য)
{
	যদি (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		ফেরত env_get_yesno("bootstopusesha256") == 1;
	অন্যথা যদি (IS_ENABLED(CONFIG_CRYPT_PW))
		ফেরত false;
	অন্যথা
		ফেরত true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
স্থির পূর্ণসংখ্যা abortboot_চাবি_sequence(পূর্ণসংখ্যা bootdelay)
{
	পূর্ণসংখ্যা abort;
	uint64_t etime = endtick(bootdelay);

	যদি (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	যদি (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		যদি (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		অন্যথা
			abort = passwd_abort_sha256(etime);
	} অন্যথা {
		abort = passwd_abort_চাবি(etime);
	}
	যদি (!abort)
		debug_bootkeys("key timeout\n");

	ফেরত abort;
}

স্থির শূন্য print_boot_delay(পূর্ণসংখ্যা bootdelay)
{
	printf(ANSI_CLEAR_সারি "\rHit any key to stop autoboot: %d", bootdelay);
}

স্থির পূর্ণসংখ্যা abortboot_single_চাবি(পূর্ণসংখ্যা bootdelay)
{
	পূর্ণসংখ্যা abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	যদি (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	যখন ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		করো {
			যদি (tstc()) {	/* we got a key press	*/
				পূর্ণসংখ্যা চাবি;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				চাবি = getchar();/* consume input	*/
				যদি (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = চাবি;
				বিরতি;
			}
			udelay(10000);
		} যখন (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	ফেরত abort;
}

স্থির পূর্ণসংখ্যা abortboot(পূর্ণসংখ্যা bootdelay)
{
	পূর্ণসংখ্যা abort = 0;

	যদি (bootdelay >= 0) {
		যদি (autoboot_keyed())
			abort = abortboot_চাবি_sequence(bootdelay);
		অন্যথা
			abort = abortboot_single_চাবি(bootdelay);
	}

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	ফেরত abort;
}

স্থির শূন্য process_fdt_options(শূন্য)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	যদি (addr)
		env_set_addr("kernaddr", (শূন্য *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	যদি (addr)
		env_set_addr("rootaddr", (শূন্য *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

ধ্রুবক অক্ষর *bootdelay_process(শূন্য)
{
	অক্ষর *s;
	পূর্ণসংখ্যা bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (পূর্ণসংখ্যা)simple_strtol(s, ফাঁকা, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	যদি (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	যদি (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	যদি (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} অন্যথা
#endif /* CONFIG_POST */
	যদি (bootcount_error())
		s = env_get("altbootcmd");
	অন্যথা
		s = env_get("bootcmd");

	যদি (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	ফেরত s;
}

শূন্য autoboot_command(ধ্রুবক অক্ষর *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	যদি (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		পূর্ণসংখ্যা পূর্ব;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		যদি (lock)
			পূর্ব = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		যদি (lock)
			disable_ctrlc(পূর্ব);	/* restore Ctrl-C checking */
	}

	যদি (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		যদি (s)
			run_command_list(s, -1, 0);
	}
}
