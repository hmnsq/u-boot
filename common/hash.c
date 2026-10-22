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

#اگر_غیر_تعریف_شدہ USE_HOSTCC
#شامل <command.h>
#شامل <dm.h>
#شامل <env.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <hw_sha.h>
#شامل <asm/cache.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <linux/errno.h>
#شامل <u-boot/hash.h>
#ورنہ
#شامل "mkimage.h"
#شامل <linux/compiler_attributes.h>
#شامل <time.h>
#شامل <linux/kconfig.h>
#اگر_ختم /* !USE_HOSTCC*/

#شامل <hash.h>
#شامل <image.h>
#شامل <u-boot/crc.h>
#شامل <u-boot/sha1.h>
#شامل <u-boot/sha256.h>
#شامل <u-boot/sha512.h>
#شامل <u-boot/md5.h>
#شامل <u-boot/sm3.h>

ساکن صحیح __maybe_unused hash_init_sha1(ساخت hash_algo *algo, خالی **ctxp)
{
	sha1_context *ctx = حافظہ_دو(حجم(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	واپس 0;
}

ساکن صحیح __maybe_unused hash_update_sha1(ساخت hash_algo *algo, خالی *ctx,
					   ثابت خالی *buf, غیرعلامتی صحیح سائز,
					   صحیح is_آخری)
{
	sha1_update((sha1_context *)ctx, buf, سائز);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_اختتام_sha1(ساخت hash_algo *algo, خالی *ctx,
					   خالی *dest_buf, صحیح سائز)
{
	اگر (سائز < algo->digest_سائز)
		واپس -1;

	sha1_اختتام((sha1_context *)ctx, dest_buf);
	آزاد(ctx);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_init_sha256(ساخت hash_algo *algo, خالی **ctxp)
{
	sha256_context *ctx = حافظہ_دو(حجم(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	واپس 0;
}

ساکن صحیح __maybe_unused hash_update_sha256(ساخت hash_algo *algo, خالی *ctx,
					     ثابت خالی *buf, uint سائز,
					     صحیح is_آخری)
{
	sha256_update((sha256_context *)ctx, buf, سائز);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_اختتام_sha256(ساخت hash_algo *algo, خالی *ctx,
					     خالی *dest_buf, صحیح سائز)
{
	اگر (سائز < algo->digest_سائز)
		واپس -1;

	sha256_اختتام((sha256_context *)ctx, dest_buf);
	آزاد(ctx);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_init_sha384(ساخت hash_algo *algo, خالی **ctxp)
{
	sha512_context *ctx = حافظہ_دو(حجم(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	واپس 0;
}

ساکن صحیح __maybe_unused hash_update_sha384(ساخت hash_algo *algo, خالی *ctx,
					     ثابت خالی *buf, uint سائز,
					     صحیح is_آخری)
{
	sha384_update((sha512_context *)ctx, buf, سائز);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_اختتام_sha384(ساخت hash_algo *algo, خالی *ctx,
					     خالی *dest_buf, صحیح سائز)
{
	اگر (سائز < algo->digest_سائز)
		واپس -1;

	sha384_اختتام((sha512_context *)ctx, dest_buf);
	آزاد(ctx);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_init_sha512(ساخت hash_algo *algo, خالی **ctxp)
{
	sha512_context *ctx = حافظہ_دو(حجم(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	واپس 0;
}

ساکن صحیح __maybe_unused hash_update_sha512(ساخت hash_algo *algo, خالی *ctx,
					     ثابت خالی *buf, uint سائز,
					     صحیح is_آخری)
{
	sha512_update((sha512_context *)ctx, buf, سائز);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_اختتام_sha512(ساخت hash_algo *algo, خالی *ctx,
					     خالی *dest_buf, صحیح سائز)
{
	اگر (سائز < algo->digest_سائز)
		واپس -1;

	sha512_اختتام((sha512_context *)ctx, dest_buf);
	آزاد(ctx);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_init_sm3(ساخت hash_algo *algo, خالی **ctxp)
{
	ساخت sm3_context *ctx = حافظہ_دو(حجم(ساخت sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	واپس 0;
}

ساکن صحیح __maybe_unused hash_update_sm3(ساخت hash_algo *algo, خالی *ctx,
					  ثابت خالی *buf, uint سائز,
					  صحیح is_آخری)
{
	sm3_update((ساخت sm3_context *)ctx, buf, سائز);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_اختتام_sm3(ساخت hash_algo *algo, خالی *ctx,
					  خالی *dest_buf, صحیح سائز)
{
	اگر (سائز < algo->digest_سائز)
		واپس -1;

	sm3_final((ساخت sm3_context *)ctx, dest_buf);
	آزاد(ctx);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_init_crc16_ccitt(ساخت hash_algo *algo,
						خالی **ctxp)
{
	uint16_t *ctx = حافظہ_دو(حجم(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	واپس 0;
}

ساکن صحیح __maybe_unused hash_update_crc16_ccitt(ساخت hash_algo *algo,
						  خالی *ctx, ثابت خالی *buf,
						  غیرعلامتی صحیح سائز,
						  صحیح is_آخری)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, سائز);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_اختتام_crc16_ccitt(ساخت hash_algo *algo,
						  خالی *ctx, خالی *dest_buf,
						  صحیح سائز)
{
	اگر (سائز < algo->digest_سائز)
		واپس -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	آزاد(ctx);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_init_چکری_جانچ_32(ساخت hash_algo *algo, خالی **ctxp)
{
	uint32_t *ctx = حافظہ_دو(حجم(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	واپس 0;
}

ساکن صحیح __maybe_unused hash_update_چکری_جانچ_32(ساخت hash_algo *algo, خالی *ctx,
					    ثابت خالی *buf, غیرعلامتی صحیح سائز,
					    صحیح is_آخری)
{
	*((uint32_t *)ctx) = چکری_جانچ_32(*((uint32_t *)ctx), buf, سائز);
	واپس 0;
}

ساکن صحیح __maybe_unused hash_اختتام_چکری_جانچ_32(ساخت hash_algo *algo, خالی *ctx,
					    خالی *dest_buf, صحیح سائز)
{
	اگر (سائز < algo->digest_سائز)
		واپس -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	آزاد(ctx);
	واپس 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
ساکن ساخت hash_algo hash_algo[] = {
#اگر CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_سائز	= MD5_مجموع_LEN,
		.chunk_سائز	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_سائز	= SHA1_مجموع_LEN,
		.chunk_سائز	= CHUNKSZ_SHA1,
#اگر CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#ورنہ
		.hash_func_ws	= sha1_csum_wd,
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_اختتام	= hw_sha_اختتام,
#ورنہ
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_اختتام	= hash_اختتام_sha1,
#اگر_ختم
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_سائز	= SHA256_مجموع_LEN,
		.chunk_سائز	= CHUNKSZ_SHA256,
#اگر CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#ورنہ
		.hash_func_ws	= sha256_csum_wd,
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_اختتام	= hw_sha_اختتام,
#ورنہ
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_اختتام	= hash_اختتام_sha256,
#اگر_ختم
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_سائز	= SHA384_مجموع_LEN,
		.chunk_سائز	= CHUNKSZ_SHA384,
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#ورنہ
		.hash_func_ws	= sha384_csum_wd,
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_اختتام	= hw_sha_اختتام,
#ورنہ
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_اختتام	= hash_اختتام_sha384,
#اگر_ختم
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_سائز	= SHA512_مجموع_LEN,
		.chunk_سائز	= CHUNKSZ_SHA512,
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#ورنہ
		.hash_func_ws	= sha512_csum_wd,
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_اختتام	= hw_sha_اختتام,
#ورنہ
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_اختتام	= hash_اختتام_sha512,
#اگر_ختم
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_سائز	= SM3_DIGEST_سائز,
		.chunk_سائز	= SM3_بلاک_سائز,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_اختتام	= hash_اختتام_sm3,
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_سائز	= 2,
		.chunk_سائز	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_اختتام	= hash_اختتام_crc16_ccitt,
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_سائز	= 1,
		.chunk_سائز	= CHUNKSZ_چکری_جانچ_32,
		.hash_func_ws	= crc8_wd_buf,
	},
#اگر_ختم
#اگر CONFIG_IS_ENABLED(چکری_جانچ_32_2)
	{
		.name		= "crc32",
		.digest_سائز	= 4,
		.chunk_سائز	= CHUNKSZ_چکری_جانچ_32,
		.hash_func_ws	= چکری_جانچ_32_wd_buf,
		.hash_init	= hash_init_چکری_جانچ_32,
		.hash_update	= hash_update_چکری_جانچ_32,
		.hash_اختتام	= hash_اختتام_چکری_جانچ_32,
	},
#اگر_ختم
};

/* Try to minimize code size for boards that don't want much hashing */
#اگر CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1_مجموع) || \
	CONFIG_IS_ENABLED(چکری_جانچ_32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5_مجموع) || CONFIG_IS_ENABLED(SM3)
#تعریف_کرو multi_hash()	1
#ورنہ
#تعریف_کرو multi_hash()	0
#اگر_ختم

صحیح hash_lookup_algo(ثابت حرف *algo_name, ساخت hash_algo **algop)
{
	صحیح i;

	برائے (i = 0; i < ARRAY_سائز(hash_algo); i++) {
		اگر (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			واپس 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	واپس -EPROTONOSUPPORT;
}

صحیح hash_progressive_lookup_algo(ثابت حرف *algo_name,
				 ساخت hash_algo **algop)
{
	صحیح i;

	برائے (i = 0; i < ARRAY_سائز(hash_algo); i++) {
		اگر (!strcmp(algo_name, hash_algo[i].name)) {
			اگر (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				واپس 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	واپس -EPROTONOSUPPORT;
}

#اگر_غیر_تعریف_شدہ USE_HOSTCC
صحیح hash_parse_string(ثابت حرف *algo_name, ثابت حرف *str, uint8_t *نتیجہ)
{
	ساخت hash_algo *algo;
	صحیح ret;
	صحیح i;

	ret = hash_lookup_algo(algo_name, &algo);
	اگر (ret)
		واپس ret;

	برائے (i = 0; i < algo->digest_سائز; i++) {
		حرف chr[3];

		strlcpy(chr, &str[i * 2], 3);
		نتیجہ[i] = hextoul(chr, تہی);
	}

	واپس 0;
}

صحیح hash_بلاک(ثابت حرف *algo_name, ثابت خالی *ڈیٹا, غیرعلامتی صحیح len,
	       uint8_t *output, صحیح *output_سائز)
{
	ساخت hash_algo *algo;
	صحیح ret;

	ret = hash_lookup_algo(algo_name, &algo);
	اگر (ret)
		واپس ret;

	اگر (output_سائز && *output_سائز < algo->digest_سائز) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_سائز, algo->digest_سائز);
		واپس -ENOSPC;
	}
	اگر (output_سائز)
		*output_سائز = algo->digest_سائز;
	algo->hash_func_ws(ڈیٹا, len, output, algo->chunk_سائز);

	واپس 0;
}

#اگر !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1_مجموع) || defined(CONFIG_CMD_چکری_جانچ_32)) || \
	defined(CONFIG_CMD_MD5_مجموع)
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
ساکن خالی store_نتیجہ(ساخت hash_algo *algo, ثابت uint8_t *مجموع,
			 ثابت حرف *dest, صحیح allow_env_vars)
{
	غیرعلامتی صحیح i;
	صحیح env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	اگر (allow_env_vars) {
		اگر (*dest == '*')
			dest++;
		ورنہ
			env_var = 1;
	}

	اگر (env_var) {
		حرف str_output[HASH_MAX_DIGEST_سائز * 2 + 1];
		حرف *str_ptr = str_output;

		برائے (i = 0; i < algo->digest_سائز; i++) {
			sprintf(str_ptr, "%02x", مجموع[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_سیٹ(dest, str_output);
	} ورنہ {
		ulong addr;
		خالی *buf;

		addr = hextoul(dest, تہی);
		buf = map_sysmem(addr, algo->digest_سائز);
		حافظہ_نقل(buf, مجموع, algo->digest_سائز);
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
ساکن صحیح parse_verify_مجموع(ساخت hash_algo *algo, حرف *verify_str,
			    uint8_t *vsum, صحیح allow_env_vars)
{
	صحیح env_var = 0;

	/* See comment above in store_result() */
	اگر (allow_env_vars) {
		اگر (*verify_str == '*')
			verify_str++;
		ورنہ
			env_var = 1;
	}

	اگر (!env_var) {
		ulong addr;
		خالی *buf;

		addr = hextoul(verify_str, تہی);
		buf = map_sysmem(addr, algo->digest_سائز);
		حافظہ_نقل(vsum, buf, algo->digest_سائز);
	} ورنہ {
		حرف *vsum_str;
		صحیح digits = algo->digest_سائز * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		اگر (لڑی_لمبائی(verify_str) == digits)
			vsum_str = verify_str;
		ورنہ {
			vsum_str = env_get(verify_str);
			اگر (vsum_str == تہی || لڑی_لمبائی(vsum_str) != digits) {
				لکھو("Expected %d hex digits in env var\n",
				       digits);
				واپس 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	واپس 0;
}

ساکن خالی hash_show(ساخت hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	صحیح i;

	لکھو("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	برائے (i = 0; i < algo->digest_سائز; i++)
		لکھو("%02x", output[i]);
}

صحیح hash_command(ثابت حرف *algo_name, صحیح flags, ساخت cmd_tbl *cmdtp,
		 صحیح flag, صحیح argc, حرف *ثابت argv[])
{
	ulong addr, len;

	اگر ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		واپس CMD_RET_USAGE;

	addr = hextoul(*argv++, تہی);
	len = hextoul(*argv++, تہی);

	اگر (multi_hash()) {
		ساخت hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_سائز];
		خالی *buf;

		اگر (hash_lookup_algo(algo_name, &algo)) {
			لکھو("Unknown hash algorithm '%s'\n", algo_name);
			واپس CMD_RET_USAGE;
		}
		argc -= 2;

		اگر (algo->digest_سائز > HASH_MAX_DIGEST_سائز) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			واپس 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  حجم(uint32_t) * HASH_MAX_DIGEST_سائز);
		اگر (!output)
			واپس CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		اگر (CONFIG_IS_ENABLED(DM_HASH)) {
			شمار HASH_ALGO hash_algo;
			صحیح ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			اگر (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_سائز);
				اگر (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					آزاد(output);
					واپس CMD_RET_FAILURE;
				}
				اگر (!ret)
					جاؤ مکمل;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_سائز);
مکمل:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#اگر defined(CONFIG_چکری_جانچ_32_VERIFY) || defined(CONFIG_SHA1_مجموع_VERIFY) || \
	defined(CONFIG_MD5_مجموع_VERIFY) || defined(CONFIG_HASH_VERIFY)
		اگر (flags & HASH_FLAG_VERIFY) {
#ورنہ
		اگر (0) {
#اگر_ختم
			اگر (parse_verify_مجموع(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				لکھو("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				آزاد(output);
				واپس 1;
			}
			اگر (memcmp(output, vsum, algo->digest_سائز) != 0) {
				صحیح i;

				hash_show(algo, addr, len, output);
				لکھو(" != ");
				برائے (i = 0; i < algo->digest_سائز; i++)
					لکھو("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				آزاد(output);
				واپس 1;
			}
		} ورنہ {
			hash_show(algo, addr, len, output);
			لکھو("\n");

			اگر (argc) {
				store_نتیجہ(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		آزاد(output);

	/* Horrible code size hack for boards that just want crc32 */
	} ورنہ {
		ulong چکری_جانچ;
		ulong *ptr;

		چکری_جانچ = چکری_جانچ_32_wd(0, (ثابت uchar *)addr, len, CHUNKSZ_چکری_جانچ_32);

		لکھو("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, چکری_جانچ);

		اگر (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], تہی);
			*ptr = چکری_جانچ;
		}
	}

	واپس 0;
}
#اگر_ختم /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#اگر_ختم /* !USE_HOSTCC */
