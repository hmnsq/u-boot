// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ضمن <config.h>
#ضمن <ansi.h>
#ضمن <autoboot.h>
#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <command.h>
#ضمن <console.h>
#ضمن <env.h>
#ضمن <errno.h>
#ضمن <fdtdec.h>
#ضمن <hash.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <memalign.h>
#ضمن <menu.h>
#ضمن <post.h>
#ضمن <time.h>
#ضمن <asm/global_data.h>
#ضمن <linux/delay.h>
#ضمن <u-boot/sha256.h>
#ضمن <bootcount.h>
#ضمن <crypt.h>
#ضمن <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ساكن صحيح stored_bootdelay;
ساكن صحيح menukey;

#𐤀𐤌 defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#وإلا
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#𐤀𐤌 defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#وإلا
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#وإلا
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
ساكن صحيح passwd_abort_crypt(uint64_t etime)
{
	ثابت حرف *crypt_env_str = env_get("bootstopkeycrypt");
	حرف presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	صحيح abort = 0;
	صحيح never_timeout = 0;
	صحيح err;

	𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	𐤀𐤌 (!crypt_env_str)
		أرجع 0;

	/* We expect the stop-string to be newline-terminated */
	𐤐𐤏𐤋 {
		𐤀𐤌 (tstc()) {
			/* Check for input string overflow */
			𐤀𐤌 (presskey_len >= حجم(presskey))
				أرجع 0;

			presskey[presskey_len] = getchar();

			𐤀𐤌 ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					استمر;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				𐤀𐤌 (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				اكسر;
			} وإلا {
				presskey_len++;
			}
		}
		udelay(10000);
	} طالما (never_timeout || get_ticks() <= etime);

	أرجع abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
ساكن صحيح slow_equals(u8 *a, u8 *b, صحيح len)
{
	صحيح diff = 0;
	صحيح i;

	لكل (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	أرجع diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ساكن صحيح passwd_abort_sha256(uint64_t etime)
{
	ثابت حرف *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	حرف *presskey;
	حرف *c;
	ثابت حرف *algo_name = "sha256";
	u_int presskey_len = 0;
	صحيح abort = 0;
	صحيح size = حجم(sha);
	صحيح ret;

	𐤀𐤌 (sha_env_str == عدم)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	𐤀𐤌 (!presskey)
		أرجع -ENOMEM;

	c = strstr(sha_env_str, ":");
	𐤀𐤌 (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	𐤀𐤌 (ret) {
		printf("Hash %s not supported!\n", algo_name);
		حرر(presskey);
		أرجع 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	𐤀𐤌 (!sha) {
		حرر(presskey);
		أرجع -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	𐤐𐤏𐤋 {
		𐤀𐤌 (tstc()) {
			/* Check for input string overflow */
			𐤀𐤌 (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				حرر(presskey);
				حرر(sha);
				أرجع 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (ثابت فراغ *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			𐤀𐤌 (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} طالما (!abort && get_ticks() <= etime);

	حرر(presskey);
	حرر(sha);
	أرجع abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ساكن صحيح passwd_abort_key(uint64_t etime)
{
	صحيح abort = 0;
	بنية {
		حرف *str;
		u_int len;
		صحيح retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	حرف presskey[DELAY_STOP_STR_MAX_LENGTH];
	صحيح presskey_len = 0;
	صحيح presskey_max = 0;
	صحيح i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	𐤀𐤌 (delaykey[0].str == عدم)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	𐤀𐤌 (delaykey[1].str == عدم)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	لكل (i = 0; i < حجم(delaykey) / حجم(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == عدم ?
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
	𐤐𐤏𐤋 {
		𐤀𐤌 (tstc()) {
			𐤀𐤌 (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} وإلا {
				لكل (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		لكل (i = 0; i < حجم(delaykey) / حجم(delaykey[0]); i++) {
			𐤀𐤌 (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				𐤀𐤌 (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} طالما (!abort && get_ticks() <= etime);

	أرجع abort;
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
ساكن bool fallback_to_sha256(فراغ)
{
	𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		أرجع env_get_yesno("bootstopusesha256") == 1;
	وإلا 𐤀𐤌 (IS_ENABLED(CONFIG_CRYPT_PW))
		أرجع false;
	وإلا
		أرجع true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ساكن صحيح abortboot_key_sequence(صحيح bootdelay)
{
	صحيح abort;
	uint64_t etime = endtick(bootdelay);

	𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		𐤀𐤌 (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		وإلا
			abort = passwd_abort_sha256(etime);
	} وإلا {
		abort = passwd_abort_key(etime);
	}
	𐤀𐤌 (!abort)
		debug_bootkeys("key timeout\n");

	أرجع abort;
}

ساكن فراغ print_boot_delay(صحيح bootdelay)
{
	printf(ANSI_CLEAR_LINE "\rHit any key to stop autoboot: %d", bootdelay);
}

ساكن صحيح abortboot_single_key(صحيح bootdelay)
{
	صحيح abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	𐤀𐤌 (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	طالما ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		𐤐𐤏𐤋 {
			𐤀𐤌 (tstc()) {	/* we got a key press	*/
				صحيح key;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				key = getchar();/* consume input	*/
				𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = key;
				اكسر;
			}
			udelay(10000);
		} طالما (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	أرجع abort;
}

ساكن صحيح abortboot(صحيح bootdelay)
{
	صحيح abort = 0;

	𐤀𐤌 (bootdelay >= 0) {
		𐤀𐤌 (autoboot_keyed())
			abort = abortboot_key_sequence(bootdelay);
		وإلا
			abort = abortboot_single_key(bootdelay);
	}

	𐤀𐤌 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	أرجع abort;
}

ساكن فراغ process_fdt_options(فراغ)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	𐤀𐤌 (addr)
		env_set_addr("kernaddr", (فراغ *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	𐤀𐤌 (addr)
		env_set_addr("rootaddr", (فراغ *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

ثابت حرف *bootdelay_process(فراغ)
{
	حرف *s;
	صحيح bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (صحيح)simple_strtol(s, عدم, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	𐤀𐤌 (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	𐤀𐤌 (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} وإلا
#endif /* CONFIG_POST */
	𐤀𐤌 (bootcount_error())
		s = env_get("altbootcmd");
	وإلا
		s = env_get("bootcmd");

	𐤀𐤌 (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	أرجع s;
}

فراغ autoboot_command(ثابت حرف *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	𐤀𐤌 (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		صحيح prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		𐤀𐤌 (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		𐤀𐤌 (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	𐤀𐤌 (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		𐤀𐤌 (s)
			run_command_list(s, -1, 0);
	}
}
