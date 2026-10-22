// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <ansi.h>
#ಅನ್ತರ್ಭಾವ <autoboot.h>
#ಅನ್ತರ್ಭಾವ <bootretry.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fdtdec.h>
#ಅನ್ತರ್ಭಾವ <hash.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <memalign.h>
#ಅನ್ತರ್ಭಾವ <menu.h>
#ಅನ್ತರ್ಭಾವ <post.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha256.h>
#ಅನ್ತರ್ಭಾವ <bootcount.h>
#ಅನ್ತರ್ಭಾವ <crypt.h>
#ಅನ್ತರ್ಭಾವ <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ stored_bootdelay;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ menukey;

#ಯದಿ defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#ಅನ್ಯಥಾ
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#ಯದಿ defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#ಅನ್ಯಥಾ
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ passwd_abort_crypt(uint64_t etime)
{
	ಧ್ರುವ ವರ್ಣ *crypt_env_str = env_get("bootstopkeycrypt");
	ವರ್ಣ presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	ಪೂರ್ಣಾಙ್ಕ abort = 0;
	ಪೂರ್ಣಾಙ್ಕ never_timeout = 0;
	ಪೂರ್ಣಾಙ್ಕ err;

	ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	ಯದಿ (!crypt_env_str)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* We expect the stop-string to be newline-terminated */
	ಕುರು {
		ಯದಿ (tstc()) {
			/* Check for input string overflow */
			ಯದಿ (presskey_len >= ಪರಿಮಾಣ(presskey))
				ಪ್ರತ್ಯಾಗಮ 0;

			presskey[presskey_len] = getchar();

			ಯದಿ ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					ಅನುವರ್ತ;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				ಯದಿ (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				ಭಙ್ಗ;
			} ಅನ್ಯಥಾ {
				presskey_len++;
			}
		}
		udelay(10000);
	} ಯಾವತ್ (never_timeout || get_ticks() <= etime);

	ಪ್ರತ್ಯಾಗಮ abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ slow_equals(u8 *a, u8 *b, ಪೂರ್ಣಾಙ್ಕ len)
{
	ಪೂರ್ಣಾಙ್ಕ diff = 0;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	ಪ್ರತ್ಯಾಗಮ diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ passwd_abort_sha256(uint64_t etime)
{
	ಧ್ರುವ ವರ್ಣ *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	ವರ್ಣ *presskey;
	ವರ್ಣ *c;
	ಧ್ರುವ ವರ್ಣ *algo_name = "sha256";
	u_int presskey_len = 0;
	ಪೂರ್ಣಾಙ್ಕ abort = 0;
	ಪೂರ್ಣಾಙ್ಕ size = ಪರಿಮಾಣ(sha);
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (sha_env_str == ರಿಕ್ತ)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	ಯದಿ (!presskey)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	c = strstr(sha_env_str, ":");
	ಯದಿ (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	ಯದಿ (ret) {
		printf("Hash %s not supported!\n", algo_name);
		ಮುಕ್ತ(presskey);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	ಯದಿ (!sha) {
		ಮುಕ್ತ(presskey);
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	ಕುರು {
		ಯದಿ (tstc()) {
			/* Check for input string overflow */
			ಯದಿ (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				ಮುಕ್ತ(presskey);
				ಮುಕ್ತ(sha);
				ಪ್ರತ್ಯಾಗಮ 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (ಧ್ರುವ ಶೂನ್ಯ *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			ಯದಿ (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} ಯಾವತ್ (!abort && get_ticks() <= etime);

	ಮುಕ್ತ(presskey);
	ಮುಕ್ತ(sha);
	ಪ್ರತ್ಯಾಗಮ abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ passwd_abort_ಕೀಲಿ(uint64_t etime)
{
	ಪೂರ್ಣಾಙ್ಕ abort = 0;
	ಸಂರಚನಾ {
		ವರ್ಣ *str;
		u_int len;
		ಪೂರ್ಣಾಙ್ಕ retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	ವರ್ಣ presskey[DELAY_STOP_STR_MAX_LENGTH];
	ಪೂರ್ಣಾಙ್ಕ presskey_len = 0;
	ಪೂರ್ಣಾಙ್ಕ presskey_max = 0;
	ಪೂರ್ಣಾಙ್ಕ i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	ಯದಿ (delaykey[0].str == ರಿಕ್ತ)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	ಯದಿ (delaykey[1].str == ರಿಕ್ತ)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	ಪ್ರತಿ (i = 0; i < ಪರಿಮಾಣ(delaykey) / ಪರಿಮಾಣ(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ರಿಕ್ತ ?
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
	ಕುರು {
		ಯದಿ (tstc()) {
			ಯದಿ (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} ಅನ್ಯಥಾ {
				ಪ್ರತಿ (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		ಪ್ರತಿ (i = 0; i < ಪರಿಮಾಣ(delaykey) / ಪರಿಮಾಣ(delaykey[0]); i++) {
			ಯದಿ (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				ಯದಿ (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} ಯಾವತ್ (!abort && get_ticks() <= etime);

	ಪ್ರತ್ಯಾಗಮ abort;
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
ಸ್ಥಿರ bool fallback_to_sha256(ಶೂನ್ಯ)
{
	ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		ಪ್ರತ್ಯಾಗಮ env_get_yesno("bootstopusesha256") == 1;
	ಅನ್ಯಥಾ ಯದಿ (IS_ENABLED(CONFIG_CRYPT_PW))
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ abortboot_ಕೀಲಿ_sequence(ಪೂರ್ಣಾಙ್ಕ bootdelay)
{
	ಪೂರ್ಣಾಙ್ಕ abort;
	uint64_t etime = endtick(bootdelay);

	ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		ಯದಿ (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		ಅನ್ಯಥಾ
			abort = passwd_abort_sha256(etime);
	} ಅನ್ಯಥಾ {
		abort = passwd_abort_ಕೀಲಿ(etime);
	}
	ಯದಿ (!abort)
		debug_bootkeys("key timeout\n");

	ಪ್ರತ್ಯಾಗಮ abort;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_boot_delay(ಪೂರ್ಣಾಙ್ಕ bootdelay)
{
	printf(ANSI_CLEAR_ಸಾಲು "\rHit any key to stop autoboot: %d", bootdelay);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ abortboot_single_ಕೀಲಿ(ಪೂರ್ಣಾಙ್ಕ bootdelay)
{
	ಪೂರ್ಣಾಙ್ಕ abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	ಯದಿ (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	ಯಾವತ್ ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		ಕುರು {
			ಯದಿ (tstc()) {	/* we got a key press	*/
				ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				ಕೀಲಿ = getchar();/* consume input	*/
				ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = ಕೀಲಿ;
				ಭಙ್ಗ;
			}
			udelay(10000);
		} ಯಾವತ್ (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	ಪ್ರತ್ಯಾಗಮ abort;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ abortboot(ಪೂರ್ಣಾಙ್ಕ bootdelay)
{
	ಪೂರ್ಣಾಙ್ಕ abort = 0;

	ಯದಿ (bootdelay >= 0) {
		ಯದಿ (autoboot_keyed())
			abort = abortboot_ಕೀಲಿ_sequence(bootdelay);
		ಅನ್ಯಥಾ
			abort = abortboot_single_ಕೀಲಿ(bootdelay);
	}

	ಯದಿ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	ಪ್ರತ್ಯಾಗಮ abort;
}

ಸ್ಥಿರ ಶೂನ್ಯ process_fdt_options(ಶೂನ್ಯ)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	ಯದಿ (addr)
		env_set_addr("kernaddr", (ಶೂನ್ಯ *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	ಯದಿ (addr)
		env_set_addr("rootaddr", (ಶೂನ್ಯ *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

ಧ್ರುವ ವರ್ಣ *bootdelay_process(ಶೂನ್ಯ)
{
	ವರ್ಣ *s;
	ಪೂರ್ಣಾಙ್ಕ bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (ಪೂರ್ಣಾಙ್ಕ)simple_strtol(s, ರಿಕ್ತ, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	ಯದಿ (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} ಅನ್ಯಥಾ
#endif /* CONFIG_POST */
	ಯದಿ (bootcount_error())
		s = env_get("altbootcmd");
	ಅನ್ಯಥಾ
		s = env_get("bootcmd");

	ಯದಿ (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	ಪ್ರತ್ಯಾಗಮ s;
}

ಶೂನ್ಯ autoboot_command(ಧ್ರುವ ವರ್ಣ *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	ಯದಿ (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		ಪೂರ್ಣಾಙ್ಕ prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		ಯದಿ (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		ಯದಿ (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		ಯದಿ (s)
			run_command_list(s, -1, 0);
	}
}
