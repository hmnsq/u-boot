// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#समावेश <config.h>
#समावेश <ansi.h>
#समावेश <autoboot.h>
#समावेश <bootretry.h>
#समावेश <cli.h>
#समावेश <command.h>
#समावेश <console.h>
#समावेश <env.h>
#समावेश <errno.h>
#समावेश <fdtdec.h>
#समावेश <hash.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <memalign.h>
#समावेश <menu.h>
#समावेश <post.h>
#समावेश <time.h>
#समावेश <asm/global_data.h>
#समावेश <linux/delay.h>
#समावेश <u-boot/sha256.h>
#समावेश <bootcount.h>
#समावेश <crypt.h>
#समावेश <dm/ofnode.h>

DECLARE_GLOBAL_तथ्य_PTR;

#घोषणा DELAY_STOP_STR_MAX_लंबाई 64

#अगर_अघोषित DEBUG_BOOTKEYS
#घोषणा DEBUG_BOOTKEYS 0
#पूर्ण_अगर
#घोषणा debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
अटल पूर्णांक stored_bootdelay;
अटल पूर्णांक menukey;

#अगर defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#घोषणा AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#अन्यथा
#घोषणा AUTOBOOT_STOP_STR_CRYPT	""
#पूर्ण_अगर
#अगर defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#घोषणा AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#अन्यथा
#घोषणा AUTOBOOT_STOP_STR_SHA256	""
#पूर्ण_अगर

