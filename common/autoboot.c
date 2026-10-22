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

DECLARE_GLOBAL_ڈیٹا_PTR;

#تعریف_کرو DELAY_STOP_STR_MAX_لمبائی 64

#اگر_غیر_تعریف_شدہ DEBUG_BOOTKEYS
#تعریف_کرو DEBUG_BOOTKEYS 0
#اگر_ختم
#تعریف_کرو debug_bootkeys(fmt, دلائل...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ساکن صحیح stored_bootdelay;
ساکن صحیح menukey;

#اگر defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#تعریف_کرو AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#ورنہ
#تعریف_کرو AUTOBOOT_STOP_STR_CRYPT	""
#اگر_ختم
#اگر defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#تعریف_کرو AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#ورنہ
#تعریف_کرو AUTOBOOT_STOP_STR_SHA256	""
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_AUTOBOOT_USE_MENUKEY
#تعریف_کرو AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#ورنہ
#تعریف_کرو AUTOBOOT_MENUKEY 0
#اگر_ختم

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
ساکن صحیح passwd_abort_crypt(uint64_t etime)
{
	ثابت حرف *crypt_env_str = env_get("bootstopkeycrypt");
	حرف presskey[DELAY_STOP_STR_MAX_لمبائی];
	u_int presskey_len = 0;
	صحیح abort = 0;
	صحیح never_timeout = 0;
	صحیح err;

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	اگر (!crypt_env_str)
		واپس 0;

	/* We expect the stop-string to be newline-terminated */
	کرو {
		اگر (tstc()) {
			/* Check for input string overflow */
			اگر (presskey_len >= حجم(presskey))
				واپس 0;

			presskey[presskey_len] = getchar();

			اگر ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				اگر (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					جاری;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				اگر (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				توڑو;
			} ورنہ {
				presskey_len++;
			}
		}
		udelay(10000);
	} جب (never_timeout || get_ticks() <= etime);

	واپس abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
ساکن صحیح slow_equals(u8 *a, u8 *b, صحیح len)
{
	صحیح diff = 0;
	صحیح i;

	برائے (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	واپس diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ساکن صحیح passwd_abort_sha256(uint64_t etime)
{
	ثابت حرف *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_مجموع_LEN];
	u8 *sha;
	حرف *presskey;
	حرف *c;
	ثابت حرف *algo_name = "sha256";
	u_int presskey_len = 0;
	صحیح abort = 0;
	صحیح سائز = حجم(sha);
	صحیح ret;

	اگر (sha_env_str == تہی)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_لمبائی);
	اگر (!presskey)
		واپس -ENOMEM;

	c = لڑی_تلاش(sha_env_str, ":");
	اگر (c && (c - sha_env_str < DELAY_STOP_STR_MAX_لمبائی)) {
		/* preload presskey with salt */
		حافظہ_نقل(presskey, sha_env_str, c - sha_env_str);
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
		لکھو("Hash %s not supported!\n", algo_name);
		آزاد(presskey);
		واپس 0;
	}

	sha = malloc_cache_aligned(SHA256_مجموع_LEN);
	اگر (!sha) {
		آزاد(presskey);
		واپس -ENOMEM;
	}
	سائز = SHA256_مجموع_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	کرو {
		اگر (tstc()) {
			/* Check for input string overflow */
			اگر (presskey_len >= DELAY_STOP_STR_MAX_لمبائی) {
				آزاد(presskey);
				آزاد(sha);
				واپس 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_بلاک(algo_name, (ثابت خالی *)presskey,
				   presskey_len, sha, &سائز);

			/* And check if sha matches saved value in env */
			اگر (slow_equals(sha, sha_env, SHA256_مجموع_LEN))
				abort = 1;
		}
		udelay(10000);
	} جب (!abort && get_ticks() <= etime);

	آزاد(presskey);
	آزاد(sha);
	واپس abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
ساکن صحیح passwd_abort_کلید(uint64_t etime)
{
	صحیح abort = 0;
	ساخت {
		حرف *str;
		u_int len;
		صحیح retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	حرف presskey[DELAY_STOP_STR_MAX_لمبائی];
	صحیح presskey_len = 0;
	صحیح presskey_max = 0;
	صحیح i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	اگر (delaykey[0].str == تہی)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	اگر (delaykey[1].str == تہی)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	برائے (i = 0; i < حجم(delaykey) / حجم(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == تہی ?
				    0 : لڑی_لمبائی(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_لمبائی ?
				    DELAY_STOP_STR_MAX_لمبائی : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	کرو {
		اگر (tstc()) {
			اگر (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} ورنہ {
				برائے (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		برائے (i = 0; i < حجم(delaykey) / حجم(delaykey[0]); i++) {
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
	} جب (!abort && get_ticks() <= etime);

	واپس abort;
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
ساکن bool fallback_مقصد_sha256(خالی)
{
	اگر (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		واپس env_get_yesno("bootstopusesha256") == 1;
	ورنہ اگر (IS_ENABLED(CONFIG_CRYPT_PW))
		واپس false;
	ورنہ
		واپس true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ساکن صحیح abortboot_کلید_sequence(صحیح bootdelay)
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
	لکھو(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		اگر (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_مقصد_sha256())
			abort = passwd_abort_crypt(etime);
		ورنہ
			abort = passwd_abort_sha256(etime);
	} ورنہ {
		abort = passwd_abort_کلید(etime);
	}
	اگر (!abort)
		debug_bootkeys("key timeout\n");

	واپس abort;
}

ساکن خالی print_boot_delay(صحیح bootdelay)
{
	لکھو(ANSI_CLEAR_سطر "\rHit any key to stop autoboot: %d", bootdelay);
}

ساکن صحیح abortboot_single_کلید(صحیح bootdelay)
{
	صحیح abort = 0;
	غیرعلامتی طویل ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	اگر (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	جب ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		کرو {
			اگر (tstc()) {	/* we got a key press	*/
				صحیح کلید;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				کلید = getchar();/* consume input	*/
				اگر (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = کلید;
				توڑو;
			}
			udelay(10000);
		} جب (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	واپس abort;
}

ساکن صحیح abortboot(صحیح bootdelay)
{
	صحیح abort = 0;

	اگر (bootdelay >= 0) {
		اگر (autoboot_keyed())
			abort = abortboot_کلید_sequence(bootdelay);
		ورنہ
			abort = abortboot_single_کلید(bootdelay);
	}

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	واپس abort;
}

ساکن خالی process_fdt_options(خالی)
{
#اگر_تعریف_شدہ CONFIG_متن_بنیاد
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	اگر (addr)
		env_سیٹ_addr("kernaddr", (خالی *)(CONFIG_متن_بنیاد + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	اگر (addr)
		env_سیٹ_addr("rootaddr", (خالی *)(CONFIG_متن_بنیاد + addr));
#اگر_ختم /* CONFIG_TEXT_BASE */
}

ثابت حرف *bootdelay_process(خالی)
{
	حرف *s;
	صحیح bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (صحیح)simple_strtol(s, تہی, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	اگر (IS_ENABLED(CONFIG_کا_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#اگر_تعریف_شدہ CONFIG_POST
	اگر (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} ورنہ
#اگر_ختم /* CONFIG_POST */
	اگر (bootcount_error())
		s = env_get("altbootcmd");
	ورنہ
		s = env_get("bootcmd");

	اگر (IS_ENABLED(CONFIG_کا_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	واپس s;
}

خالی autoboot_command(ثابت حرف *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	اگر (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool تالا;
		صحیح پچھلا;

		تالا = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		اگر (تالا)
			پچھلا = disable_ctrlc(1); /* disable Ctrl-C checking */

		تسلسل_command_list(s, -1, 0);

		اگر (تالا)
			disable_ctrlc(پچھلا);	/* restore Ctrl-C checking */
	}

	اگر (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		اگر (s)
			تسلسل_command_list(s, -1, 0);
	}
}
