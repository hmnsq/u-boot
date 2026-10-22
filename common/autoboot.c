// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <ansi.h>
#அந்தர்பாவ <autoboot.h>
#அந்தர்பாவ <bootretry.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fdtdec.h>
#அந்தர்பாவ <hash.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <memalign.h>
#அந்தர்பாவ <menu.h>
#அந்தர்பாவ <post.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <linux/delay.h>
#அந்தர்பாவ <u-boot/sha256.h>
#அந்தர்பாவ <bootcount.h>
#அந்தர்பாவ <crypt.h>
#அந்தர்பாவ <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
நிலையான முழு stored_bootdelay;
நிலையான முழு menukey;

#எனில் defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#இல்லையெனில்
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#எனில் defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#இல்லையெனில்
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#இல்லையெனில்
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
நிலையான முழு passwd_abort_crypt(uint64_t etime)
{
	மாறிலி எழுத்து *crypt_env_str = env_get("bootstopkeycrypt");
	எழுத்து presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	முழு abort = 0;
	முழு never_timeout = 0;
	முழு err;

	எனில் (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	எனில் (!crypt_env_str)
		திரும்பு 0;

	/* We expect the stop-string to be newline-terminated */
	செய் {
		எனில் (tstc()) {
			/* Check for input string overflow */
			எனில் (presskey_len >= அளவு(presskey))
				திரும்பு 0;

			presskey[presskey_len] = getchar();

			எனில் ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				எனில் (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					தொடர்;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				எனில் (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				நிறுத்து;
			} இல்லையெனில் {
				presskey_len++;
			}
		}
		udelay(10000);
	} போது (never_timeout || get_ticks() <= etime);

	திரும்பு abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
நிலையான முழு slow_equals(u8 *a, u8 *b, முழு len)
{
	முழு diff = 0;
	முழு i;

	க்கு (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	திரும்பு diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
நிலையான முழு passwd_abort_sha256(uint64_t etime)
{
	மாறிலி எழுத்து *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	எழுத்து *presskey;
	எழுத்து *c;
	மாறிலி எழுத்து *algo_name = "sha256";
	u_int presskey_len = 0;
	முழு abort = 0;
	முழு size = அளவு(sha);
	முழு ret;

	எனில் (sha_env_str == வெற்று)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	எனில் (!presskey)
		திரும்பு -ENOMEM;

	c = strstr(sha_env_str, ":");
	எனில் (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	எனில் (ret) {
		printf("Hash %s not supported!\n", algo_name);
		விடுவி(presskey);
		திரும்பு 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	எனில் (!sha) {
		விடுவி(presskey);
		திரும்பு -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	செய் {
		எனில் (tstc()) {
			/* Check for input string overflow */
			எனில் (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				விடுவி(presskey);
				விடுவி(sha);
				திரும்பு 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (மாறிலி வெறுமை *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			எனில் (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} போது (!abort && get_ticks() <= etime);

	விடுவி(presskey);
	விடுவி(sha);
	திரும்பு abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
நிலையான முழு passwd_abort_சாவி(uint64_t etime)
{
	முழு abort = 0;
	கட்டமைப்பு {
		எழுத்து *str;
		u_int len;
		முழு retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	எழுத்து presskey[DELAY_STOP_STR_MAX_LENGTH];
	முழு presskey_len = 0;
	முழு presskey_max = 0;
	முழு i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	எனில் (delaykey[0].str == வெற்று)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	எனில் (delaykey[1].str == வெற்று)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	க்கு (i = 0; i < அளவு(delaykey) / அளவு(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == வெற்று ?
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
	செய் {
		எனில் (tstc()) {
			எனில் (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} இல்லையெனில் {
				க்கு (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		க்கு (i = 0; i < அளவு(delaykey) / அளவு(delaykey[0]); i++) {
			எனில் (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				எனில் (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} போது (!abort && get_ticks() <= etime);

	திரும்பு abort;
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
நிலையான bool fallback_to_sha256(வெறுமை)
{
	எனில் (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		திரும்பு env_get_yesno("bootstopusesha256") == 1;
	இல்லையெனில் எனில் (IS_ENABLED(CONFIG_CRYPT_PW))
		திரும்பு false;
	இல்லையெனில்
		திரும்பு true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
நிலையான முழு abortboot_சாவி_sequence(முழு bootdelay)
{
	முழு abort;
	uint64_t etime = endtick(bootdelay);

	எனில் (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	எனில் (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		எனில் (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		இல்லையெனில்
			abort = passwd_abort_sha256(etime);
	} இல்லையெனில் {
		abort = passwd_abort_சாவி(etime);
	}
	எனில் (!abort)
		debug_bootkeys("key timeout\n");

	திரும்பு abort;
}

நிலையான வெறுமை print_boot_delay(முழு bootdelay)
{
	printf(ANSI_CLEAR_வரி "\rHit any key to stop autoboot: %d", bootdelay);
}

நிலையான முழு abortboot_single_சாவி(முழு bootdelay)
{
	முழு abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	எனில் (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	போது ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		செய் {
			எனில் (tstc()) {	/* we got a key press	*/
				முழு சாவி;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				சாவி = getchar();/* consume input	*/
				எனில் (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = சாவி;
				நிறுத்து;
			}
			udelay(10000);
		} போது (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	திரும்பு abort;
}

நிலையான முழு abortboot(முழு bootdelay)
{
	முழு abort = 0;

	எனில் (bootdelay >= 0) {
		எனில் (autoboot_keyed())
			abort = abortboot_சாவி_sequence(bootdelay);
		இல்லையெனில்
			abort = abortboot_single_சாவி(bootdelay);
	}

	எனில் (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	திரும்பு abort;
}

நிலையான வெறுமை process_fdt_options(வெறுமை)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	எனில் (addr)
		env_set_addr("kernaddr", (வெறுமை *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	எனில் (addr)
		env_set_addr("rootaddr", (வெறுமை *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

மாறிலி எழுத்து *bootdelay_process(வெறுமை)
{
	எழுத்து *s;
	முழு bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (முழு)simple_strtol(s, வெற்று, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	எனில் (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	எனில் (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	எனில் (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} இல்லையெனில்
#endif /* CONFIG_POST */
	எனில் (bootcount_error())
		s = env_get("altbootcmd");
	இல்லையெனில்
		s = env_get("bootcmd");

	எனில் (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	திரும்பு s;
}

வெறுமை autoboot_command(மாறிலி எழுத்து *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	எனில் (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		முழு prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		எனில் (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		எனில் (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	எனில் (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		எனில் (s)
			run_command_list(s, -1, 0);
	}
}
