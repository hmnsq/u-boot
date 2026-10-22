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

DECLARE_GLOBAL_معلومات_PTR;

#وصف_ڪريو DELAY_STOP_STR_MAX_ڊيگهائي 64

#جيڪڏهن_وصف_نه_ٿيل DEBUG_BOOTKEYS
#وصف_ڪريو DEBUG_BOOTKEYS 0
#جيڪڏهن_پورو
#وصف_ڪريو debug_bootkeys(fmt, دليلون...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
ساكن صحيح stored_bootdelay;
ساكن صحيح menukey;

#إذا defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#وصف_ڪريو AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#وإلا
#وصف_ڪريو AUTOBOOT_STOP_STR_CRYPT	""
#جيڪڏهن_پورو
#إذا defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#وصف_ڪريو AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#وإلا
#وصف_ڪريو AUTOBOOT_STOP_STR_SHA256	""
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_AUTOBOOT_USE_MENUKEY
#وصف_ڪريو AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#وإلا
#وصف_ڪريو AUTOBOOT_MENUKEY 0
#جيڪڏهن_پورو

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
	حرف presskey[DELAY_STOP_STR_MAX_ڊيگهائي];
	u_int presskey_len = 0;
	صحيح abort = 0;
	صحيح never_timeout = 0;
	صحيح err;

	إذا (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	إذا (!crypt_env_str)
		أرجع 0;

	/* We expect the stop-string to be newline-terminated */
	افعل {
		إذا (tstc()) {
			/* Check for input string overflow */
			إذا (presskey_len >= حجم(presskey))
				أرجع 0;

			presskey[presskey_len] = getchar();

			إذا ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				إذا (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					استمر;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				إذا (err)
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
	u8 sha_env[SHA256_جوڙ_LEN];
	u8 *sha;
	حرف *presskey;
	حرف *c;
	ثابت حرف *algo_name = "sha256";
	u_int presskey_len = 0;
	صحيح abort = 0;
	صحيح سائيز = حجم(sha);
	صحيح ret;

	إذا (sha_env_str == عدم)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_ڊيگهائي);
	إذا (!presskey)
		أرجع -ENOMEM;

	c = لڙي_ڳولا(sha_env_str, ":");
	إذا (c && (c - sha_env_str < DELAY_STOP_STR_MAX_ڊيگهائي)) {
		/* preload presskey with salt */
		يادگيري_نقل(presskey, sha_env_str, c - sha_env_str);
		presskey_len = c - sha_env_str;
		sha_env_str = c + 1;
	}
	/*
	 * Generate the binary value from the environment hash value
	 * so that we can compare this value with the computed hash
	 * from the user input
	 */
	ret = hash_parse_string(algo_name, sha_env_str, sha_env);
	إذا (ret) {
		لکو("Hash %s not supported!\n", algo_name);
		حرر(presskey);
		أرجع 0;
	}

	sha = malloc_cache_aligned(SHA256_جوڙ_LEN);
	إذا (!sha) {
		حرر(presskey);
		أرجع -ENOMEM;
	}
	سائيز = SHA256_جوڙ_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	افعل {
		إذا (tstc()) {
			/* Check for input string overflow */
			إذا (presskey_len >= DELAY_STOP_STR_MAX_ڊيگهائي) {
				حرر(presskey);
				حرر(sha);
				أرجع 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_بلاڪ(algo_name, (ثابت فراغ *)presskey,
				   presskey_len, sha, &سائيز);

			/* And check if sha matches saved value in env */
			إذا (slow_equals(sha, sha_env, SHA256_جوڙ_LEN))
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
ساكن صحيح passwd_abort_ڪنجي(uint64_t etime)
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

	حرف presskey[DELAY_STOP_STR_MAX_ڊيگهائي];
	صحيح presskey_len = 0;
	صحيح presskey_max = 0;
	صحيح i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	إذا (delaykey[0].str == عدم)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	إذا (delaykey[1].str == عدم)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	لكل (i = 0; i < حجم(delaykey) / حجم(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == عدم ?
				    0 : لڙي_ڊيگهه(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_ڊيگهائي ?
				    DELAY_STOP_STR_MAX_ڊيگهائي : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	افعل {
		إذا (tstc()) {
			إذا (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} وإلا {
				لكل (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		لكل (i = 0; i < حجم(delaykey) / حجم(delaykey[0]); i++) {
			إذا (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				إذا (!delaykey[i].retry)
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
ساكن bool fallback_منزل_sha256(فراغ)
{
	إذا (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		أرجع env_get_yesno("bootstopusesha256") == 1;
	وإلا إذا (IS_ENABLED(CONFIG_CRYPT_PW))
		أرجع false;
	وإلا
		أرجع true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
ساكن صحيح abortboot_ڪنجي_sequence(صحيح bootdelay)
{
	صحيح abort;
	uint64_t etime = endtick(bootdelay);

	إذا (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	لکو(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	إذا (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		إذا (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_منزل_sha256())
			abort = passwd_abort_crypt(etime);
		وإلا
			abort = passwd_abort_sha256(etime);
	} وإلا {
		abort = passwd_abort_ڪنجي(etime);
	}
	إذا (!abort)
		debug_bootkeys("key timeout\n");

	أرجع abort;
}

ساكن فراغ print_boot_delay(صحيح bootdelay)
{
	لکو(ANSI_CLEAR_سٽ "\rHit any key to stop autoboot: %d", bootdelay);
}

ساكن صحيح abortboot_single_ڪنجي(صحيح bootdelay)
{
	صحيح abort = 0;
	بنانشان ڊگهو ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	إذا (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	طالما ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		افعل {
			إذا (tstc()) {	/* we got a key press	*/
				صحيح ڪنجي;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				ڪنجي = getchar();/* consume input	*/
				إذا (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = ڪنجي;
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

	إذا (bootdelay >= 0) {
		إذا (autoboot_keyed())
			abort = abortboot_ڪنجي_sequence(bootdelay);
		وإلا
			abort = abortboot_single_ڪنجي(bootdelay);
	}

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	أرجع abort;
}

ساكن فراغ process_fdt_options(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_متن_بنياد
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	إذا (addr)
		env_سيٽ_addr("kernaddr", (فراغ *)(CONFIG_متن_بنياد + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	إذا (addr)
		env_سيٽ_addr("rootaddr", (فراغ *)(CONFIG_متن_بنياد + addr));
#جيڪڏهن_پورو /* CONFIG_TEXT_BASE */
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
	إذا (IS_ENABLED(CONFIG_جو_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	إذا (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#جيڪڏهن_وصف_ٿيل CONFIG_POST
	إذا (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} وإلا
#جيڪڏهن_پورو /* CONFIG_POST */
	إذا (bootcount_error())
		s = env_get("altbootcmd");
	وإلا
		s = env_get("bootcmd");

	إذا (IS_ENABLED(CONFIG_جو_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	أرجع s;
}

فراغ autoboot_command(ثابت حرف *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	إذا (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool تالو;
		صحيح prev;

		تالو = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		إذا (تالو)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		لاڳيتو_command_list(s, -1, 0);

		إذا (تالو)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	إذا (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		إذا (s)
			لاڳيتو_command_list(s, -1, 0);
	}
}
