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

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
স্থির পূর্ণাঙ্ক stored_bootdelay;
স্থির পূর্ণাঙ্ক menukey;

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
স্থির পূর্ণাঙ্ক passwd_abort_crypt(uint64_t etime)
{
	ধ্রুব বর্ণ *crypt_env_str = env_get("bootstopkeycrypt");
	বর্ণ presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	পূর্ণাঙ্ক abort = 0;
	পূর্ণাঙ্ক never_timeout = 0;
	পূর্ণাঙ্ক err;

	যদি (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	যদি (!crypt_env_str)
		প্রত্যাগম 0;

	/* We expect the stop-string to be newline-terminated */
	কুরু {
		যদি (tstc()) {
			/* Check for input string overflow */
			যদি (presskey_len >= পরিমাণ(presskey))
				প্রত্যাগম 0;

			presskey[presskey_len] = getchar();

			যদি ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				যদি (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					অনুবর্ত;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				যদি (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				ভঙ্গ;
			} অন্যথা {
				presskey_len++;
			}
		}
		udelay(10000);
	} যাবত্ (never_timeout || get_ticks() <= etime);

	প্রত্যাগম abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
স্থির পূর্ণাঙ্ক slow_equals(u8 *a, u8 *b, পূর্ণাঙ্ক len)
{
	পূর্ণাঙ্ক diff = 0;
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	প্রত্যাগম diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
স্থির পূর্ণাঙ্ক passwd_abort_sha256(uint64_t etime)
{
	ধ্রুব বর্ণ *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	বর্ণ *presskey;
	বর্ণ *c;
	ধ্রুব বর্ণ *algo_name = "sha256";
	u_int presskey_len = 0;
	পূর্ণাঙ্ক abort = 0;
	পূর্ণাঙ্ক size = পরিমাণ(sha);
	পূর্ণাঙ্ক ret;

	যদি (sha_env_str == রিক্ত)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	যদি (!presskey)
		প্রত্যাগম -ENOMEM;

	c = strstr(sha_env_str, ":");
	যদি (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
		প্রত্যাগম 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	যদি (!sha) {
		মুক্ত(presskey);
		প্রত্যাগম -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	কুরু {
		যদি (tstc()) {
			/* Check for input string overflow */
			যদি (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				মুক্ত(presskey);
				মুক্ত(sha);
				প্রত্যাগম 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (ধ্রুব শূন্য *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			যদি (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} যাবত্ (!abort && get_ticks() <= etime);

	মুক্ত(presskey);
	মুক্ত(sha);
	প্রত্যাগম abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
স্থির পূর্ণাঙ্ক passwd_abort_চাবি(uint64_t etime)
{
	পূর্ণাঙ্ক abort = 0;
	সংরচনা {
		বর্ণ *str;
		u_int len;
		পূর্ণাঙ্ক retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	বর্ণ presskey[DELAY_STOP_STR_MAX_LENGTH];
	পূর্ণাঙ্ক presskey_len = 0;
	পূর্ণাঙ্ক presskey_max = 0;
	পূর্ণাঙ্ক i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	যদি (delaykey[0].str == রিক্ত)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	যদি (delaykey[1].str == রিক্ত)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	প্রতি (i = 0; i < পরিমাণ(delaykey) / পরিমাণ(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == রিক্ত ?
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
	কুরু {
		যদি (tstc()) {
			যদি (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} অন্যথা {
				প্রতি (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		প্রতি (i = 0; i < পরিমাণ(delaykey) / পরিমাণ(delaykey[0]); i++) {
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
	} যাবত্ (!abort && get_ticks() <= etime);

	প্রত্যাগম abort;
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
		প্রত্যাগম env_get_yesno("bootstopusesha256") == 1;
	অন্যথা যদি (IS_ENABLED(CONFIG_CRYPT_PW))
		প্রত্যাগম false;
	অন্যথা
		প্রত্যাগম true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
স্থির পূর্ণাঙ্ক abortboot_চাবি_sequence(পূর্ণাঙ্ক bootdelay)
{
	পূর্ণাঙ্ক abort;
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

	প্রত্যাগম abort;
}

স্থির শূন্য print_boot_delay(পূর্ণাঙ্ক bootdelay)
{
	printf(ANSI_CLEAR_সারি "\rHit any key to stop autoboot: %d", bootdelay);
}

স্থির পূর্ণাঙ্ক abortboot_single_চাবি(পূর্ণাঙ্ক bootdelay)
{
	পূর্ণাঙ্ক abort = 0;
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

	যাবত্ ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		কুরু {
			যদি (tstc()) {	/* we got a key press	*/
				পূর্ণাঙ্ক চাবি;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				চাবি = getchar();/* consume input	*/
				যদি (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = চাবি;
				ভঙ্গ;
			}
			udelay(10000);
		} যাবত্ (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	প্রত্যাগম abort;
}

স্থির পূর্ণাঙ্ক abortboot(পূর্ণাঙ্ক bootdelay)
{
	পূর্ণাঙ্ক abort = 0;

	যদি (bootdelay >= 0) {
		যদি (autoboot_keyed())
			abort = abortboot_চাবি_sequence(bootdelay);
		অন্যথা
			abort = abortboot_single_চাবি(bootdelay);
	}

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	প্রত্যাগম abort;
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

ধ্রুব বর্ণ *bootdelay_process(শূন্য)
{
	বর্ণ *s;
	পূর্ণাঙ্ক bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (পূর্ণাঙ্ক)simple_strtol(s, রিক্ত, 10) : CONFIG_BOOTDELAY;

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

	প্রত্যাগম s;
}

শূন্য autoboot_command(ধ্রুব বর্ণ *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	যদি (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		পূর্ণাঙ্ক prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		যদি (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		যদি (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	যদি (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		যদি (s)
			run_command_list(s, -1, 0);
	}
}
