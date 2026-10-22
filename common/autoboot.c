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
static целое stored_bootdelay;
static целое menukey;

#если defined(CONFIG_AUTOBOOT_STOP_STR_CRYPT)
#define AUTOBOOT_STOP_STR_CRYPT	CONFIG_AUTOBOOT_STOP_STR_CRYPT
#иначе
#define AUTOBOOT_STOP_STR_CRYPT	""
#endif
#если defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)
#define AUTOBOOT_STOP_STR_SHA256	CONFIG_AUTOBOOT_STOP_STR_SHA256
#иначе
#define AUTOBOOT_STOP_STR_SHA256	""
#endif

#ifdef CONFIG_AUTOBOOT_USE_MENUKEY
#define AUTOBOOT_MENUKEY CONFIG_AUTOBOOT_MENUKEY
#иначе
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
static целое passwd_abort_crypt(uint64_t etime)
{
	const char *crypt_env_str = env_get("bootstopkeycrypt");
	char presskey[DELAY_STOP_STR_MAX_LENGTH];
	u_int presskey_len = 0;
	целое abort = 0;
	целое never_timeout = 0;
	целое err;

	если (IS_ENABLED(CONFIG_AUTOBOOT_STOP_STR_ENABLE) && !crypt_env_str)
		crypt_env_str = AUTOBOOT_STOP_STR_CRYPT;

	если (!crypt_env_str)
		вернуть 0;

	/* We expect the stop-string to be newline-terminated */
	do {
		если (tstc()) {
			/* Check for input string overflow */
			если (presskey_len >= sizeof(presskey))
				вернуть 0;

			presskey[presskey_len] = getchar();

			если ((presskey[presskey_len] == '\r') ||
			    (presskey[presskey_len] == '\n')) {
				если (IS_ENABLED(CONFIG_AUTOBOOT_NEVER_TIMEOUT) &&
				    !presskey_len) {
					never_timeout = 1;
					continue;
				}
				presskey[presskey_len] = '\0';
				err = crypt_compare(crypt_env_str, presskey,
						    &abort);
				если (err)
					debug_bootkeys(
						"crypt_compare() failed with: %s\n",
						errno_str(err));
				/* you had one chance */
				break;
			} иначе {
				presskey_len++;
			}
		}
		udelay(10000);
	} пока (never_timeout || get_ticks() <= etime);

	вернуть abort;
}

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
static целое slow_equals(u8 *a, u8 *b, целое len)
{
	целое diff = 0;
	целое i;

	для (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	вернуть diff == 0;
}

/**
 * passwd_abort_sha256() - check for a hashed key sequence to abort booting
 *
 * This checks for the user entering a SHA256 hash within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
static целое passwd_abort_sha256(uint64_t etime)
{
	const char *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 *sha;
	char *presskey;
	char *c;
	const char *algo_name = "sha256";
	u_int presskey_len = 0;
	целое abort = 0;
	целое size = sizeof(sha);
	целое ret;

	если (sha_env_str == NULL)
		sha_env_str = AUTOBOOT_STOP_STR_SHA256;

	presskey = malloc_cache_aligned(DELAY_STOP_STR_MAX_LENGTH);
	если (!presskey)
		вернуть -ENOMEM;

	c = strstr(sha_env_str, ":");
	если (c && (c - sha_env_str < DELAY_STOP_STR_MAX_LENGTH)) {
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
	если (ret) {
		printf("Hash %s not supported!\n", algo_name);
		free(presskey);
		вернуть 0;
	}

	sha = malloc_cache_aligned(SHA256_SUM_LEN);
	если (!sha) {
		free(presskey);
		вернуть -ENOMEM;
	}
	size = SHA256_SUM_LEN;
	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	do {
		если (tstc()) {
			/* Check for input string overflow */
			если (presskey_len >= DELAY_STOP_STR_MAX_LENGTH) {
				free(presskey);
				free(sha);
				вернуть 0;
			}

			presskey[presskey_len++] = getchar();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (const void *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			если (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
		udelay(10000);
	} пока (!abort && get_ticks() <= etime);

	free(presskey);
	free(sha);
	вернуть abort;
}

/**
 * passwd_abort_key() - check for a key sequence to aborted booting
 *
 * This checks for the user entering a string within a given time.
 *
 * @etime: Timeout value ticks (stop when get_ticks() reachs this)
 * Return: 0 if autoboot should continue, 1 if it should stop
 */
static целое passwd_abort_key(uint64_t etime)
{
	целое abort = 0;
	struct {
		char *str;
		u_int len;
		целое retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	char presskey[DELAY_STOP_STR_MAX_LENGTH];
	целое presskey_len = 0;
	целое presskey_max = 0;
	целое i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	если (delaykey[0].str == NULL)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	если (delaykey[1].str == NULL)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	для (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i++) {
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
		если (tstc()) {
			если (presskey_len < presskey_max) {
				presskey[presskey_len++] = getchar();
			} иначе {
				для (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getchar();
			}
		}

		для (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i++) {
			если (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				если (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		udelay(10000);
	} пока (!abort && get_ticks() <= etime);

	вернуть abort;
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
	если (IS_ENABLED(CONFIG_AUTOBOOT_SHA256_FALLBACK))
		вернуть env_get_yesno("bootstopusesha256") == 1;
	иначе если (IS_ENABLED(CONFIG_CRYPT_PW))
		вернуть false;
	иначе
		вернуть true;
}

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
static целое abortboot_key_sequence(целое bootdelay)
{
	целое abort;
	uint64_t etime = endtick(bootdelay);

	если (IS_ENABLED(CONFIG_AUTOBOOT_FLUSH_STDIN))
		console_flush_stdin();
#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	если (IS_ENABLED(CONFIG_AUTOBOOT_ENCRYPTION)) {
		если (IS_ENABLED(CONFIG_CRYPT_PW) && !fallback_to_sha256())
			abort = passwd_abort_crypt(etime);
		иначе
			abort = passwd_abort_sha256(etime);
	} иначе {
		abort = passwd_abort_key(etime);
	}
	если (!abort)
		debug_bootkeys("key timeout\n");

	вернуть abort;
}

static void print_boot_delay(целое bootdelay)
{
	printf(ANSI_CLEAR_LINE "\rHit any key to stop autoboot: %d", bootdelay);
}

static целое abortboot_single_key(целое bootdelay)
{
	целое abort = 0;
	unsigned long ts;

	print_boot_delay(bootdelay);

	/*
	 * Check if key already pressed
	 */
	если (tstc()) {	/* we got a key press	*/
		getchar();	/* consume input	*/
		print_boot_delay(0);
		abort = 1;	/* don't auto boot	*/
	}

	пока ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		do {
			если (tstc()) {	/* we got a key press	*/
				целое key;

				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
				key = getchar();/* consume input	*/
				если (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY))
					menukey = key;
				break;
			}
			udelay(10000);
		} пока (!abort && get_timer(ts) < 1000);

		print_boot_delay(bootdelay);
	}

	putc('\n');

	вернуть abort;
}

static целое abortboot(целое bootdelay)
{
	целое abort = 0;

	если (bootdelay >= 0) {
		если (autoboot_keyed())
			abort = abortboot_key_sequence(bootdelay);
		иначе
			abort = abortboot_single_key(bootdelay);
	}

	если (IS_ENABLED(CONFIG_SILENT_CONSOLE) && abort)
		gd->flags &= ~GD_FLG_SILENT;

	вернуть abort;
}

static void process_fdt_options(void)
{
#ifdef CONFIG_TEXT_BASE
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = ofnode_conf_read_int("kernel-offset", 0);
	если (addr)
		env_set_addr("kernaddr", (void *)(CONFIG_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = ofnode_conf_read_int("rootdisk-offset", 0);
	если (addr)
		env_set_addr("rootaddr", (void *)(CONFIG_TEXT_BASE + addr));
#endif /* CONFIG_TEXT_BASE */
}

const char *bootdelay_process(void)
{
	char *s;
	целое bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (целое)simple_strtol(s, NULL, 10) : CONFIG_BOOTDELAY;

	/*
	 * Does it really make sense that the devicetree overrides the user
	 * setting? It is possibly helpful for security since the device tree
	 * may be signed whereas the environment is often loaded from storage.
	 */
	если (IS_ENABLED(CONFIG_OF_CONTROL))
		bootdelay = ofnode_conf_read_int("bootdelay", bootdelay);

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

	если (IS_ENABLED(CONFIG_AUTOBOOT_MENU_SHOW))
		bootdelay = menu_show(bootdelay);
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	если (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} иначе
#endif /* CONFIG_POST */
	если (bootcount_error())
		s = env_get("altbootcmd");
	иначе
		s = env_get("bootcmd");

	если (IS_ENABLED(CONFIG_OF_CONTROL))
		process_fdt_options();
	stored_bootdelay = bootdelay;

	вернуть s;
}

void autoboot_command(const char *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	если (s && (stored_bootdelay == -2 ||
		 (stored_bootdelay != -1 && !abortboot(stored_bootdelay)))) {
		bool lock;
		целое prev;

		lock = autoboot_keyed() &&
			!IS_ENABLED(CONFIG_AUTOBOOT_KEYED_CTRLC);
		если (lock)
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(s, -1, 0);

		если (lock)
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}

	если (IS_ENABLED(CONFIG_AUTOBOOT_USE_MENUKEY) &&
	    menukey == AUTOBOOT_MENUKEY) {
		s = env_get("menucmd");
		если (s)
			run_command_list(s, -1, 0);
	}
}
