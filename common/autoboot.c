// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <ansi.h>
#അന്തര്ഭാവ <autoboot.h>
#അന്തര്ഭാവ <bootretry.h>
#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fdtdec.h>
#അന്തര്ഭാവ <hash.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <memalign.h>
#അന്തര്ഭാവ <menu.h>
#അന്തര്ഭാവ <post.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <linux/delay.h>
#അന്തര്ഭാവ <u-boot/sha256.h>
#അന്തര്ഭാവ <bootcount.h>
#അന്തര്ഭാവ <crypt.h>
#അന്തര്ഭാവ <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
സ്ഥിതം പൂർണ്ണസംഖ്യ stored_bootdelay;
സ്ഥിതം പൂർണ്ണസംഖ്യ menukey;

#എങ്കിൽ defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#അല്ലെങ്കിൽ
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#എങ്കിൽ defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#അല്ലെങ്കിൽ
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#അല്ലെങ്കിൽ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ passwd_abort_crypt(uint64_t etime)
{
	സ്ഥിരം അക്ഷരം *crypt_env_str = env_get("bootstopkeycrypt");
	അക്ഷരം presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	പൂർണ്ണസംഖ്യ abort = 0;
	പൂർണ്ണസംഖ്യ never_timeout = 0;
	പൂർണ്ണസംഖ്യ err;

	എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	എങ്കിൽ (!crypt_env_str)
		തിരികെ 0;

	/* We expect the stop-string to be newline-terminated */
	ചെയ്യുക {
		എങ്കിൽ (tstc()) {
			/* Check for input string overflow */
			എങ്കിൽ (presskey_len >= വലുപ്പം(presskey))
				തിരികെ 0;

			presskey[presskey_len] = getchar();

			എങ്കിൽ ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					തുടരുക;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				എങ്കിൽ (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				നിർത്തുക;
			} അല്ലെങ്കിൽ {
				presskey_len++;
			}
		}
		udelay(10000);
	} ആകുമ്പോൾ (never_timeout || get_ticks() <= etime);

	തിരികെ abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ slow_equals(u8 *a, u8 *b, പൂർണ്ണസംഖ്യ len)
{
	പൂർണ്ണസംഖ്യ diff = 0;
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	തിരികെ diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ passwd_abort_sha256(uint64_t etime)
{
	സ്ഥിരം അക്ഷരം *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	അക്ഷരം *presskey;
	അക്ഷരം *c;
	സ്ഥിരം അക്ഷരം *algo_name = "sha256";
	u_int presskey_len = 0;
	പൂർണ്ണസംഖ്യ abort = 0;
	പൂർണ്ണസംഖ്യ size = വലുപ്പം(sha);
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (sha_env_str == ശൂന്യ)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	എങ്കിൽ (!presskey)
		തിരികെ -ENOMEM;

	c = strstr(sha_env_str, ":");
	എങ്കിൽ (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	എങ്കിൽ (ret) {
		printf("Hash %s not supported!\n", algo_name);
		സ്വതന്ത്രം(presskey);
		തിരികെ 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	എങ്കിൽ (!sha) {
		സ്വതന്ത്രം(presskey);
		തിരികെ -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	ചെയ്യുക {
		എങ്കിൽ (tstc()) {
			/* Check for input string overflow */
			എങ്കിൽ (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				സ്വതന്ത്രം(presskey);
				സ്വതന്ത്രം(sha);
				തിരികെ 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (സ്ഥിരം ശൂന്യം *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			എങ്കിൽ (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} ആകുമ്പോൾ (!abort && get_ticks() <= etime);

	സ്വതന്ത്രം(presskey);
	സ്വതന്ത്രം(sha);
	തിരികെ abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ passwd_abort_താക്കോൽ(uint64_t etime)
{
	പൂർണ്ണസംഖ്യ abort = 0;
	ഘടന {
		അക്ഷരം *str;
		u_int len;
		പൂർണ്ണസംഖ്യ retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	അക്ഷരം presskey[DELAY_STOP_STR_MAX_LENGTH];
	പൂർണ്ണസംഖ്യ presskey_len = 0;
	പൂർണ്ണസംഖ്യ presskey_max = 0;
	പൂർണ്ണസംഖ്യ i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	എങ്കിൽ (delaykey[0].str == ശൂന്യ)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	എങ്കിൽ (delaykey[1].str == ശൂന്യ)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	വേണ്ടി (i = 0; i < വലുപ്പം(delaykey) / വലുപ്പം(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ശൂന്യ ?
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
	ചെയ്യുക {
		എങ്കിൽ (tstc()) {
			എങ്കിൽ (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} അല്ലെങ്കിൽ {
				വേണ്ടി (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		വേണ്ടി (i = 0; i < വലുപ്പം(delaykey) / വലുപ്പം(delaykey[0]); i++) {
			എങ്കിൽ (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				എങ്കിൽ (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} ആകുമ്പോൾ (!abort && get_ticks() <= etime);

	തിരികെ abort;
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
സ്ഥിതം bool fallback_to_sha256(ശൂന്യം)
{
	എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		തിരികെ env_get_yesno("bootstopusesha256") == 1;
	അല്ലെങ്കിൽ എങ്കിൽ (IS_ENABLED(CONFIG_CRYPT_PW))
		തിരികെ false;
	അല്ലെങ്കിൽ
		തിരികെ true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ abortboot_താക്കോൽ_sequence(പൂർണ്ണസംഖ്യ bootdelay)
{
	പൂർണ്ണസംഖ്യ abort;
	uint64_t etime = endtick(bootdelay);

	എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		എങ്കിൽ (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		അല്ലെങ്കിൽ
			abort = passwd_abort_sha256(etime);
	} അല്ലെങ്കിൽ {
		abort = passwd_abort_താക്കോൽ(etime);
	}
	എങ്കിൽ (!abort)
		debug_bootkeys("key timeout\n");

	തിരികെ abort;
}

സ്ഥിതം ശൂന്യം print_boot_delay(പൂർണ്ണസംഖ്യ bootdelay)
{
	printf(ANSI_CLEAR_വരി "\rHit any key to stop autoboot: %d", bootdelay);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ abortboot_single_താക്കോൽ(പൂർണ്ണസംഖ്യ bootdelay)
{
	പൂർണ്ണസംഖ്യ abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	എങ്കിൽ (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	ആകുമ്പോൾ ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		ചെയ്യുക {
			എങ്കിൽ (tstc()) {	/* we got a key press	*/
				പൂർണ്ണസംഖ്യ താക്കോൽ;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				താക്കോൽ = getchar();/* consume input	*/
				എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = താക്കോൽ;
				നിർത്തുക;
			}
			udelay(10000);
		} ആകുമ്പോൾ (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	തിരികെ abort;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ abortboot(പൂർണ്ണസംഖ്യ bootdelay)
{
	പൂർണ്ണസംഖ്യ abort = 0;

	എങ്കിൽ (bootdelay >= 0) {
		എങ്കിൽ (autoboot_keyed())
			abort = abortboot_താക്കോൽ_sequence(bootdelay);
		അല്ലെങ്കിൽ
			abort = abortboot_single_താക്കോൽ(bootdelay);
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	തിരികെ abort;
}

സ്ഥിതം ശൂന്യം process_fdt_options(ശൂന്യം)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	എങ്കിൽ (addr)
		env_set_addr("kernaddr", (ശൂന്യം *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	എങ്കിൽ (addr)
		env_set_addr("rootaddr", (ശൂന്യം *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

സ്ഥിരം അക്ഷരം *bootdelay_process(ശൂന്യം)
{
	അക്ഷരം *s;
	പൂർണ്ണസംഖ്യ bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (പൂർണ്ണസംഖ്യ)simple_strtol(s, ശൂന്യ, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	എങ്കിൽ (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} അല്ലെങ്കിൽ
#endif /* CONFIG_POST */
	എങ്കിൽ (bootcount_error())
		s = env_get("altbootcmd");
	അല്ലെങ്കിൽ
		s = env_get("bootcmd");

	എങ്കിൽ (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	തിരികെ s;
}

ശൂന്യം autoboot_command(സ്ഥിരം അക്ഷരം *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	എങ്കിൽ (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		പൂർണ്ണസംഖ്യ prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		എങ്കിൽ (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		എങ്കിൽ (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		എങ്കിൽ (s)
			run_command_list(s, -1, 0);
	}
}