#अगर_घोषित CONFIG_AUTOBOOT_USE_MENUKEY
#घोषणा AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#अन्यथा
#घोषणा AUTOBOOT_MENUKEY 0
#पूर्ण_अगर

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
अटल पूर्णांक passwd_abort_crypt(uint64_t etime)
{
	स्थिर अक्षर *crypt_env_str = env_get("bootstopkeycrypt");
	अक्षर presskey[DELAY_STOP_STR_MAX_लंबाई];
	u_int presskey_len = 0;
	पूर्णांक पात = 0;
	पूर्णांक never_timeout = 0;
	पूर्णांक err;

	अगर (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	अगर (!crypt_env_str)
		वापस 0;

	/* We expect the stop-string to be newline-terminated */
	करो {
		अगर (tstc()) {
			/* Check for input string overflow */
			अगर (presskey_len >= माप(presskey))
				वापस 0;

			presskey[presskey_len] = अक्षर_लो();

			अगर ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				अगर (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					जारी;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &पात);
				अगर (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				अवरोध;
			} अन्यथा {
				presskey_len++;
			}
		}
		udelay(10000);
	} जबतक (never_timeout || get_ticks() <= etime);

	वापस पात;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
अटल पूर्णांक slow_equals(u8 *a, u8 *b, पूर्णांक len)
{
	पूर्णांक diff = 0;
	पूर्णांक i;

	क्रम (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	वापस diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
अटल पूर्णांक passwd_abort_sha256(uint64_t etime)
{
	स्थिर अक्षर *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_योग_LEN];
	u8 *sha;
	अक्षर *presskey;
	अक्षर *c;
	स्थिर अक्षर *algo_name = "sha256";
	u_int presskey_len = 0;
	पूर्णांक पात = 0;
	पूर्णांक आकार = माप(sha);
	पूर्णांक ret;

	अगर (sha_env_str == शून्य)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_लंबाई);
	अगर (!presskey)
		वापस -ENOMEM;

	c = म_माला(sha_env_str, ":");
	अगर (c && (c - sha_env_str < DELAY_STOP_STR_MAX_लंबाई)) {
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
	अगर (ret) {
		म_लिखो("Hash %s not supported!\n", algo_name);
		मुक्त(presskey);
		वापस 0;
	}

	sha = malloc_cache_aligned(SHA256_योग_LEN);
	अगर (!sha) {
		मुक्त(presskey);
		वापस -ENOMEM;
	}
	आकार = SHA256_योग_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	करो {
		अगर (tstc()) {
			/* Check for input string overflow */
			अगर (presskey_len >= DELAY_STOP_STR_MAX_लंबाई) {
				मुक्त(presskey);
				मुक्त(sha);
				वापस 0;
			}

			presskey[presskey_len++] = अक्षर_लो();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (स्थिर व्योम *)presskey,
				   presskey_len, sha, &आकार);

			/* And check if sha matches saved value in env */
			अगर (slow_equals(sha, sha_env, SHA256_योग_LEN))
				पात = 1;
		}
		udelay(10000);
	} जबतक (!पात && get_ticks() <= etime);

	मुक्त(presskey);
	मुक्त(sha);
	वापस पात;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
अटल पूर्णांक passwd_abort_कुंजी(uint64_t etime)
{
	पूर्णांक पात = 0;
	काष्ठा {
		अक्षर *str;
		u_int len;
		पूर्णांक retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	अक्षर presskey[DELAY_STOP_STR_MAX_लंबाई];
	पूर्णांक presskey_len = 0;
	पूर्णांक presskey_max = 0;
	पूर्णांक i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	अगर (delaykey[0].str == शून्य)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	अगर (delaykey[1].str == शून्य)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	क्रम (i = 0; i < माप(delaykey) / माप(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == शून्य ?
				    0 : म_माप(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > DELAY_STOP_STR_MAX_लंबाई ?
				    DELAY_STOP_STR_MAX_लंबाई : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	करो {
		अगर (tstc()) {
			अगर (presskey_len < presskey_max) {
				presskey[presskey_len++] = अक्षर_लो();
			} अन्यथा {
				क्रम (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = अक्षर_लो();
			}
		}

		क्रम (i = 0; i < माप(delaykey) / माप(delaykey[0]); i++) {
			अगर (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				स_भेद(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				अगर (!delaykey[i].retry)
					bootretry_dont_retry();
				पात = 1;
			}
		}
		udelay(10000);
	} जबतक (!पात && get_ticks() <= etime);

	वापस पात;
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
अटल bool fallback_to_sha256(व्योम)
{
	अगर (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		वापस env_get_yesno("bootstopusesha256") == 1;
	अन्यथा अगर (IS_ENABLED(CONFIG_CRYPT_PW))
		वापस false;
	अन्यथा
		वापस true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
अटल पूर्णांक abortboot_कुंजी_sequence(पूर्णांक bootdelay)
{
	पूर्णांक पात;
	uint64_t etime = endtick(bootdelay);

	अगर (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	म_लिखो(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	अगर (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		अगर (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			पात = passwd_abort_crypt(etime);
		अन्यथा
			पात = passwd_abort_sha256(etime);
	} अन्यथा {
		पात = passwd_abort_कुंजी(etime);
	}
	अगर (!पात)
		debug_bootkeys("key timeout\n");

	वापस पात;
}

अटल व्योम print_boot_delay(पूर्णांक bootdelay)
{
	म_लिखो(ANSI_CLEAR_पंक्ति "\rHit any key to stop autoboot: %d", bootdelay);
}

अटल पूर्णांक abortboot_single_कुंजी(पूर्णांक bootdelay)
{
	पूर्णांक पात = 0;
	अचिन्हित दीर्घ ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	अगर (tstc()) {	/* we got a key press	*/
		अक्षर_लो();	/* consume input	*/
		print_boot_delay(0);
		पात = 1;	/* don't auto boot	*/
	}

	जबतक ((bootdelay > 0) && (!पात)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		करो {
			अगर (tstc()) {	/* we got a key press	*/
				पूर्णांक कुंजी;

				पात  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				कुंजी = अक्षर_लो();/* consume input	*/
				अगर (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = कुंजी;
				अवरोध;
			}
			udelay(10000);
		} जबतक (!पात && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	अ_दो('\n');

	वापस पात;
}

अटल पूर्णांक abortboot(पूर्णांक bootdelay)
{
	पूर्णांक पात = 0;

	अगर (bootdelay >= 0) {
		अगर (autoboot_keyed())
			पात = abortboot_कुंजी_sequence(bootdelay);
		अन्यथा
			पात = abortboot_single_कुंजी(bootdelay);
	}

	अगर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && पात)
		gd->flags &= ~GD_FLG_SILENT;

	वापस पात;
}

अटल व्योम process_fdt_options(व्योम)
{
#अगर_घोषित CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	अगर (addr)
		env_set_addr("kernaddr", (व्योम *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	अगर (addr)
		env_set_addr("rootaddr", (व्योम *)(CONFIG_TEXT_BASE + addr));
#पूर्ण_अगर /* CONFIG_TEXT_BASE */
}

स्थिर अक्षर *bootdelay_process(व्योम)
{
	अक्षर *s;
	पूर्णांक bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (पूर्णांक)simple_strtol(s, शून्य, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	अगर (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	अगर (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#अगर_घोषित CONFIG_POST
	अगर (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} अन्यथा
#पूर्ण_अगर /* CONFIG_POST */
	अगर (bootcount_error())
		s = env_get("altbootcmd");
	अन्यथा
		s = env_get("bootcmd");

	अगर (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	वापस s;
}

व्योम autoboot_command(स्थिर अक्षर *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	अगर (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		पूर्णांक पूर्व;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		अगर (lock)
			पूर्व = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		अगर (lock)
			disable_ctrlc(पूर्व);	/* restore Ctrl-C checking */
	}

	अगर (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		अगर (s)
			run_command_list(s, -1, 0);
	}
}
