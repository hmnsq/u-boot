// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef USE_HOSTCC
#include <command.h>
#include <dm.h>
#include <env.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <hw_sha.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <u-boot/hash.h>
#иначе
#include "mkimage.h"
#include <linux/compiler_attributes.h>
#include <time.h>
#include <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#include <hash.h>
#include <image.h>
#include <u-boot/crc.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>
#include <u-boot/md5.h>
#include <u-boot/sm3.h>

static цяло __maybe_unused hash_init_sha1(struct hash_algo *algo, void **ctxp)
{
	sha1_context *ctx = malloc(sizeof(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	върни 0;
}

static цяло __maybe_unused hash_update_sha1(struct hash_algo *algo, void *ctx,
					   const void *buf, unsigned цяло size,
					   цяло is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	върни 0;
}

static цяло __maybe_unused hash_finish_sha1(struct hash_algo *algo, void *ctx,
					   void *dest_buf, цяло size)
{
	ако (size < algo->digest_size)
		върни -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	free(ctx);
	върни 0;
}

static цяло __maybe_unused hash_init_sha256(struct hash_algo *algo, void **ctxp)
{
	sha256_context *ctx = malloc(sizeof(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	върни 0;
}

static цяло __maybe_unused hash_update_sha256(struct hash_algo *algo, void *ctx,
					     const void *buf, uint size,
					     цяло is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	върни 0;
}

static цяло __maybe_unused hash_finish_sha256(struct hash_algo *algo, void *ctx,
					     void *dest_buf, цяло size)
{
	ако (size < algo->digest_size)
		върни -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	free(ctx);
	върни 0;
}

static цяло __maybe_unused hash_init_sha384(struct hash_algo *algo, void **ctxp)
{
	sha512_context *ctx = malloc(sizeof(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	върни 0;
}

static цяло __maybe_unused hash_update_sha384(struct hash_algo *algo, void *ctx,
					     const void *buf, uint size,
					     цяло is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	върни 0;
}

static цяло __maybe_unused hash_finish_sha384(struct hash_algo *algo, void *ctx,
					     void *dest_buf, цяло size)
{
	ако (size < algo->digest_size)
		върни -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	free(ctx);
	върни 0;
}

static цяло __maybe_unused hash_init_sha512(struct hash_algo *algo, void **ctxp)
{
	sha512_context *ctx = malloc(sizeof(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	върни 0;
}

static цяло __maybe_unused hash_update_sha512(struct hash_algo *algo, void *ctx,
					     const void *buf, uint size,
					     цяло is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	върни 0;
}

static цяло __maybe_unused hash_finish_sha512(struct hash_algo *algo, void *ctx,
					     void *dest_buf, цяло size)
{
	ако (size < algo->digest_size)
		върни -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	free(ctx);
	върни 0;
}

static цяло __maybe_unused hash_init_sm3(struct hash_algo *algo, void **ctxp)
{
	struct sm3_context *ctx = malloc(sizeof(struct sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	върни 0;
}

static цяло __maybe_unused hash_update_sm3(struct hash_algo *algo, void *ctx,
					  const void *buf, uint size,
					  цяло is_last)
{
	sm3_update((struct sm3_context *)ctx, buf, size);
	върни 0;
}

static цяло __maybe_unused hash_finish_sm3(struct hash_algo *algo, void *ctx,
					  void *dest_buf, цяло size)
{
	ако (size < algo->digest_size)
		върни -1;

	sm3_final((struct sm3_context *)ctx, dest_buf);
	free(ctx);
	върни 0;
}

static цяло __maybe_unused hash_init_crc16_ccitt(struct hash_algo *algo,
						void **ctxp)
{
	uint16_t *ctx = malloc(sizeof(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	върни 0;
}

static цяло __maybe_unused hash_update_crc16_ccitt(struct hash_algo *algo,
						  void *ctx, const void *buf,
						  unsigned цяло size,
						  цяло is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	върни 0;
}

static цяло __maybe_unused hash_finish_crc16_ccitt(struct hash_algo *algo,
						  void *ctx, void *dest_buf,
						  цяло size)
{
	ако (size < algo->digest_size)
		върни -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	free(ctx);
	върни 0;
}

static цяло __maybe_unused hash_init_crc32(struct hash_algo *algo, void **ctxp)
{
	uint32_t *ctx = malloc(sizeof(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	върни 0;
}

static цяло __maybe_unused hash_update_crc32(struct hash_algo *algo, void *ctx,
					    const void *buf, unsigned цяло size,
					    цяло is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	върни 0;
}

static цяло __maybe_unused hash_finish_crc32(struct hash_algo *algo, void *ctx,
					    void *dest_buf, цяло size)
{
	ако (size < algo->digest_size)
		върни -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	free(ctx);
	върни 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
static struct hash_algo hash_algo[] = {
#ако CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#ако CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#ако CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#иначе
		.hash_func_ws	= sha1_csum_wd,
#endif
#ако CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#иначе
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#ако CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#ако CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#иначе
		.hash_func_ws	= sha256_csum_wd,
#endif
#ако CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#иначе
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#ако CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#ако CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#иначе
		.hash_func_ws	= sha384_csum_wd,
#endif
#ако CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#иначе
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#ако CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#ако CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#иначе
		.hash_func_ws	= sha512_csum_wd,
#endif
#ако CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#иначе
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#ако CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_size	= SM3_DIGEST_SIZE,
		.chunk_size	= SM3_BLOCK_SIZE,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_finish	= hash_finish_sm3,
	},
#endif
#ако CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_size	= 2,
		.chunk_size	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_finish	= hash_finish_crc16_ccitt,
	},
#endif
#ако CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#ако CONFIG_IS_ENABLED(CRC32)
	{
		.name		= "crc32",
		.digest_size	= 4,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc32_wd_buf,
		.hash_init	= hash_init_crc32,
		.hash_update	= hash_update_crc32,
		.hash_finish	= hash_finish_crc32,
	},
#endif
};

/* Try to minimize code size for boards that don't want much hashing */
#ако CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#иначе
#define multi_hash()	0
#endif

цяло hash_lookup_algo(const char *algo_name, struct hash_algo **algop)
{
	цяло i;

	за (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		ако (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			върни 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	върни -EPROTONOSUPPORT;
}

цяло hash_progressive_lookup_algo(const char *algo_name,
				 struct hash_algo **algop)
{
	цяло i;

	за (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		ако (!strcmp(algo_name, hash_algo[i].name)) {
			ако (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				върни 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	върни -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
цяло hash_parse_string(const char *algo_name, const char *str, uint8_t *result)
{
	struct hash_algo *algo;
	цяло ret;
	цяло i;

	ret = hash_lookup_algo(algo_name, &algo);
	ако (ret)
		върни ret;

	за (i = 0; i < algo->digest_size; i++) {
		char chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, NULL);
	}

	върни 0;
}

цяло hash_block(const char *algo_name, const void *data, unsigned цяло len,
	       uint8_t *output, цяло *output_size)
{
	struct hash_algo *algo;
	цяло ret;

	ret = hash_lookup_algo(algo_name, &algo);
	ако (ret)
		върни ret;

	ако (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		върни -ENOSPC;
	}
	ако (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	върни 0;
}

#ако !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1SUM) || defined(CONFIG_CMD_CRC32)) || \
	defined(CONFIG_CMD_MD5SUM)
/**
 * store_result: Store the resulting sum to an address or variable
 *
 * @algo:		Hash algorithm being used
 * @sum:		Hash digest (algo->digest_size bytes)
 * @dest:		Destination, interpreted as a hex address if it starts
 *			with * (or allow_env_vars is 0) or otherwise as an
 *			environment variable.
 * @allow_env_vars:	non-zero to permit storing the result to an
 *			variable environment
 */
static void store_result(struct hash_algo *algo, const uint8_t *sum,
			 const char *dest, цяло allow_env_vars)
{
	unsigned цяло i;
	цяло env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	ако (allow_env_vars) {
		ако (*dest == '*')
			dest++;
		иначе
			env_var = 1;
	}

	ако (env_var) {
		char str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		char *str_ptr = str_output;

		за (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} иначе {
		ulong addr;
		void *buf;

		addr = hextoul(dest, NULL);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(buf, sum, algo->digest_size);
		unmap_sysmem(buf);
	}
}

/**
 * parse_verify_sum: Parse a hash verification parameter
 *
 * @algo:		Hash algorithm being used
 * @verify_str:		Argument to parse. If it starts with * then it is
 *			interpreted as a hex address containing the hash.
 *			If the length is exactly the right number of hex digits
 *			for the digest size, then we assume it is a hex digest.
 *			Otherwise we assume it is an environment variable, and
 *			look up its value (it must contain a hex digest).
 * @vsum:		Returns binary digest value (algo->digest_size bytes)
 * @allow_env_vars:	non-zero to permit storing the result to an environment
 *			variable. If 0 then verify_str is assumed to be an
 *			address, and the * prefix is not expected.
 * Return: 0 if ok, non-zero on error
 */
static цяло parse_verify_sum(struct hash_algo *algo, char *verify_str,
			    uint8_t *vsum, цяло allow_env_vars)
{
	цяло env_var = 0;

	/* See comment above in store_result() */
	ако (allow_env_vars) {
		ако (*verify_str == '*')
			verify_str++;
		иначе
			env_var = 1;
	}

	ако (!env_var) {
		ulong addr;
		void *buf;

		addr = hextoul(verify_str, NULL);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} иначе {
		char *vsum_str;
		цяло digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		ако (strlen(verify_str) == digits)
			vsum_str = verify_str;
		иначе {
			vsum_str = env_get(verify_str);
			ако (vsum_str == NULL || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				върни 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	върни 0;
}

static void hash_show(struct hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	цяло i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	за (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

цяло hash_command(const char *algo_name, цяло flags, struct cmd_tbl *cmdtp,
		 цяло flag, цяло argc, char *const argv[])
{
	ulong addr, len;

	ако ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		върни CMD_RET_USAGE;

	addr = hextoul(*argv++, NULL);
	len = hextoul(*argv++, NULL);

	ако (multi_hash()) {
		struct hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		void *buf;

		ако (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			върни CMD_RET_USAGE;
		}
		argc -= 2;

		ако (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			върни 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  sizeof(uint32_t) * HASH_MAX_DIGEST_SIZE);
		ако (!output)
			върни CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		ако (CONFIG_IS_ENABLED(DM_HASH)) {
			enum HASH_ALGO hash_algo;
			цяло ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			ако (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				ако (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					free(output);
					върни CMD_RET_FAILURE;
				}
				ако (!ret)
					goto done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#ако defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		ако (flags & HASH_FLAG_VERIFY) {
#иначе
		ако (0) {
#endif
			ако (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				free(output);
				върни 1;
			}
			ако (memcmp(output, vsum, algo->digest_size) != 0) {
				цяло i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				за (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				free(output);
				върни 1;
			}
		} иначе {
			hash_show(algo, addr, len, output);
			printf("\n");

			ако (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		free(output);

	/* Horrible code size hack for boards that just want crc32 */
	} иначе {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (const uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		ако (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], NULL);
			*ptr = crc;
		}
	}

	върни 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
