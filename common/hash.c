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

#جيڪڏهن_وصف_نه_ٿيل USE_HOSTCC
#ضمن <command.h>
#ضمن <dm.h>
#ضمن <env.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <hw_sha.h>
#ضمن <asm/cache.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <linux/errno.h>
#ضمن <u-boot/hash.h>
#وإلا
#ضمن "mkimage.h"
#ضمن <linux/compiler_attributes.h>
#ضمن <time.h>
#ضمن <linux/kconfig.h>
#جيڪڏهن_پورو /* !USE_HOSTCC*/

#ضمن <hash.h>
#ضمن <image.h>
#ضمن <u-boot/crc.h>
#ضمن <u-boot/sha1.h>
#ضمن <u-boot/sha256.h>
#ضمن <u-boot/sha512.h>
#ضمن <u-boot/md5.h>
#ضمن <u-boot/sm3.h>

ساكن صحيح __maybe_unused hash_init_sha1(بنية hash_algo *algo, فراغ **ctxp)
{
	sha1_context *ctx = يادگيري_ڏيو(حجم(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_update_sha1(بنية hash_algo *algo, فراغ *ctx,
					   ثابت فراغ *buf, بنانشان صحيح سائيز,
					   صحيح is_آخري)
{
	sha1_update((sha1_context *)ctx, buf, سائيز);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_پڄاڻي_sha1(بنية hash_algo *algo, فراغ *ctx,
					   فراغ *dest_buf, صحيح سائيز)
{
	إذا (سائيز < algo->digest_سائيز)
		أرجع -1;

	sha1_پڄاڻي((sha1_context *)ctx, dest_buf);
	حرر(ctx);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_init_sha256(بنية hash_algo *algo, فراغ **ctxp)
{
	sha256_context *ctx = يادگيري_ڏيو(حجم(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_update_sha256(بنية hash_algo *algo, فراغ *ctx,
					     ثابت فراغ *buf, uint سائيز,
					     صحيح is_آخري)
{
	sha256_update((sha256_context *)ctx, buf, سائيز);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_پڄاڻي_sha256(بنية hash_algo *algo, فراغ *ctx,
					     فراغ *dest_buf, صحيح سائيز)
{
	إذا (سائيز < algo->digest_سائيز)
		أرجع -1;

	sha256_پڄاڻي((sha256_context *)ctx, dest_buf);
	حرر(ctx);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_init_sha384(بنية hash_algo *algo, فراغ **ctxp)
{
	sha512_context *ctx = يادگيري_ڏيو(حجم(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_update_sha384(بنية hash_algo *algo, فراغ *ctx,
					     ثابت فراغ *buf, uint سائيز,
					     صحيح is_آخري)
{
	sha384_update((sha512_context *)ctx, buf, سائيز);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_پڄاڻي_sha384(بنية hash_algo *algo, فراغ *ctx,
					     فراغ *dest_buf, صحيح سائيز)
{
	إذا (سائيز < algo->digest_سائيز)
		أرجع -1;

	sha384_پڄاڻي((sha512_context *)ctx, dest_buf);
	حرر(ctx);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_init_sha512(بنية hash_algo *algo, فراغ **ctxp)
{
	sha512_context *ctx = يادگيري_ڏيو(حجم(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_update_sha512(بنية hash_algo *algo, فراغ *ctx,
					     ثابت فراغ *buf, uint سائيز,
					     صحيح is_آخري)
{
	sha512_update((sha512_context *)ctx, buf, سائيز);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_پڄاڻي_sha512(بنية hash_algo *algo, فراغ *ctx,
					     فراغ *dest_buf, صحيح سائيز)
{
	إذا (سائيز < algo->digest_سائيز)
		أرجع -1;

	sha512_پڄاڻي((sha512_context *)ctx, dest_buf);
	حرر(ctx);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_init_sm3(بنية hash_algo *algo, فراغ **ctxp)
{
	بنية sm3_context *ctx = يادگيري_ڏيو(حجم(بنية sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_update_sm3(بنية hash_algo *algo, فراغ *ctx,
					  ثابت فراغ *buf, uint سائيز,
					  صحيح is_آخري)
{
	sm3_update((بنية sm3_context *)ctx, buf, سائيز);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_پڄاڻي_sm3(بنية hash_algo *algo, فراغ *ctx,
					  فراغ *dest_buf, صحيح سائيز)
{
	إذا (سائيز < algo->digest_سائيز)
		أرجع -1;

	sm3_final((بنية sm3_context *)ctx, dest_buf);
	حرر(ctx);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_init_crc16_ccitt(بنية hash_algo *algo,
						فراغ **ctxp)
{
	uint16_t *ctx = يادگيري_ڏيو(حجم(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_update_crc16_ccitt(بنية hash_algo *algo,
						  فراغ *ctx, ثابت فراغ *buf,
						  بنانشان صحيح سائيز,
						  صحيح is_آخري)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, سائيز);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_پڄاڻي_crc16_ccitt(بنية hash_algo *algo,
						  فراغ *ctx, فراغ *dest_buf,
						  صحيح سائيز)
{
	إذا (سائيز < algo->digest_سائيز)
		أرجع -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	حرر(ctx);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_init_چڪري_جاچ_32(بنية hash_algo *algo, فراغ **ctxp)
{
	uint32_t *ctx = يادگيري_ڏيو(حجم(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_update_چڪري_جاچ_32(بنية hash_algo *algo, فراغ *ctx,
					    ثابت فراغ *buf, بنانشان صحيح سائيز,
					    صحيح is_آخري)
{
	*((uint32_t *)ctx) = چڪري_جاچ_32(*((uint32_t *)ctx), buf, سائيز);
	أرجع 0;
}

ساكن صحيح __maybe_unused hash_پڄاڻي_چڪري_جاچ_32(بنية hash_algo *algo, فراغ *ctx,
					    فراغ *dest_buf, صحيح سائيز)
{
	إذا (سائيز < algo->digest_سائيز)
		أرجع -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	حرر(ctx);
	أرجع 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
ساكن بنية hash_algo hash_algo[] = {
#إذا CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_سائيز	= MD5_جوڙ_LEN,
		.chunk_سائيز	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_سائيز	= SHA1_جوڙ_LEN,
		.chunk_سائيز	= CHUNKSZ_SHA1,
#إذا CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#وإلا
		.hash_func_ws	= sha1_csum_wd,
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_پڄاڻي	= hw_sha_پڄاڻي,
#وإلا
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_پڄاڻي	= hash_پڄاڻي_sha1,
#جيڪڏهن_پورو
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_سائيز	= SHA256_جوڙ_LEN,
		.chunk_سائيز	= CHUNKSZ_SHA256,
#إذا CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#وإلا
		.hash_func_ws	= sha256_csum_wd,
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_پڄاڻي	= hw_sha_پڄاڻي,
#وإلا
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_پڄاڻي	= hash_پڄاڻي_sha256,
#جيڪڏهن_پورو
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_سائيز	= SHA384_جوڙ_LEN,
		.chunk_سائيز	= CHUNKSZ_SHA384,
#إذا CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#وإلا
		.hash_func_ws	= sha384_csum_wd,
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_پڄاڻي	= hw_sha_پڄاڻي,
#وإلا
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_پڄاڻي	= hash_پڄاڻي_sha384,
#جيڪڏهن_پورو
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_سائيز	= SHA512_جوڙ_LEN,
		.chunk_سائيز	= CHUNKSZ_SHA512,
#إذا CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#وإلا
		.hash_func_ws	= sha512_csum_wd,
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_پڄاڻي	= hw_sha_پڄاڻي,
#وإلا
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_پڄاڻي	= hash_پڄاڻي_sha512,
#جيڪڏهن_پورو
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_سائيز	= SM3_DIGEST_سائيز,
		.chunk_سائيز	= SM3_بلاڪ_سائيز,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_پڄاڻي	= hash_پڄاڻي_sm3,
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_سائيز	= 2,
		.chunk_سائيز	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_پڄاڻي	= hash_پڄاڻي_crc16_ccitt,
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_سائيز	= 1,
		.chunk_سائيز	= CHUNKSZ_چڪري_جاچ_32,
		.hash_func_ws	= crc8_wd_buf,
	},
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(چڪري_جاچ_32_2)
	{
		.name		= "crc32",
		.digest_سائيز	= 4,
		.chunk_سائيز	= CHUNKSZ_چڪري_جاچ_32,
		.hash_func_ws	= چڪري_جاچ_32_wd_buf,
		.hash_init	= hash_init_چڪري_جاچ_32,
		.hash_update	= hash_update_چڪري_جاچ_32,
		.hash_پڄاڻي	= hash_پڄاڻي_چڪري_جاچ_32,
	},
#جيڪڏهن_پورو
};

/* Try to minimize code size for boards that don't want much hashing */
#إذا CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1_جوڙ) || \
	CONFIG_IS_ENABLED(چڪري_جاچ_32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5_جوڙ) || CONFIG_IS_ENABLED(SM3)
#وصف_ڪريو multi_hash()	1
#وإلا
#وصف_ڪريو multi_hash()	0
#جيڪڏهن_پورو

صحيح hash_lookup_algo(ثابت حرف *algo_name, بنية hash_algo **algop)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_سائيز(hash_algo); i++) {
		إذا (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			أرجع 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	أرجع -EPROTONOSUPPORT;
}

صحيح hash_progressive_lookup_algo(ثابت حرف *algo_name,
				 بنية hash_algo **algop)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_سائيز(hash_algo); i++) {
		إذا (!strcmp(algo_name, hash_algo[i].name)) {
			إذا (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				أرجع 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	أرجع -EPROTONOSUPPORT;
}

#جيڪڏهن_وصف_نه_ٿيل USE_HOSTCC
صحيح hash_parse_string(ثابت حرف *algo_name, ثابت حرف *str, uint8_t *نتيجو)
{
	بنية hash_algo *algo;
	صحيح ret;
	صحيح i;

	ret = hash_lookup_algo(algo_name, &algo);
	إذا (ret)
		أرجع ret;

	لكل (i = 0; i < algo->digest_سائيز; i++) {
		حرف chr[3];

		strlcpy(chr, &str[i * 2], 3);
		نتيجو[i] = hextoul(chr, عدم);
	}

	أرجع 0;
}

صحيح hash_بلاڪ(ثابت حرف *algo_name, ثابت فراغ *معلومات, بنانشان صحيح len,
	       uint8_t *output, صحيح *output_سائيز)
{
	بنية hash_algo *algo;
	صحيح ret;

	ret = hash_lookup_algo(algo_name, &algo);
	إذا (ret)
		أرجع ret;

	إذا (output_سائيز && *output_سائيز < algo->digest_سائيز) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_سائيز, algo->digest_سائيز);
		أرجع -ENOSPC;
	}
	إذا (output_سائيز)
		*output_سائيز = algo->digest_سائيز;
	algo->hash_func_ws(معلومات, len, output, algo->chunk_سائيز);

	أرجع 0;
}

#إذا !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1_جوڙ) || defined(CONFIG_CMD_چڪري_جاچ_32)) || \
	defined(CONFIG_CMD_MD5_جوڙ)
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
ساكن فراغ store_نتيجو(بنية hash_algo *algo, ثابت uint8_t *جوڙ,
			 ثابت حرف *dest, صحيح allow_env_vars)
{
	بنانشان صحيح i;
	صحيح env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	إذا (allow_env_vars) {
		إذا (*dest == '*')
			dest++;
		وإلا
			env_var = 1;
	}

	إذا (env_var) {
		حرف str_output[HASH_MAX_DIGEST_سائيز * 2 + 1];
		حرف *str_ptr = str_output;

		لكل (i = 0; i < algo->digest_سائيز; i++) {
			sprintf(str_ptr, "%02x", جوڙ[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_سيٽ(dest, str_output);
	} وإلا {
		ulong addr;
		فراغ *buf;

		addr = hextoul(dest, عدم);
		buf = map_sysmem(addr, algo->digest_سائيز);
		يادگيري_نقل(buf, جوڙ, algo->digest_سائيز);
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
ساكن صحيح parse_verify_جوڙ(بنية hash_algo *algo, حرف *verify_str,
			    uint8_t *vsum, صحيح allow_env_vars)
{
	صحيح env_var = 0;

	/* See comment above in store_result() */
	إذا (allow_env_vars) {
		إذا (*verify_str == '*')
			verify_str++;
		وإلا
			env_var = 1;
	}

	إذا (!env_var) {
		ulong addr;
		فراغ *buf;

		addr = hextoul(verify_str, عدم);
		buf = map_sysmem(addr, algo->digest_سائيز);
		يادگيري_نقل(vsum, buf, algo->digest_سائيز);
	} وإلا {
		حرف *vsum_str;
		صحيح digits = algo->digest_سائيز * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		إذا (لڙي_ڊيگهه(verify_str) == digits)
			vsum_str = verify_str;
		وإلا {
			vsum_str = env_get(verify_str);
			إذا (vsum_str == عدم || لڙي_ڊيگهه(vsum_str) != digits) {
				لکو("Expected %d hex digits in env var\n",
				       digits);
				أرجع 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	أرجع 0;
}

ساكن فراغ hash_show(بنية hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	صحيح i;

	لکو("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	لكل (i = 0; i < algo->digest_سائيز; i++)
		لکو("%02x", output[i]);
}

صحيح hash_command(ثابت حرف *algo_name, صحيح flags, بنية cmd_tbl *cmdtp,
		 صحيح flag, صحيح argc, حرف *ثابت argv[])
{
	ulong addr, len;

	إذا ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		أرجع CMD_RET_USAGE;

	addr = hextoul(*argv++, عدم);
	len = hextoul(*argv++, عدم);

	إذا (multi_hash()) {
		بنية hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_سائيز];
		فراغ *buf;

		إذا (hash_lookup_algo(algo_name, &algo)) {
			لکو("Unknown hash algorithm '%s'\n", algo_name);
			أرجع CMD_RET_USAGE;
		}
		argc -= 2;

		إذا (algo->digest_سائيز > HASH_MAX_DIGEST_سائيز) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			أرجع 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  حجم(uint32_t) * HASH_MAX_DIGEST_سائيز);
		إذا (!output)
			أرجع CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		إذا (CONFIG_IS_ENABLED(DM_HASH)) {
			تعداد HASH_ALGO hash_algo;
			صحيح ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			إذا (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_سائيز);
				إذا (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					حرر(output);
					أرجع CMD_RET_FAILURE;
				}
				إذا (!ret)
					اذهب پورو;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_سائيز);
پورو:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#إذا defined(CONFIG_چڪري_جاچ_32_VERIFY) || defined(CONFIG_SHA1_جوڙ_VERIFY) || \
	defined(CONFIG_MD5_جوڙ_VERIFY) || defined(CONFIG_HASH_VERIFY)
		إذا (flags & HASH_FLAG_VERIFY) {
#وإلا
		إذا (0) {
#جيڪڏهن_پورو
			إذا (parse_verify_جوڙ(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				لکو("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				حرر(output);
				أرجع 1;
			}
			إذا (memcmp(output, vsum, algo->digest_سائيز) != 0) {
				صحيح i;

				hash_show(algo, addr, len, output);
				لکو(" != ");
				لكل (i = 0; i < algo->digest_سائيز; i++)
					لکو("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				حرر(output);
				أرجع 1;
			}
		} وإلا {
			hash_show(algo, addr, len, output);
			لکو("\n");

			إذا (argc) {
				store_نتيجو(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		حرر(output);

	/* Horrible code size hack for boards that just want crc32 */
	} وإلا {
		ulong چڪري_جاچ;
		ulong *ptr;

		چڪري_جاچ = چڪري_جاچ_32_wd(0, (ثابت uchar *)addr, len, CHUNKSZ_چڪري_جاچ_32);

		لکو("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, چڪري_جاچ);

		إذا (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], عدم);
			*ptr = چڪري_جاچ;
		}
	}

	أرجع 0;
}
#جيڪڏهن_پورو /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#جيڪڏهن_پورو /* !USE_HOSTCC */
