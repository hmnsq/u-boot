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
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <hw_sha.h>
#અન્તર્ભાવ <asm/cache.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <linux/errno.h>
#અન્તર્ભાવ <u-boot/hash.h>
#નહીંતર
#અન્તર્ભાવ "mkimage.h"
#અન્તર્ભાવ <linux/compiler_attributes.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#અન્તર્ભાવ <hash.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <u-boot/crc.h>
#અન્તર્ભાવ <u-boot/sha1.h>
#અન્તર્ભાવ <u-boot/sha256.h>
#અન્તર્ભાવ <u-boot/sha512.h>
#અન્તર્ભાવ <u-boot/md5.h>
#અન્તર્ભાવ <u-boot/sm3.h>

સ્થિર પૂર્ણાંક __maybe_unused hash_init_sha1(સંરચના hash_algo *algo, શૂન્ય **ctxp)
{
	sha1_context *ctx = malloc(કદ(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_update_sha1(સંરચના hash_algo *algo, શૂન્ય *ctx,
					   અચલ શૂન્ય *buf, unsigned પૂર્ણાંક size,
					   પૂર્ણાંક is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_finish_sha1(સંરચના hash_algo *algo, શૂન્ય *ctx,
					   શૂન્ય *dest_buf, પૂર્ણાંક size)
{
	જો (size < algo->digest_size)
		પાછા -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	મુક્ત(ctx);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_init_sha256(સંરચના hash_algo *algo, શૂન્ય **ctxp)
{
	sha256_context *ctx = malloc(કદ(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_update_sha256(સંરચના hash_algo *algo, શૂન્ય *ctx,
					     અચલ શૂન્ય *buf, uint size,
					     પૂર્ણાંક is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_finish_sha256(સંરચના hash_algo *algo, શૂન્ય *ctx,
					     શૂન્ય *dest_buf, પૂર્ણાંક size)
{
	જો (size < algo->digest_size)
		પાછા -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	મુક્ત(ctx);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_init_sha384(સંરચના hash_algo *algo, શૂન્ય **ctxp)
{
	sha512_context *ctx = malloc(કદ(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_update_sha384(સંરચના hash_algo *algo, શૂન્ય *ctx,
					     અચલ શૂન્ય *buf, uint size,
					     પૂર્ણાંક is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_finish_sha384(સંરચના hash_algo *algo, શૂન્ય *ctx,
					     શૂન્ય *dest_buf, પૂર્ણાંક size)
{
	જો (size < algo->digest_size)
		પાછા -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	મુક્ત(ctx);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_init_sha512(સંરચના hash_algo *algo, શૂન્ય **ctxp)
{
	sha512_context *ctx = malloc(કદ(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_update_sha512(સંરચના hash_algo *algo, શૂન્ય *ctx,
					     અચલ શૂન્ય *buf, uint size,
					     પૂર્ણાંક is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_finish_sha512(સંરચના hash_algo *algo, શૂન્ય *ctx,
					     શૂન્ય *dest_buf, પૂર્ણાંક size)
{
	જો (size < algo->digest_size)
		પાછા -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	મુક્ત(ctx);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_init_sm3(સંરચના hash_algo *algo, શૂન્ય **ctxp)
{
	સંરચના sm3_context *ctx = malloc(કદ(સંરચના sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_update_sm3(સંરચના hash_algo *algo, શૂન્ય *ctx,
					  અચલ શૂન્ય *buf, uint size,
					  પૂર્ણાંક is_last)
{
	sm3_update((સંરચના sm3_context *)ctx, buf, size);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_finish_sm3(સંરચના hash_algo *algo, શૂન્ય *ctx,
					  શૂન્ય *dest_buf, પૂર્ણાંક size)
{
	જો (size < algo->digest_size)
		પાછા -1;

	sm3_final((સંરચના sm3_context *)ctx, dest_buf);
	મુક્ત(ctx);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_init_crc16_ccitt(સંરચના hash_algo *algo,
						શૂન્ય **ctxp)
{
	uint16_t *ctx = malloc(કદ(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_update_crc16_ccitt(સંરચના hash_algo *algo,
						  શૂન્ય *ctx, અચલ શૂન્ય *buf,
						  unsigned પૂર્ણાંક size,
						  પૂર્ણાંક is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_finish_crc16_ccitt(સંરચના hash_algo *algo,
						  શૂન્ય *ctx, શૂન્ય *dest_buf,
						  પૂર્ણાંક size)
{
	જો (size < algo->digest_size)
		પાછા -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	મુક્ત(ctx);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_init_crc32(સંરચના hash_algo *algo, શૂન્ય **ctxp)
{
	uint32_t *ctx = malloc(કદ(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_update_crc32(સંરચના hash_algo *algo, શૂન્ય *ctx,
					    અચલ શૂન્ય *buf, unsigned પૂર્ણાંક size,
					    પૂર્ણાંક is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused hash_finish_crc32(સંરચના hash_algo *algo, શૂન્ય *ctx,
					    શૂન્ય *dest_buf, પૂર્ણાંક size)
{
	જો (size < algo->digest_size)
		પાછા -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	મુક્ત(ctx);
	પાછા 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
સ્થિર સંરચના hash_algo hash_algo[] = {
#જો CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#જો CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#જો CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#નહીંતર
		.hash_func_ws	= sha1_csum_wd,
#endif
#જો CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#નહીંતર
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#જો CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#જો CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#નહીંતર
		.hash_func_ws	= sha256_csum_wd,
#endif
#જો CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#નહીંતર
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#જો CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#જો CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#નહીંતર
		.hash_func_ws	= sha384_csum_wd,
#endif
#જો CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#નહીંતર
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#જો CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#જો CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#નહીંતર
		.hash_func_ws	= sha512_csum_wd,
#endif
#જો CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#નહીંતર
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#જો CONFIG_IS_ENABLED(SM3)
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
#જો CONFIG_IS_ENABLED(CRC16)
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
#જો CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#જો CONFIG_IS_ENABLED(CRC32)
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
#જો CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#નહીંતર
#define multi_hash()	0
#endif

પૂર્ણાંક hash_lookup_algo(અચલ અક્ષર *algo_name, સંરચના hash_algo **algop)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		જો (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			પાછા 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	પાછા -EPROTONOSUPPORT;
}

પૂર્ણાંક hash_progressive_lookup_algo(અચલ અક્ષર *algo_name,
				 સંરચના hash_algo **algop)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		જો (!strcmp(algo_name, hash_algo[i].name)) {
			જો (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				પાછા 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	પાછા -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
પૂર્ણાંક hash_parse_string(અચલ અક્ષર *algo_name, અચલ અક્ષર *str, uint8_t *result)
{
	સંરચના hash_algo *algo;
	પૂર્ણાંક ret;
	પૂર્ણાંક i;

	ret = hash_lookup_algo(algo_name, &algo);
	જો (ret)
		પાછા ret;

	માટે (i = 0; i < algo->digest_size; i++) {
		અક્ષર chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, ખાલી);
	}

	પાછા 0;
}

પૂર્ણાંક hash_block(અચલ અક્ષર *algo_name, અચલ શૂન્ય *data, unsigned પૂર્ણાંક len,
	       uint8_t *output, પૂર્ણાંક *output_size)
{
	સંરચના hash_algo *algo;
	પૂર્ણાંક ret;

	ret = hash_lookup_algo(algo_name, &algo);
	જો (ret)
		પાછા ret;

	જો (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		પાછા -ENOSPC;
	}
	જો (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	પાછા 0;
}

#જો !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
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
સ્થિર શૂન્ય store_result(સંરચના hash_algo *algo, અચલ uint8_t *sum,
			 અચલ અક્ષર *dest, પૂર્ણાંક allow_env_vars)
{
	unsigned પૂર્ણાંક i;
	પૂર્ણાંક env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	જો (allow_env_vars) {
		જો (*dest == '*')
			dest++;
		નહીંતર
			env_var = 1;
	}

	જો (env_var) {
		અક્ષર str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		અક્ષર *str_ptr = str_output;

		માટે (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} નહીંતર {
		ulong addr;
		શૂન્ય *buf;

		addr = hextoul(dest, ખાલી);
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
સ્થિર પૂર્ણાંક parse_verify_sum(સંરચના hash_algo *algo, અક્ષર *verify_str,
			    uint8_t *vsum, પૂર્ણાંક allow_env_vars)
{
	પૂર્ણાંક env_var = 0;

	/* See comment above in store_result() */
	જો (allow_env_vars) {
		જો (*verify_str == '*')
			verify_str++;
		નહીંતર
			env_var = 1;
	}

	જો (!env_var) {
		ulong addr;
		શૂન્ય *buf;

		addr = hextoul(verify_str, ખાલી);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} નહીંતર {
		અક્ષર *vsum_str;
		પૂર્ણાંક digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		જો (strlen(verify_str) == digits)
			vsum_str = verify_str;
		નહીંતર {
			vsum_str = env_get(verify_str);
			જો (vsum_str == ખાલી || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				પાછા 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	પાછા 0;
}

સ્થિર શૂન્ય hash_show(સંરચના hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	પૂર્ણાંક i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	માટે (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

પૂર્ણાંક hash_command(અચલ અક્ષર *algo_name, પૂર્ણાંક flags, સંરચના cmd_tbl *cmdtp,
		 પૂર્ણાંક flag, પૂર્ણાંક argc, અક્ષર *અચલ argv[])
{
	ulong addr, len;

	જો ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		પાછા CMD_RET_USAGE;

	addr = hextoul(*argv++, ખાલી);
	len = hextoul(*argv++, ખાલી);

	જો (multi_hash()) {
		સંરચના hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		શૂન્ય *buf;

		જો (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			પાછા CMD_RET_USAGE;
		}
		argc -= 2;

		જો (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			પાછા 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  કદ(uint32_t) * HASH_MAX_DIGEST_SIZE);
		જો (!output)
			પાછા CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		જો (CONFIG_IS_ENABLED(DM_HASH)) {
			ગણના HASH_ALGO hash_algo;
			પૂર્ણાંક ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			જો (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				જો (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					મુક્ત(output);
					પાછા CMD_RET_FAILURE;
				}
				જો (!ret)
					જાઓ done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#જો defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		જો (flags & HASH_FLAG_VERIFY) {
#નહીંતર
		જો (0) {
#endif
			જો (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				મુક્ત(output);
				પાછા 1;
			}
			જો (memcmp(output, vsum, algo->digest_size) != 0) {
				પૂર્ણાંક i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				માટે (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				મુક્ત(output);
				પાછા 1;
			}
		} નહીંતર {
			hash_show(algo, addr, len, output);
			printf("\n");

			જો (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		મુક્ત(output);

	/* Horrible code size hack for boards that just want crc32 */
	} નહીંતર {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (અચલ uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		જો (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ખાલી);
			*ptr = crc;
		}
	}

	પાછા 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
