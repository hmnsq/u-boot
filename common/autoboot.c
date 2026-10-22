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
स्थिर पूर्णाङ्क stored_bootdelay;
स्थिर पूर्णाङ्क menukey;

#यदि defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#अन्यथा
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#यदि defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
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
स्थिर पूर्णाङ्क passwd_abort_crypt(uint64_t etime)
{
	अचर वर्ण *crypt_env_str = env_get("bootstopkeycrypt");
	वर्ण presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	पूर्णाङ्क पात = 0;
	पूर्णाङ्क never_timeout = 0;
	पूर्णाङ्क err;

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	यदि (!crypt_env_str)
		फर्काऊ 0;

	/* We expect the stop-string to be newline-terminated */
	गर {
		यदि (tstc()) {
			/* Check for input string overflow */
			यदि (presskey_len >= परिमाण(presskey))
				फर्काऊ 0;

			presskey[presskey_len] = अक्षर_लो();

			यदि ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				यदि (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					जारी;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &पात);
				यदि (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				रोक;
			} अन्यथा {
				presskey_len++;
			}
		}
		udelay(10000);
	} जबसम्म (never_timeout || get_ticks() <= etime);

	फर्काऊ पात;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
स्थिर पूर्णाङ्क slow_equals(u8 *a, u8 *b, पूर्णाङ्क len)
{
	पूर्णाङ्क diff = 0;
	पूर्णाङ्क i;

	लागि (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	फर्काऊ diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
स्थिर पूर्णाङ्क passwd_abort_sha256(uint64_t etime)
{
	अचर वर्ण *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	वर्ण *presskey;
	वर्ण *c;
	अचर वर्ण *algo_name = "sha256";
	u_int presskey_len = 0;
	पूर्णाङ्क पात = 0;
	पूर्णाङ्क size = परिमाण(sha);
	पूर्णाङ्क ret;

	यदि (sha_env_str == रिक्त)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	यदि (!presskey)
		फर्काऊ -ENOMEM;

	c = म_माला(sha_env_str, ":");
	यदि (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	यदि (ret) {
		म_लिखो("Hash %s not supported!\n", algo_name);
		मुक्त(presskey);
		फर्काऊ 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	यदि (!sha) {
		मुक्त(presskey);
		फर्काऊ -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	गर {
		यदि (tstc()) {
			/* Check for input string overflow */
			यदि (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				मुक्त(presskey);
				मुक्त(sha);
				फर्काऊ 0;
			}

			presskey[presskey_len++] = अक्षर_लो();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (अचर शून्य *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			यदि (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				पात = 1;
		}
		udelay(10000);
	} जबसम्म (!पात && get_ticks() <= etime);

	मुक्त(presskey);
	मुक्त(sha);
	फर्काऊ पात;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
स्थिर पूर्णाङ्क passwd_abort_साँचो(uint64_t etime)
{
	पूर्णाङ्क पात = 0;
	संरचना {
		वर्ण *str;
		u_int len;
		पूर्णाङ्क retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	वर्ण presskey[DELAY_STOP_STR_MAX_LENGTH];
	पूर्णाङ्क presskey_len = 0;
	पूर्णाङ्क presskey_max = 0;
	पूर्णाङ्क i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	यदि (delaykey[0].str == रिक्त)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	यदि (delaykey[1].str == रिक्त)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	लागि (i = 0; i < परिमाण(delaykey) / परिमाण(delaykey[0]); i++) {
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
	गर {
		यदि (tstc()) {
			यदि (presskey_len < presskey_max) {
				presskey[presskey_len++] = अक्षर_लो();
			} अन्यथा {
				लागि (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = अक्षर_लो();
			}
		}

		लागि (i = 0; i < परिमाण(delaykey) / परिमाण(delaykey[0]); i++) {
			यदि (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				स_भेद(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				यदि (!delaykey[i].retry)
					bootretry_dont_retry();
				पात = 1;
			}
		}
		udelay(10000);
	} जबसम्म (!पात && get_ticks() <= etime);

	फर्काऊ पात;
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
	यदि (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		फर्काऊ env_get_yesno("bootstopusesha256") == 1;
	अन्यथा यदि (IS_ENABLED(CONFIG_CRYPT_PW))
		फर्काऊ false;
	अन्यथा
		फर्काऊ true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
स्थिर पूर्णाङ्क abortboot_साँचो_sequence(पूर्णाङ्क bootdelay)
{
	पूर्णाङ्क पात;
	uint64_t etime = endtick(bootdelay);

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	म_लिखो(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		यदि (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			पात = passwd_abort_crypt(etime);
		अन्यथा
			पात = passwd_abort_sha256(etime);
	} अन्यथा {
		पात = passwd_abort_साँचो(etime);
	}
	यदि (!पात)
		debug_bootkeys("key timeout\n");

	फर्काऊ पात;
}

स्थिर शून्य print_boot_delay(पूर्णाङ्क bootdelay)
{
	म_लिखो(ANSI_CLEAR_पङ्क्ति "\rHit any key to stop autoboot: %d", bootdelay);
}

स्थिर पूर्णाङ्क abortboot_single_साँचो(पूर्णाङ्क bootdelay)
{
	पूर्णाङ्क पात = 0;
	अचिन्हित दीर्घ ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	यदि (tstc()) {	/* we got a key press	*/
		अक्षर_लो();	/* consume input	*/
		print_boot_delay(0);
		पात = 1;	/* don't auto boot	*/
	}

	जबसम्म ((bootdelay > 0) && (!पात)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		गर {
			यदि (tstc()) {	/* we got a key press	*/
				पूर्णाङ्क साँचो;

				पात  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				साँचो = अक्षर_लो();/* consume input	*/
				यदि (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = साँचो;
				रोक;
			}
			udelay(10000);
		} जबसम्म (!पात && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	अ_दो('\n');

	फर्काऊ पात;
}

स्थिर पूर्णाङ्क abortboot(पूर्णाङ्क bootdelay)
{
	पूर्णाङ्क पात = 0;

	यदि (bootdelay >= 0) {
		यदि (autoboot_keyed())
			पात = abortboot_साँचो_sequence(bootdelay);
		अन्यथा
			पात = abortboot_single_साँचो(bootdelay);
	}

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && पात)
		gd->flags &= ~GD_FLG_SILENT;

	फर्काऊ पात;
}

स्थिर शून्य process_fdt_options(शून्य)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	यदि (addr)
		env_set_addr("kernaddr", (शून्य *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	यदि (addr)
		env_set_addr("rootaddr", (शून्य *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

अचर वर्ण *bootdelay_process(शून्य)
{
	वर्ण *s;
	पूर्णाङ्क bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (पूर्णाङ्क)simple_strtol(s, रिक्त, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	यदि (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	यदि (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} अन्यथा
#endif /* CONFIG_POST */
	यदि (bootcount_error())
		s = env_get("altbootcmd");
	अन्यथा
		s = env_get("bootcmd");

	यदि (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	फर्काऊ s;
}

शून्य autoboot_command(अचर वर्ण *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	यदि (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		पूर्णाङ्क prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		यदि (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		यदि (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	यदि (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		यदि (s)
			run_command_list(s, -1, 0);
	}
}
