// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <config.h>
#include <ansi.h>
#include <autoboot.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <errno.h>
#include <fdtdec.h>
#include <hash.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <menu.h>
#include <post.h>
#include <time.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <u-boot/sha256.h>
#include <bootcount.h>
#include <crypt.h>
#include <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;

#define DELAY_STOP_STR_MAX_LENGTH 64

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
static පූර්ණ_සංඛ්‍යා stored_bootdelay;
static පූර්ණ_සංඛ්‍යා menukey;

#නම් defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#නැතහොත්
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#නම් defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#නැතහොත්
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#නැතහොත්
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
static පූර්ණ_සංඛ්‍යා passwd_abort_crypt(uint64_t etime)
{
	const var_na *crypt_env_str = env_get("bootstopkeycrypt");
	var_na presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	පූර්ණ_සංඛ්‍යා abort = 0;
	පූර්ණ_සංඛ්‍යා never_timeout = 0;
	පූර්ණ_සංඛ්‍යා err;

	නම් (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	නම් (!crypt_env_str)
		ආපසු 0;

	/* We expect the stop-string to be newline-terminated */
	do {
		නම් (tstc()) {
			/* Check for input string overflow */
			නම් (presskey_len >= parimaa_na(presskey))
				ආපසු 0;

			presskey[presskey_len] = getchar();

			නම් ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				නම් (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					continue;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				නම් (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				bha_nga;
			} නැතහොත් {
				presskey_len++;
			}
		}
		udelay(10000);
	} අතරතුර (never_timeout || get_ticks() <= etime);

	ආපසු abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
static පූර්ණ_සංඛ්‍යා slow_equals(u8 *a, u8 *b, පූර්ණ_සංඛ්‍යා len)
{
	පූර්ණ_සංඛ්‍යා diff = 0;
	පූර්ණ_සංඛ්‍යා i;

	සඳහා (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	ආපසු diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
static පූර්ණ_සංඛ්‍යා passwd_abort_sha256(uint64_t etime)
{
	const var_na *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	var_na *presskey;
	var_na *c;
	const var_na *algo_name = "sha256";
	u_int presskey_len = 0;
	පූර්ණ_සංඛ්‍යා abort = 0;
	පූර්ණ_සංඛ්‍යා size = parimaa_na(sha);
	පූර්ණ_සංඛ්‍යා ret;

	නම් (sha_env_str == NULL)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	නම් (!presskey)
		ආපසු -ENOMEM;

	c = strstr(sha_env_str, ":");
	නම් (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	නම් (ret) {
		printf("Hash %s not supported!\n", algo_name);
		free(presskey);
		ආපසු 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	නම් (!sha) {
		free(presskey);
		ආපසු -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	do {
		නම් (tstc()) {
			/* Check for input string overflow */
			නම් (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				free(presskey);
				free(sha);
				ආපසු 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (const void *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			නම් (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} අතරතුර (!abort && get_ticks() <= etime);

	free(presskey);
	free(sha);
	ආපසු abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
static පූර්ණ_සංඛ්‍යා passwd_abort_key(uint64_t etime)
{
	පූර්ණ_සංඛ්‍යා abort = 0;
	sa_nracanaa {
		var_na *str;
		u_int len;
		පූර්ණ_සංඛ්‍යා retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	var_na presskey[DELAY_STOP_STR_MAX_LENGTH];
	පූර්ණ_සංඛ්‍යා presskey_len = 0;
	පූර්ණ_සංඛ්‍යා presskey_max = 0;
	පූර්ණ_සංඛ්‍යා i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	නම් (delaykey[0].str == NULL)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	නම් (delaykey[1].str == NULL)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	සඳහා (i = 0; i < parimaa_na(delaykey) / parimaa_na(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == NULL ?
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
	do {
		නම් (tstc()) {
			නම් (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} නැතහොත් {
				සඳහා (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		සඳහා (i = 0; i < parimaa_na(delaykey) / parimaa_na(delaykey[0]); i++) {
			නම් (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				නම් (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} අතරතුර (!abort && get_ticks() <= etime);

	ආපසු abort;
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
static bool fallback_to_sha256(void)
{
	නම් (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		ආපසු env_get_yesno("bootstopusesha256") == 1;
	නැතහොත් නම් (IS_ENABLED(CONFIG_CRYPT_PW))
		ආපසු false;
	නැතහොත්
		ආපසු true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
static පූර්ණ_සංඛ්‍යා abortboot_key_sequence(පූර්ණ_සංඛ්‍යා bootdelay)
{
	පූර්ණ_සංඛ්‍යා abort;
	uint64_t etime = endtick(bootdelay);

	නම් (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	නම් (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		නම් (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		නැතහොත්
			abort = passwd_abort_sha256(etime);
	} නැතහොත් {
		abort = passwd_abort_key(etime);
	}
	නම් (!abort)
		debug_bootkeys("key timeout\n");

	ආපසු abort;
}

static void print_boot_delay(පූර්ණ_සංඛ්‍යා bootdelay)
{
	printf(ANSI_CLEAR_LINE "\rHit any key to stop autoboot: %d", bootdelay);
}

static පූර්ණ_සංඛ්‍යා abortboot_single_key(පූර්ණ_සංඛ්‍යා bootdelay)
{
	පූර්ණ_සංඛ්‍යා abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	නම් (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	අතරතුර ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		do {
			නම් (tstc()) {	/* we got a key press	*/
				පූර්ණ_සංඛ්‍යා key;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				key = getchar();/* consume input	*/
				නම් (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = key;
				bha_nga;
			}
			udelay(10000);
		} අතරතුර (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	ආපසු abort;
}

static පූර්ණ_සංඛ්‍යා abortboot(පූර්ණ_සංඛ්‍යා bootdelay)
{
	පූර්ණ_සංඛ්‍යා abort = 0;

	නම් (bootdelay >= 0) {
		නම් (autoboot_keyed())
			abort = abortboot_key_sequence(bootdelay);
		නැතහොත්
			abort = abortboot_single_key(bootdelay);
	}

	නම් (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	ආපසු abort;
}

static void process_fdt_options(void)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	නම් (addr)
		env_set_addr("kernaddr", (void *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	නම් (addr)
		env_set_addr("rootaddr", (void *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

const var_na *bootdelay_process(void)
{
	var_na *s;
	පූර්ණ_සංඛ්‍යා bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (පූර්ණ_සංඛ්‍යා)simple_strtol(s, NULL, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	නම් (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	නම් (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	නම් (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} නැතහොත්
#endif /* CONFIG_POST */
	නම් (bootcount_error())
		s = env_get("altbootcmd");
	නැතහොත්
		s = env_get("bootcmd");

	නම් (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	ආපසු s;
}

void autoboot_command(const var_na *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	නම් (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		පූර්ණ_සංඛ්‍යා prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		නම් (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		නම් (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	නම් (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		නම් (s)
			run_command_list(s, -1, 0);
	}
}
