// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <ansi.h>
#अन्तर्भाव <autoboot.h>
#अन्तर्भाव <bootretry.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <hash.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <memalign.h>
#अन्तर्भाव <menu.h>
#अन्तर्भाव <post.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/delay.h>
#अन्तर्भाव <u-boot/sha256.h>
#अन्तर्भाव <bootcount.h>
#अन्तर्भाव <crypt.h>
#अन्तर्भाव <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
स्थिर पूर्णांक stored_bootdelay;
स्थिर पूर्णांक menukey;

#जर defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#अन्यथा
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#जर defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#अन्यथा
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#अन्यथा
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
स्थिर पूर्णांक passwd_abort_crypt(uint64_t etime)
{
	स्थिरांक अक्षर *crypt_env_str = env_get("bootstopkeycrypt");
	अक्षर presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	पूर्णांक पात = 0;
	पूर्णांक never_timeout = 0;
	पूर्णांक err;

	जर (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	जर (!crypt_env_str)
		परत 0;

	/* We expect the stop-string to be newline-terminated */
	करा {
		जर (tstc()) {
			/* Check for input string overflow */
			जर (presskey_len >= आकार(presskey))
				परत 0;

			presskey[presskey_len] = अक्षर_लो();

			जर ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				जर (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					सुरूठेवा;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &पात);
				जर (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				थांबा;
			} अन्यथा {
				presskey_len++;
			}
		}
		udelay(10000);
	} पर्यंत (never_timeout || get_ticks() <= etime);

	परत पात;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
स्थिर पूर्णांक slow_equals(u8 *a, u8 *b, पूर्णांक len)
{
	पूर्णांक diff = 0;
	पूर्णांक i;

	साठी (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	परत diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
स्थिर पूर्णांक passwd_abort_sha256(uint64_t etime)
{
	स्थिरांक अक्षर *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	अक्षर *presskey;
	अक्षर *c;
	स्थिरांक अक्षर *algo_name = "sha256";
	u_int presskey_len = 0;
	पूर्णांक पात = 0;
	पूर्णांक size = आकार(sha);
	पूर्णांक ret;

	जर (sha_env_str == रिक्त)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	जर (!presskey)
		परत -ENOMEM;

	c = म_माला(sha_env_str, ":");
	जर (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
		/* preload presskey with salt */
		स_नकल(presskey, sha_env_str, c - sha_env_str);
		presskey_len = c - sha_env_str;
		sha_env_str = c + 1;
	}
	/*
	 * Generate the binary value from the environment hash value
	 * so that we can compare this value with the computed hash
	 * from the user input
	 */
	ret = hash_parse_string(algo_name, sha_env_str, sha_env);
	जर (ret) {
		म_लिखो("Hash %s not supported!\n", algo_name);
		मुक्त(presskey);
		परत 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	जर (!sha) {
		मुक्त(presskey);
		परत -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	करा {
		जर (tstc()) {
			/* Check for input string overflow */
			जर (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				मुक्त(presskey);
				मुक्त(sha);
				परत 0;
			}

			presskey[presskey_len++] = अक्षर_लो();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (स्थिरांक शून्य *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			जर (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				पात = 1;
		}
		udelay(10000);
	} पर्यंत (!पात && get_ticks() <= etime);

	मुक्त(presskey);
	मुक्त(sha);
	परत पात;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
स्थिर पूर्णांक passwd_abort_किल्ली(uint64_t etime)
{
	पूर्णांक पात = 0;
	संरचना {
		अक्षर *str;
		u_int len;
		पूर्णांक retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	अक्षर presskey[DELAY_STOP_STR_MAX_LENGTH];
	पूर्णांक presskey_len = 0;
	पूर्णांक presskey_max = 0;
	पूर्णांक i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	जर (delaykey[0].str == रिक्त)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	जर (delaykey[1].str == रिक्त)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	साठी (i = 0; i < आकार(delaykey) / आकार(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == रिक्त ?
				    0 : म_माप(delaykey[i].str);
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
	करा {
		जर (tstc()) {
			जर (presskey_len < presskey_max) {
				presskey[presskey_len++] = अक्षर_लो();
			} अन्यथा {
				साठी (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = अक्षर_लो();
			}
		}

		साठी (i = 0; i < आकार(delaykey) / आकार(delaykey[0]); i++) {
			जर (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				स_भेद(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				जर (!delaykey[i].retry)
					bootretry_dont_retry();
				पात = 1;
			}
		}
		udelay(10000);
	} पर्यंत (!पात && get_ticks() <= etime);

	परत पात;
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
स्थिर bool fallback_to_sha256(शून्य)
{
	जर (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		परत env_get_yesno("bootstopusesha256") == 1;
	अन्यथा जर (IS_ENABLED(CONFIG_CRYPT_PW))
		परत false;
	अन्यथा
		परत true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
स्थिर पूर्णांक abortboot_किल्ली_sequence(पूर्णांक bootdelay)
{
	पूर्णांक पात;
	uint64_t etime = endtick(bootdelay);

	जर (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	म_लिखो(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	जर (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		जर (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			पात = passwd_abort_crypt(etime);
		अन्यथा
			पात = passwd_abort_sha256(etime);
	} अन्यथा {
		पात = passwd_abort_किल्ली(etime);
	}
	जर (!पात)
		debug_bootkeys("key timeout\n");

	परत पात;
}

स्थिर शून्य print_boot_delay(पूर्णांक bootdelay)
{
	म_लिखो(ANSI_CLEAR_ओळ "\rHit any key to stop autoboot: %d", bootdelay);
}

स्थिर पूर्णांक abortboot_single_किल्ली(पूर्णांक bootdelay)
{
	पूर्णांक पात = 0;
	अचिन्हित दीर्घ ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	जर (tstc()) {	/* we got a key press	*/
		अक्षर_लो();	/* consume input	*/
		print_boot_delay(0);
		पात = 1;	/* don't auto boot	*/
	}

	पर्यंत ((bootdelay > 0) && (!पात)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		करा {
			जर (tstc()) {	/* we got a key press	*/
				पूर्णांक किल्ली;

				पात  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				किल्ली = अक्षर_लो();/* consume input	*/
				जर (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = किल्ली;
				थांबा;
			}
			udelay(10000);
		} पर्यंत (!पात && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	अ_दो('\n');

	परत पात;
}

स्थिर पूर्णांक abortboot(पूर्णांक bootdelay)
{
	पूर्णांक पात = 0;

	जर (bootdelay >= 0) {
		जर (autoboot_keyed())
			पात = abortboot_किल्ली_sequence(bootdelay);
		अन्यथा
			पात = abortboot_single_किल्ली(bootdelay);
	}

	जर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && पात)
		gd->flags &= ~GD_FLG_SILENT;

	परत पात;
}

स्थिर शून्य process_fdt_options(शून्य)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	जर (addr)
		env_set_addr("kernaddr", (शून्य *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	जर (addr)
		env_set_addr("rootaddr", (शून्य *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

स्थिरांक अक्षर *bootdelay_process(शून्य)
{
	अक्षर *s;
	पूर्णांक bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (पूर्णांक)simple_strtol(s, रिक्त, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	जर (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	जर (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	जर (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} अन्यथा
#endif /* CONFIG_POST */
	जर (bootcount_error())
		s = env_get("altbootcmd");
	अन्यथा
		s = env_get("bootcmd");

	जर (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	परत s;
}

शून्य autoboot_command(स्थिरांक अक्षर *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	जर (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		पूर्णांक prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		जर (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		जर (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	जर (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		जर (s)
			run_command_list(s, -1, 0);
	}
}
