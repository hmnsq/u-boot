// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <ansi.h>
#ଅନ୍ତର୍ଭାଵ <autoboot.h>
#ଅନ୍ତର୍ଭାଵ <bootretry.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fdtdec.h>
#ଅନ୍ତର୍ଭାଵ <hash.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <memalign.h>
#ଅନ୍ତର୍ଭାଵ <menu.h>
#ଅନ୍ତର୍ଭାଵ <post.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/sha256.h>
#ଅନ୍ତର୍ଭାଵ <bootcount.h>
#ଅନ୍ତର୍ଭାଵ <crypt.h>
#ଅନ୍ତର୍ଭାଵ <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stored_bootdelay;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା menukey;

#ଯଦି defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#ଅନ୍ୟଥା
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#ଯଦି defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#ଅନ୍ୟଥା
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#ଅନ୍ୟଥା
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା passwd_abort_crypt(uint64_t etime)
{
	ଅଚଳ ଅକ୍ଷର *crypt_env_str = env_get("bootstopkeycrypt");
	ଅକ୍ଷର presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା abort = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା never_timeout = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	ଯଦି (!crypt_env_str)
		ଫେରନ୍ତୁ 0;

	/* We expect the stop-string to be newline-terminated */
	କର {
		ଯଦି (tstc()) {
			/* Check for input string overflow */
			ଯଦି (presskey_len >= ଆକାର(presskey))
				ଫେରନ୍ତୁ 0;

			presskey[presskey_len] = getchar();

			ଯଦି ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					ଜାରି;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				ଯଦି (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା {
				presskey_len++;
			}
		}
		udelay(10000);
	} ଯେତେବେଳେ (never_timeout || get_ticks() <= etime);

	ଫେରନ୍ତୁ abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା slow_equals(u8 *a, u8 *b, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା diff = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	ଫେରନ୍ତୁ diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା passwd_abort_sha256(uint64_t etime)
{
	ଅଚଳ ଅକ୍ଷର *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	ଅକ୍ଷର *presskey;
	ଅକ୍ଷର *c;
	ଅଚଳ ଅକ୍ଷର *algo_name = "sha256";
	u_int presskey_len = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା abort = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = ଆକାର(sha);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (sha_env_str == ଖାଲି)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	ଯଦି (!presskey)
		ଫେରନ୍ତୁ -ENOMEM;

	c = strstr(sha_env_str, ":");
	ଯଦି (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	ଯଦି (ret) {
		printf("Hash %s not supported!\n", algo_name);
		ମୁକ୍ତ(presskey);
		ଫେରନ୍ତୁ 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	ଯଦି (!sha) {
		ମୁକ୍ତ(presskey);
		ଫେରନ୍ତୁ -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	କର {
		ଯଦି (tstc()) {
			/* Check for input string overflow */
			ଯଦି (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				ମୁକ୍ତ(presskey);
				ମୁକ୍ତ(sha);
				ଫେରନ୍ତୁ 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (ଅଚଳ ଶୂନ୍ୟ *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			ଯଦି (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} ଯେତେବେଳେ (!abort && get_ticks() <= etime);

	ମୁକ୍ତ(presskey);
	ମୁକ୍ତ(sha);
	ଫେରନ୍ତୁ abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା passwd_abort_ଚାବି(uint64_t etime)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା abort = 0;
	ସଂରଚନା {
		ଅକ୍ଷର *str;
		u_int len;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	ଅକ୍ଷର presskey[DELAY_STOP_STR_MAX_LENGTH];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା presskey_len = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା presskey_max = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	ଯଦି (delaykey[0].str == ଖାଲି)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	ଯଦି (delaykey[1].str == ଖାଲି)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	ପାଇଁ (i = 0; i < ଆକାର(delaykey) / ଆକାର(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == ଖାଲି ?
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
	କର {
		ଯଦି (tstc()) {
			ଯଦି (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} ଅନ୍ୟଥା {
				ପାଇଁ (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		ପାଇଁ (i = 0; i < ଆକାର(delaykey) / ଆକାର(delaykey[0]); i++) {
			ଯଦି (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				ଯଦି (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} ଯେତେବେଳେ (!abort && get_ticks() <= etime);

	ଫେରନ୍ତୁ abort;
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
ସ୍ଥିର bool fallback_to_sha256(ଶୂନ୍ୟ)
{
	ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		ଫେରନ୍ତୁ env_get_yesno("bootstopusesha256") == 1;
	ଅନ୍ୟଥା ଯଦି (IS_ENABLED(CONFIG_CRYPT_PW))
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା abortboot_ଚାବି_sequence(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootdelay)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା abort;
	uint64_t etime = endtick(bootdelay);

	ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		ଯଦି (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		ଅନ୍ୟଥା
			abort = passwd_abort_sha256(etime);
	} ଅନ୍ୟଥା {
		abort = passwd_abort_ଚାବି(etime);
	}
	ଯଦି (!abort)
		debug_bootkeys("key timeout\n");

	ଫେରନ୍ତୁ abort;
}

ସ୍ଥିର ଶୂନ୍ୟ print_boot_delay(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootdelay)
{
	printf(ANSI_CLEAR_ଧାଡ଼ି "\rHit any key to stop autoboot: %d", bootdelay);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା abortboot_single_ଚାବି(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootdelay)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	ଯଦି (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	ଯେତେବେଳେ ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		କର {
			ଯଦି (tstc()) {	/* we got a key press	*/
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				ଚାବି = getchar();/* consume input	*/
				ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = ଚାବି;
				ଭାଙ୍ଗ;
			}
			udelay(10000);
		} ଯେତେବେଳେ (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	ଫେରନ୍ତୁ abort;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା abortboot(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootdelay)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା abort = 0;

	ଯଦି (bootdelay >= 0) {
		ଯଦି (autoboot_keyed())
			abort = abortboot_ଚାବି_sequence(bootdelay);
		ଅନ୍ୟଥା
			abort = abortboot_single_ଚାବି(bootdelay);
	}

	ଯଦି (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	ଫେରନ୍ତୁ abort;
}

ସ୍ଥିର ଶୂନ୍ୟ process_fdt_options(ଶୂନ୍ୟ)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	ଯଦି (addr)
		env_set_addr("kernaddr", (ଶୂନ୍ୟ *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	ଯଦି (addr)
		env_set_addr("rootaddr", (ଶୂନ୍ୟ *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

ଅଚଳ ଅକ୍ଷର *bootdelay_process(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *s;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)simple_strtol(s, ଖାଲି, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	ଯଦି (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	ଯଦି (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} ଅନ୍ୟଥା
#endif /* CONFIG_POST */
	ଯଦି (bootcount_error())
		s = env_get("altbootcmd");
	ଅନ୍ୟଥା
		s = env_get("bootcmd");

	ଯଦି (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	ଫେରନ୍ତୁ s;
}

ଶୂନ୍ୟ autoboot_command(ଅଚଳ ଅକ୍ଷର *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	ଯଦି (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		ଯଦି (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		ଯଦି (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		ଯଦି (s)
			run_command_list(s, -1, 0);
	}
}
