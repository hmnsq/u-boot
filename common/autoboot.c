// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <ansi.h>
#અન્તર્ભાવ <autoboot.h>
#અન્તર્ભાવ <bootretry.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fdtdec.h>
#અન્તર્ભાવ <hash.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <memalign.h>
#અન્તર્ભાવ <menu.h>
#અન્તર્ભાવ <post.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <linux/delay.h>
#અન્તર્ભાવ <u-boot/sha256.h>
#અન્તર્ભાવ <bootcount.h>
#અન્તર્ભાવ <crypt.h>
#અન્તર્ભાવ <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
સ્થિર પૂર્ણાંક stored_bootdelay;
સ્થિર પૂર્ણાંક menukey;

#જો defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#નહીંતર
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#જો defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#નહીંતર
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#નહીંતર
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
સ્થિર પૂર્ણાંક passwd_abort_crypt(uint64_t etime)
{
	અચલ અક્ષર *crypt_env_str = env_get("bootstopkeycrypt");
	અક્ષર presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	પૂર્ણાંક abort = 0;
	પૂર્ણાંક never_timeout = 0;
	પૂર્ણાંક err;

	જો (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	જો (!crypt_env_str)
		પાછા 0;

	/* We expect the stop-string to be newline-terminated */
	કરો {
		જો (tstc()) {
			/* Check for input string overflow */
			જો (presskey_len >= કદ(presskey))
				પાછા 0;

			presskey[presskey_len] = getchar();

			જો ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				જો (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					ચાલુ;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				જો (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				તોડો;
			} નહીંતર {
				presskey_len++;
			}
		}
		udelay(10000);
	} જ્યારે (never_timeout || get_ticks() <= etime);

	પાછા abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
સ્થિર પૂર્ણાંક slow_equals(u8 *a, u8 *b, પૂર્ણાંક len)
{
	પૂર્ણાંક diff = 0;
	પૂર્ણાંક i;

	માટે (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	પાછા diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
સ્થિર પૂર્ણાંક passwd_abort_sha256(uint64_t etime)
{
	અચલ અક્ષર *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	અક્ષર *presskey;
	અક્ષર *c;
	અચલ અક્ષર *algo_name = "sha256";
	u_int presskey_len = 0;
	પૂર્ણાંક abort = 0;
	પૂર્ણાંક size = કદ(sha);
	પૂર્ણાંક ret;

	જો (sha_env_str == ખાલી)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	જો (!presskey)
		પાછા -ENOMEM;

	c = strstr(sha_env_str, ":");
	જો (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	જો (ret) {
		printf("Hash %s not supported!\n", algo_name);
		મુક્ત(presskey);
		પાછા 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	જો (!sha) {
		મુક્ત(presskey);
		પાછા -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	કરો {
		જો (tstc()) {
			/* Check for input string overflow */
			જો (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				મુક્ત(presskey);
				મુક્ત(sha);
				પાછા 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (અચલ શૂન્ય *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			જો (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} જ્યારે (!abort && get_ticks() <= etime);

	મુક્ત(presskey);
	મુક્ત(sha);
	પાછા abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
સ્થિર પૂર્ણાંક passwd_abort_ચાવી(uint64_t etime)
{
	પૂર્ણાંક abort = 0;
	સંરચના {
		અક્ષર *str;
		u_int len;
		પૂર્ણાંક retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	અક્ષર presskey[DELAY_STOP_STR_MAX_LENGTH];
	પૂર્ણાંક presskey_len = 0;
	પૂર્ણાંક presskey_max = 0;
	પૂર્ણાંક i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	જો (delaykey[0].str == ખાલી)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	જો (delaykey[1].str == ખાલી)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	માટે (i = 0; i < કદ(delaykey) / કદ(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ખાલી ?
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
	કરો {
		જો (tstc()) {
			જો (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} નહીંતર {
				માટે (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		માટે (i = 0; i < કદ(delaykey) / કદ(delaykey[0]); i++) {
			જો (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				જો (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} જ્યારે (!abort && get_ticks() <= etime);

	પાછા abort;
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
સ્થિર bool fallback_to_sha256(શૂન્ય)
{
	જો (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		પાછા env_get_yesno("bootstopusesha256") == 1;
	નહીંતર જો (IS_ENABLED(CONFIG_CRYPT_PW))
		પાછા false;
	નહીંતર
		પાછા true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
સ્થિર પૂર્ણાંક abortboot_ચાવી_sequence(પૂર્ણાંક bootdelay)
{
	પૂર્ણાંક abort;
	uint64_t etime = endtick(bootdelay);

	જો (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	જો (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		જો (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		નહીંતર
			abort = passwd_abort_sha256(etime);
	} નહીંતર {
		abort = passwd_abort_ચાવી(etime);
	}
	જો (!abort)
		debug_bootkeys("key timeout\n");

	પાછા abort;
}

સ્થિર શૂન્ય print_boot_delay(પૂર્ણાંક bootdelay)
{
	printf(ANSI_CLEAR_લીટી "\rHit any key to stop autoboot: %d", bootdelay);
}

સ્થિર પૂર્ણાંક abortboot_single_ચાવી(પૂર્ણાંક bootdelay)
{
	પૂર્ણાંક abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	જો (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	જ્યારે ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		કરો {
			જો (tstc()) {	/* we got a key press	*/
				પૂર્ણાંક ચાવી;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				ચાવી = getchar();/* consume input	*/
				જો (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = ચાવી;
				તોડો;
			}
			udelay(10000);
		} જ્યારે (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	પાછા abort;
}

સ્થિર પૂર્ણાંક abortboot(પૂર્ણાંક bootdelay)
{
	પૂર્ણાંક abort = 0;

	જો (bootdelay >= 0) {
		જો (autoboot_keyed())
			abort = abortboot_ચાવી_sequence(bootdelay);
		નહીંતર
			abort = abortboot_single_ચાવી(bootdelay);
	}

	જો (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	પાછા abort;
}

સ્થિર શૂન્ય process_fdt_options(શૂન્ય)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	જો (addr)
		env_set_addr("kernaddr", (શૂન્ય *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	જો (addr)
		env_set_addr("rootaddr", (શૂન્ય *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

અચલ અક્ષર *bootdelay_process(શૂન્ય)
{
	અક્ષર *s;
	પૂર્ણાંક bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (પૂર્ણાંક)simple_strtol(s, ખાલી, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	જો (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	જો (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	જો (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} નહીંતર
#endif /* CONFIG_POST */
	જો (bootcount_error())
		s = env_get("altbootcmd");
	નહીંતર
		s = env_get("bootcmd");

	જો (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	પાછા s;
}

શૂન્ય autoboot_command(અચલ અક્ષર *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	જો (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		પૂર્ણાંક prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		જો (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		જો (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	જો (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		જો (s)
			run_command_list(s, -1, 0);
	}
}
