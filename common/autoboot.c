// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ansi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <autoboot.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootretry.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdtdec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memalign.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <menu.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <post.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/sha256.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootcount.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <crypt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/ofnode.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DELAY_STOP_STR_MAX_ਲੰਮਾਈ 64

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEBUG_BOOTKEYS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_BOOTKEYS 0
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_bootkeys(fmt, ਦਲੀਲਾਂ...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ਸਥਿਰ ਪੂਰਨ stored_bootdelay;
ਸਥਿਰ ਪੂਰਨ menukey;

#ਜੇ defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTOBOOT_STOP_STR_CRYPT	""
#ਜੇ_ਅੰਤ
#ਜੇ defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTOBOOT_STOP_STR_SHA256	""
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_AUTOBOOT_USE_MENUKEY
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTOBOOT_MENUKEY 0
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ passwd_abort_crypt(uint64_t etime)
{
	ਅਚਲ ਅੱਖਰ *crypt_env_str = env_get("bootstopkeycrypt");
	ਅੱਖਰ presskey[DELAY_STOP_STR_MAX_ਲੰਮਾਈ];
	u_int presskey_len = 0;
	ਪੂਰਨ abort = 0;
	ਪੂਰਨ never_timeout = 0;
	ਪੂਰਨ err;

	ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	ਜੇ (!crypt_env_str)
		ਵਾਪਸ 0;

	/* We expect the stop-string to be newline-terminated */
	ਕਰੋ {
		ਜੇ (tstc()) {
			/* Check for input string overflow */
			ਜੇ (presskey_len >= ਆਕਾਰ(presskey))
				ਵਾਪਸ 0;

			presskey[presskey_len] = getchar();

			ਜੇ ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					ਜਾਰੀ;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				ਜੇ (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ {
				presskey_len++;
			}
		}
		udelay(10000);
	} ਜਦੋਂ (never_timeout || get_ticks() <= etime);

	ਵਾਪਸ abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
ਸਥਿਰ ਪੂਰਨ slow_equals(u8 *a, u8 *b, ਪੂਰਨ len)
{
	ਪੂਰਨ diff = 0;
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	ਵਾਪਸ diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ਸਥਿਰ ਪੂਰਨ passwd_abort_sha256(uint64_t etime)
{
	ਅਚਲ ਅੱਖਰ *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_ਜੋੜ_LEN];
	u8 *sha;
	ਅੱਖਰ *presskey;
	ਅੱਖਰ *c;
	ਅਚਲ ਅੱਖਰ *algo_name = "sha256";
	u_int presskey_len = 0;
	ਪੂਰਨ abort = 0;
	ਪੂਰਨ ਮਾਪ = ਆਕਾਰ(sha);
	ਪੂਰਨ ret;

	ਜੇ (sha_env_str == ਨਿਰਾ)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_ਲੰਮਾਈ);
	ਜੇ (!presskey)
		ਵਾਪਸ -ENOMEM;

	c = ਲੜੀ_ਖੋਜ(sha_env_str, ":");
	ਜੇ (c && (c - sha_env_str < DELAY_STOP_STR_MAX_ਲੰਮਾਈ)) {
		/* preload presskey with salt */
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(presskey, sha_env_str, c - sha_env_str);
		presskey_len = c - sha_env_str;
		sha_env_str = c + 1;
	}
	/*
	 * Generate the binary value from the environment hash value
	 * so that we can compare this value with the computed hash
	 * from the user input
	 */
	ret = hash_parse_string(algo_name, sha_env_str, sha_env);
	ਜੇ (ret) {
		ਲਿਖੋ("Hash %s not supported!\n", algo_name);
		ਮੁਕਤ(presskey);
		ਵਾਪਸ 0;
	}

	sha = malloc_cache_aligned(SHA256_ਜੋੜ_LEN);
	ਜੇ (!sha) {
		ਮੁਕਤ(presskey);
		ਵਾਪਸ -ENOMEM;
	}
	ਮਾਪ = SHA256_ਜੋੜ_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	ਕਰੋ {
		ਜੇ (tstc()) {
			/* Check for input string overflow */
			ਜੇ (presskey_len >= DELAY_STOP_STR_MAX_ਲੰਮਾਈ) {
				ਮੁਕਤ(presskey);
				ਮੁਕਤ(sha);
				ਵਾਪਸ 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_ਖੰਡ(algo_name, (ਅਚਲ ਖਾਲੀ *)presskey,
				   presskey_len, sha, &ਮਾਪ);

			/* And check if sha matches saved value in env */
			ਜੇ (slow_equals(sha, sha_env, SHA256_ਜੋੜ_LEN))
				abort = 1;
		}
		udelay(10000);
	} ਜਦੋਂ (!abort && get_ticks() <= etime);

	ਮੁਕਤ(presskey);
	ਮੁਕਤ(sha);
	ਵਾਪਸ abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ਸਥਿਰ ਪੂਰਨ passwd_abort_ਕੁੰਜੀ(uint64_t etime)
{
	ਪੂਰਨ abort = 0;
	ਬਣਤਰ {
		ਅੱਖਰ *str;
		u_int len;
		ਪੂਰਨ retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	ਅੱਖਰ presskey[DELAY_STOP_STR_MAX_ਲੰਮਾਈ];
	ਪੂਰਨ presskey_len = 0;
	ਪੂਰਨ presskey_max = 0;
	ਪੂਰਨ i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	ਜੇ (delaykey[0].str == ਨਿਰਾ)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	ਜੇ (delaykey[1].str == ਨਿਰਾ)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	ਲਈ (i = 0; i < ਆਕਾਰ(delaykey) / ਆਕਾਰ(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ਨਿਰਾ ?
				    0 : ਲੜੀ_ਲੰਬਾਈ(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_ਲੰਮਾਈ ?
				    DELAY_STOP_STR_MAX_ਲੰਮਾਈ : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	ਕਰੋ {
		ਜੇ (tstc()) {
			ਜੇ (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} ਨਹੀਂਤਾਂ {
				ਲਈ (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		ਲਈ (i = 0; i < ਆਕਾਰ(delaykey) / ਆਕਾਰ(delaykey[0]); i++) {
			ਜੇ (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				ਜੇ (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} ਜਦੋਂ (!abort && get_ticks() <= etime);

	ਵਾਪਸ abort;
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
ਸਥਿਰ bool fallback_ਮੰਜ਼ਿਲ_sha256(ਖਾਲੀ)
{
	ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		ਵਾਪਸ env_get_yesno("bootstopusesha256") == 1;
	ਨਹੀਂਤਾਂ ਜੇ (IS_ENABLED(CONFIG_CRYPT_PW))
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ਸਥਿਰ ਪੂਰਨ abortboot_ਕੁੰਜੀ_sequence(ਪੂਰਨ bootdelay)
{
	ਪੂਰਨ abort;
	uint64_t etime = endtick(bootdelay);

	ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	ਲਿਖੋ(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		ਜੇ (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_ਮੰਜ਼ਿਲ_sha256())
			abort = passwd_abort_crypt(etime);
		ਨਹੀਂਤਾਂ
			abort = passwd_abort_sha256(etime);
	} ਨਹੀਂਤਾਂ {
		abort = passwd_abort_ਕੁੰਜੀ(etime);
	}
	ਜੇ (!abort)
		debug_bootkeys("key timeout\n");

	ਵਾਪਸ abort;
}

ਸਥਿਰ ਖਾਲੀ print_boot_delay(ਪੂਰਨ bootdelay)
{
	ਲਿਖੋ(ANSI_CLEAR_ਸਤਰ "\rHit any key to stop autoboot: %d", bootdelay);
}

ਸਥਿਰ ਪੂਰਨ abortboot_single_ਕੁੰਜੀ(ਪੂਰਨ bootdelay)
{
	ਪੂਰਨ abort = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	ਜੇ (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	ਜਦੋਂ ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		ਕਰੋ {
			ਜੇ (tstc()) {	/* we got a key press	*/
				ਪੂਰਨ ਕੁੰਜੀ;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				ਕੁੰਜੀ = getchar();/* consume input	*/
				ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = ਕੁੰਜੀ;
				ਰੋਕੋ;
			}
			udelay(10000);
		} ਜਦੋਂ (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	ਵਾਪਸ abort;
}

ਸਥਿਰ ਪੂਰਨ abortboot(ਪੂਰਨ bootdelay)
{
	ਪੂਰਨ abort = 0;

	ਜੇ (bootdelay >= 0) {
		ਜੇ (autoboot_keyed())
			abort = abortboot_ਕੁੰਜੀ_sequence(bootdelay);
		ਨਹੀਂਤਾਂ
			abort = abortboot_single_ਕੁੰਜੀ(bootdelay);
	}

	ਜੇ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	ਵਾਪਸ abort;
}

ਸਥਿਰ ਖਾਲੀ process_fdt_options(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਪਾਠ_ਅਧਾਰ
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	ਜੇ (addr)
		env_ਸੈੱਟ_addr("kernaddr", (ਖਾਲੀ *)(CONFIG_ਪਾਠ_ਅਧਾਰ + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	ਜੇ (addr)
		env_ਸੈੱਟ_addr("rootaddr", (ਖਾਲੀ *)(CONFIG_ਪਾਠ_ਅਧਾਰ + addr));
#ਜੇ_ਅੰਤ /* CONFIG_TEXT_BASE */
}

ਅਚਲ ਅੱਖਰ *bootdelay_process(ਖਾਲੀ)
{
	ਅੱਖਰ *s;
	ਪੂਰਨ bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (ਪੂਰਨ)simple_strtol(s, ਨਿਰਾ, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	ਜੇ (IS_ENABLED(CONFIG_ਦਾ_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POST
	ਜੇ (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* CONFIG_POST */
	ਜੇ (bootcount_error())
		s = env_get("altbootcmd");
	ਨਹੀਂਤਾਂ
		s = env_get("bootcmd");

	ਜੇ (IS_ENABLED(CONFIG_ਦਾ_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	ਵਾਪਸ s;
}

ਖਾਲੀ autoboot_command(ਅਚਲ ਅੱਖਰ *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	ਜੇ (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool ਤਾਲਾ;
		ਪੂਰਨ prev;

		ਤਾਲਾ = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		ਜੇ (ਤਾਲਾ)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		ਲੜੀਵਾਰ_command_list(s, -1, 0);

		ਜੇ (ਤਾਲਾ)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		ਜੇ (s)
			ਲੜੀਵਾਰ_command_list(s, -1, 0);
	}
}
