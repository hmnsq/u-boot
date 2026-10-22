// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#شامل <config.h>
#شامل <ansi.h>
#شامل <autoboot.h>
#شامل <bootretry.h>
#شامل <cli.h>
#شامل <command.h>
#شامل <console.h>
#شامل <env.h>
#شامل <errno.h>
#شامل <fdtdec.h>
#شامل <hash.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <memalign.h>
#شامل <menu.h>
#شامل <post.h>
#شامل <time.h>
#شامل <asm/global_data.h>
#شامل <linux/delay.h>
#شامل <u-boot/sha256.h>
#شامل <bootcount.h>
#شامل <crypt.h>
#شامل <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ایستا صحیح stored_bootdelay;
ایستا صحیح menukey;

#اگر defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#وگرنه
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#اگر defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#وگرنه
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#وگرنه
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
ایستا صحیح passwd_abort_crypt(uint64_t etime)
{
	ثابت نویسه *crypt_env_str = env_get("bootstopkeycrypt");
	نویسه presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	صحیح abort = 0;
	صحیح never_timeout = 0;
	صحیح err;

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	اگر (!crypt_env_str)
		بازگشت 0;

	/* We expect the stop-string to be newline-terminated */
	انجامبده {
		اگر (tstc()) {
			/* Check for input string overflow */
			اگر (presskey_len >= اندازه(presskey))
				بازگشت 0;

			presskey[presskey_len] = getchar();

			اگر ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				اگر (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					ادامه;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				اگر (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				بشکن;
			} وگرنه {
				presskey_len++;
			}
		}
		udelay(10000);
	} وقتی (never_timeout || get_ticks() <= etime);

	بازگشت abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
ایستا صحیح slow_equals(u8 *a, u8 *b, صحیح len)
{
	صحیح diff = 0;
	صحیح i;

	برای (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	بازگشت diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ایستا صحیح passwd_abort_sha256(uint64_t etime)
{
	ثابت نویسه *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	نویسه *presskey;
	نویسه *c;
	ثابت نویسه *algo_name = "sha256";
	u_int presskey_len = 0;
	صحیح abort = 0;
	صحیح size = اندازه(sha);
	صحیح ret;

	اگر (sha_env_str == پوچ)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	اگر (!presskey)
		بازگشت -ENOMEM;

	c = strstr(sha_env_str, ":");
	اگر (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	اگر (ret) {
		printf("Hash %s not supported!\n", algo_name);
		آزاد(presskey);
		بازگشت 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	اگر (!sha) {
		آزاد(presskey);
		بازگشت -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	انجامبده {
		اگر (tstc()) {
			/* Check for input string overflow */
			اگر (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				آزاد(presskey);
				آزاد(sha);
				بازگشت 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (ثابت تهی *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			اگر (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} وقتی (!abort && get_ticks() <= etime);

	آزاد(presskey);
	آزاد(sha);
	بازگشت abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ایستا صحیح passwd_abort_کلید(uint64_t etime)
{
	صحیح abort = 0;
	ساختار {
		نویسه *str;
		u_int len;
		صحیح retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	نویسه presskey[DELAY_STOP_STR_MAX_LENGTH];
	صحیح presskey_len = 0;
	صحیح presskey_max = 0;
	صحیح i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	اگر (delaykey[0].str == پوچ)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	اگر (delaykey[1].str == پوچ)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	برای (i = 0; i < اندازه(delaykey) / اندازه(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == پوچ ?
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
	انجامبده {
		اگر (tstc()) {
			اگر (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} وگرنه {
				برای (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		برای (i = 0; i < اندازه(delaykey) / اندازه(delaykey[0]); i++) {
			اگر (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				اگر (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} وقتی (!abort && get_ticks() <= etime);

	بازگشت abort;
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
ایستا bool fallback_to_sha256(تهی)
{
	اگر (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		بازگشت env_get_yesno("bootstopusesha256") == 1;
	وگرنه اگر (IS_ENABLED(CONFIG_CRYPT_PW))
		بازگشت false;
	وگرنه
		بازگشت true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ایستا صحیح abortboot_کلید_sequence(صحیح bootdelay)
{
	صحیح abort;
	uint64_t etime = endtick(bootdelay);

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		اگر (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		وگرنه
			abort = passwd_abort_sha256(etime);
	} وگرنه {
		abort = passwd_abort_کلید(etime);
	}
	اگر (!abort)
		debug_bootkeys("key timeout\n");

	بازگشت abort;
}

ایستا تهی print_boot_delay(صحیح bootdelay)
{
	printf(ANSI_CLEAR_سطر "\rHit any key to stop autoboot: %d", bootdelay);
}

ایستا صحیح abortboot_single_کلید(صحیح bootdelay)
{
	صحیح abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	اگر (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	وقتی ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		انجامبده {
			اگر (tstc()) {	/* we got a key press	*/
				صحیح کلید;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				کلید = getchar();/* consume input	*/
				اگر (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = کلید;
				بشکن;
			}
			udelay(10000);
		} وقتی (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	بازگشت abort;
}

ایستا صحیح abortboot(صحیح bootdelay)
{
	صحیح abort = 0;

	اگر (bootdelay >= 0) {
		اگر (autoboot_keyed())
			abort = abortboot_کلید_sequence(bootdelay);
		وگرنه
			abort = abortboot_single_کلید(bootdelay);
	}

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	بازگشت abort;
}

ایستا تهی process_fdt_options(تهی)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	اگر (addr)
		env_set_addr("kernaddr", (تهی *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	اگر (addr)
		env_set_addr("rootaddr", (تهی *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

ثابت نویسه *bootdelay_process(تهی)
{
	نویسه *s;
	صحیح bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (صحیح)simple_strtol(s, پوچ, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	اگر (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	اگر (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} وگرنه
#endif /* CONFIG_POST */
	اگر (bootcount_error())
		s = env_get("altbootcmd");
	وگرنه
		s = env_get("bootcmd");

	اگر (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	بازگشت s;
}

تهی autoboot_command(ثابت نویسه *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	اگر (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		صحیح prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		اگر (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		اگر (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		اگر (s)
			run_command_list(s, -1, 0);
	}
}
