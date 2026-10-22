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

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

#ವ್ಯಾಖ್ಯಾನಿಸು DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ 64

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DEBUG_BOOTKEYS
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_BOOTKEYS 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು debug_bootkeys(fmt, ವಾದಗಳು...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stored_bootdelay;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ menukey;

#ಆದರೆ defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#ವ್ಯಾಖ್ಯಾನಿಸು AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು AUTOBOOT_STOP_STR_CRYPT	""
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#ವ್ಯಾಖ್ಯಾನಿಸು AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು AUTOBOOT_STOP_STR_SHA256	""
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_AUTOBOOT_USE_MENUKEY
#ವ್ಯಾಖ್ಯಾನಿಸು AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು AUTOBOOT_MENUKEY 0
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ passwd_abort_crypt(uint64_t etime)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *crypt_env_str = env_get("bootstopkeycrypt");
	ಅಕ್ಷರ presskey[DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ];
	u_int presskey_len = 0;
	ಪೂರ್ಣಾಂಕ abort = 0;
	ಪೂರ್ಣಾಂಕ never_timeout = 0;
	ಪೂರ್ಣಾಂಕ err;

	ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	ಆದರೆ (!crypt_env_str)
		ಹಿಂತಿರುಗಿ 0;

	/* We expect the stop-string to be newline-terminated */
	ಮಾಡು {
		ಆದರೆ (tstc()) {
			/* Check for input string overflow */
			ಆದರೆ (presskey_len >= ಗಾತ್ರ(presskey))
				ಹಿಂತಿರುಗಿ 0;

			presskey[presskey_len] = getchar();

			ಆದರೆ ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					ಮುಂದುವರಿ;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				ಆದರೆ (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				presskey_len++;
			}
		}
		udelay(10000);
	} ಆಗ (never_timeout || get_ticks() <= etime);

	ಹಿಂತಿರುಗಿ abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ slow_equals(u8 *a, u8 *b, ಪೂರ್ಣಾಂಕ len)
{
	ಪೂರ್ಣಾಂಕ diff = 0;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	ಹಿಂತಿರುಗಿ diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ passwd_abort_sha256(uint64_t etime)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_ಮೊತ್ತ_LEN];
	u8 *sha;
	ಅಕ್ಷರ *presskey;
	ಅಕ್ಷರ *c;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *algo_name = "sha256";
	u_int presskey_len = 0;
	ಪೂರ್ಣಾಂಕ abort = 0;
	ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ಗಾತ್ರ(sha);
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (sha_env_str == ಖಾಲಿ)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ);
	ಆದರೆ (!presskey)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	c = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(sha_env_str, ":");
	ಆದರೆ (c && (c - sha_env_str < DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ)) {
		/* preload presskey with salt */
		ಸ್ಮರಣೆ_ನಕಲು(presskey, sha_env_str, c - sha_env_str);
		presskey_len = c - sha_env_str;
		sha_env_str = c + 1;
	}
	/*
	 * Generate the binary value from the environment hash value
	 * so that we can compare this value with the computed hash
	 * from the user input
	 */
	ret = hash_parse_string(algo_name, sha_env_str, sha_env);
	ಆದರೆ (ret) {
		ಬರೆ("Hash %s not supported!\n", algo_name);
		ಬಿಡುಗಡೆ(presskey);
		ಹಿಂತಿರುಗಿ 0;
	}

	sha = malloc_cache_aligned(SHA256_ಮೊತ್ತ_LEN);
	ಆದರೆ (!sha) {
		ಬಿಡುಗಡೆ(presskey);
		ಹಿಂತಿರುಗಿ -ENOMEM;
	}
	ಗಾತ್ರ_2 = SHA256_ಮೊತ್ತ_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	ಮಾಡು {
		ಆದರೆ (tstc()) {
			/* Check for input string overflow */
			ಆದರೆ (presskey_len >= DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ) {
				ಬಿಡುಗಡೆ(presskey);
				ಬಿಡುಗಡೆ(sha);
				ಹಿಂತಿರುಗಿ 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_ಖಂಡ(algo_name, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)presskey,
				   presskey_len, sha, &ಗಾತ್ರ_2);

			/* And check if sha matches saved value in env */
			ಆದರೆ (slow_equals(sha, sha_env, SHA256_ಮೊತ್ತ_LEN))
				abort = 1;
		}
		udelay(10000);
	} ಆಗ (!abort && get_ticks() <= etime);

	ಬಿಡುಗಡೆ(presskey);
	ಬಿಡುಗಡೆ(sha);
	ಹಿಂತಿರುಗಿ abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ passwd_abort_ಕೀಲಿ(uint64_t etime)
{
	ಪೂರ್ಣಾಂಕ abort = 0;
	ರಚನೆ {
		ಅಕ್ಷರ *str;
		u_int len;
		ಪೂರ್ಣಾಂಕ retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	ಅಕ್ಷರ presskey[DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ];
	ಪೂರ್ಣಾಂಕ presskey_len = 0;
	ಪೂರ್ಣಾಂಕ presskey_max = 0;
	ಪೂರ್ಣಾಂಕ i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	ಆದರೆ (delaykey[0].str == ಖಾಲಿ)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	ಆದರೆ (delaykey[1].str == ಖಾಲಿ)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	ಗಾಗಿ (i = 0; i < ಗಾತ್ರ(delaykey) / ಗಾತ್ರ(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ಖಾಲಿ ?
				    0 : ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ ?
				    DELAY_STOP_STR_MAX_ಉದ್ದಳತೆ : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	ಮಾಡು {
		ಆದರೆ (tstc()) {
			ಆದರೆ (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಗಾಗಿ (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		ಗಾಗಿ (i = 0; i < ಗಾತ್ರ(delaykey) / ಗಾತ್ರ(delaykey[0]); i++) {
			ಆದರೆ (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				ಆದರೆ (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} ಆಗ (!abort && get_ticks() <= etime);

	ಹಿಂತಿರುಗಿ abort;
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
ಸ್ಥಿರ bool fallback_ಗಮ್ಯ_sha256(ಶೂನ್ಯ)
{
	ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		ಹಿಂತಿರುಗಿ env_get_yesno("bootstopusesha256") == 1;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_ENABLED(CONFIG_CRYPT_PW))
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ abortboot_ಕೀಲಿ_sequence(ಪೂರ್ಣಾಂಕ bootdelay)
{
	ಪೂರ್ಣಾಂಕ abort;
	uint64_t etime = endtick(bootdelay);

	ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	ಬರೆ(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		ಆದರೆ (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_ಗಮ್ಯ_sha256())
			abort = passwd_abort_crypt(etime);
		ಇಲ್ಲದಿದ್ದರೆ
			abort = passwd_abort_sha256(etime);
	} ಇಲ್ಲದಿದ್ದರೆ {
		abort = passwd_abort_ಕೀಲಿ(etime);
	}
	ಆದರೆ (!abort)
		debug_bootkeys("key timeout\n");

	ಹಿಂತಿರುಗಿ abort;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_boot_delay(ಪೂರ್ಣಾಂಕ bootdelay)
{
	ಬರೆ(ANSI_CLEAR_ಸಾಲು "\rHit any key to stop autoboot: %d", bootdelay);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ abortboot_single_ಕೀಲಿ(ಪೂರ್ಣಾಂಕ bootdelay)
{
	ಪೂರ್ಣಾಂಕ abort = 0;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	ಆದರೆ (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	ಆಗ ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		ಮಾಡು {
			ಆದರೆ (tstc()) {	/* we got a key press	*/
				ಪೂರ್ಣಾಂಕ ಕೀಲಿ;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				ಕೀಲಿ = getchar();/* consume input	*/
				ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = ಕೀಲಿ;
				ನಿಲ್ಲಿಸು;
			}
			udelay(10000);
		} ಆಗ (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	ಹಿಂತಿರುಗಿ abort;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ abortboot(ಪೂರ್ಣಾಂಕ bootdelay)
{
	ಪೂರ್ಣಾಂಕ abort = 0;

	ಆದರೆ (bootdelay >= 0) {
		ಆದರೆ (autoboot_keyed())
			abort = abortboot_ಕೀಲಿ_sequence(bootdelay);
		ಇಲ್ಲದಿದ್ದರೆ
			abort = abortboot_single_ಕೀಲಿ(bootdelay);
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	ಹಿಂತಿರುಗಿ abort;
}

ಸ್ಥಿರ ಶೂನ್ಯ process_fdt_options(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ಪಠ್ಯ_ತಳ
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	ಆದರೆ (addr)
		env_ಗಣ_addr("kernaddr", (ಶೂನ್ಯ *)(CONFIG_ಪಠ್ಯ_ತಳ + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	ಆದರೆ (addr)
		env_ಗಣ_addr("rootaddr", (ಶೂನ್ಯ *)(CONFIG_ಪಠ್ಯ_ತಳ + addr));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TEXT_BASE */
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *bootdelay_process(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *s;
	ಪೂರ್ಣಾಂಕ bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (ಪೂರ್ಣಾಂಕ)simple_strtol(s, ಖಾಲಿ, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_ದ_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POST
	ಆದರೆ (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POST */
	ಆದರೆ (bootcount_error())
		s = env_get("altbootcmd");
	ಇಲ್ಲದಿದ್ದರೆ
		s = env_get("bootcmd");

	ಆದರೆ (IS_ENABLED(CONFIG_ದ_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	ಹಿಂತಿರುಗಿ s;
}

ಶೂನ್ಯ autoboot_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	ಆದರೆ (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool ಬೀಗ;
		ಪೂರ್ಣಾಂಕ prev;

		ಬೀಗ = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		ಆದರೆ (ಬೀಗ)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		ಸತತ_command_list(s, -1, 0);

		ಆದರೆ (ಬೀಗ)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	ಆದರೆ (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		ಆದರೆ (s)
			ಸತತ_command_list(s, -1, 0);
	}
}
