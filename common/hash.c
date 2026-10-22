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

#నిర్వచితం_కాకపోతే USE_HOSTCC
#అన్తర్భావ <command.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <hw_sha.h>
#అన్తర్భావ <asm/cache.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <linux/errno.h>
#అన్తర్భావ <u-boot/hash.h>
#లేకపోతే
#అన్తర్భావ "mkimage.h"
#అన్తర్భావ <linux/compiler_attributes.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <linux/kconfig.h>
#అయితే_ముగింపు /* !USE_HOSTCC*/

#అన్తర్భావ <hash.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <u-boot/crc.h>
#అన్తర్భావ <u-boot/sha1.h>
#అన్తర్భావ <u-boot/sha256.h>
#అన్తర్భావ <u-boot/sha512.h>
#అన్తర్భావ <u-boot/md5.h>
#అన్తర్భావ <u-boot/sm3.h>

స్థిర పూర్ణాంకం __maybe_unused hash_init_sha1(నిర్మాణం hash_algo *algo, శూన్య **ctxp)
{
	sha1_context *ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_update_sha1(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					   స్థిరరాశి శూన్య *buf, గుర్తులేని పూర్ణాంకం కొలత,
					   పూర్ణాంకం is_చివరి)
{
	sha1_update((sha1_context *)ctx, buf, కొలత);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_ముగింపు_sha1(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					   శూన్య *dest_buf, పూర్ణాంకం కొలత)
{
	ఒకవేళ (కొలత < algo->digest_కొలత)
		తిరిగి -1;

	sha1_ముగింపు((sha1_context *)ctx, dest_buf);
	విడుదల(ctx);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_init_sha256(నిర్మాణం hash_algo *algo, శూన్య **ctxp)
{
	sha256_context *ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_update_sha256(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					     స్థిరరాశి శూన్య *buf, uint కొలత,
					     పూర్ణాంకం is_చివరి)
{
	sha256_update((sha256_context *)ctx, buf, కొలత);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_ముగింపు_sha256(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					     శూన్య *dest_buf, పూర్ణాంకం కొలత)
{
	ఒకవేళ (కొలత < algo->digest_కొలత)
		తిరిగి -1;

	sha256_ముగింపు((sha256_context *)ctx, dest_buf);
	విడుదల(ctx);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_init_sha384(నిర్మాణం hash_algo *algo, శూన్య **ctxp)
{
	sha512_context *ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_update_sha384(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					     స్థిరరాశి శూన్య *buf, uint కొలత,
					     పూర్ణాంకం is_చివరి)
{
	sha384_update((sha512_context *)ctx, buf, కొలత);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_ముగింపు_sha384(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					     శూన్య *dest_buf, పూర్ణాంకం కొలత)
{
	ఒకవేళ (కొలత < algo->digest_కొలత)
		తిరిగి -1;

	sha384_ముగింపు((sha512_context *)ctx, dest_buf);
	విడుదల(ctx);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_init_sha512(నిర్మాణం hash_algo *algo, శూన్య **ctxp)
{
	sha512_context *ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_update_sha512(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					     స్థిరరాశి శూన్య *buf, uint కొలత,
					     పూర్ణాంకం is_చివరి)
{
	sha512_update((sha512_context *)ctx, buf, కొలత);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_ముగింపు_sha512(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					     శూన్య *dest_buf, పూర్ణాంకం కొలత)
{
	ఒకవేళ (కొలత < algo->digest_కొలత)
		తిరిగి -1;

	sha512_ముగింపు((sha512_context *)ctx, dest_buf);
	విడుదల(ctx);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_init_sm3(నిర్మాణం hash_algo *algo, శూన్య **ctxp)
{
	నిర్మాణం sm3_context *ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_update_sm3(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					  స్థిరరాశి శూన్య *buf, uint కొలత,
					  పూర్ణాంకం is_చివరి)
{
	sm3_update((నిర్మాణం sm3_context *)ctx, buf, కొలత);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_ముగింపు_sm3(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					  శూన్య *dest_buf, పూర్ణాంకం కొలత)
{
	ఒకవేళ (కొలత < algo->digest_కొలత)
		తిరిగి -1;

	sm3_final((నిర్మాణం sm3_context *)ctx, dest_buf);
	విడుదల(ctx);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_init_crc16_ccitt(నిర్మాణం hash_algo *algo,
						శూన్య **ctxp)
{
	uint16_t *ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_update_crc16_ccitt(నిర్మాణం hash_algo *algo,
						  శూన్య *ctx, స్థిరరాశి శూన్య *buf,
						  గుర్తులేని పూర్ణాంకం కొలత,
						  పూర్ణాంకం is_చివరి)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, కొలత);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_ముగింపు_crc16_ccitt(నిర్మాణం hash_algo *algo,
						  శూన్య *ctx, శూన్య *dest_buf,
						  పూర్ణాంకం కొలత)
{
	ఒకవేళ (కొలత < algo->digest_కొలత)
		తిరిగి -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	విడుదల(ctx);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_init_చక్రీయ_తనిఖీ_32(నిర్మాణం hash_algo *algo, శూన్య **ctxp)
{
	uint32_t *ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_update_చక్రీయ_తనిఖీ_32(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					    స్థిరరాశి శూన్య *buf, గుర్తులేని పూర్ణాంకం కొలత,
					    పూర్ణాంకం is_చివరి)
{
	*((uint32_t *)ctx) = చక్రీయ_తనిఖీ_32(*((uint32_t *)ctx), buf, కొలత);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused hash_ముగింపు_చక్రీయ_తనిఖీ_32(నిర్మాణం hash_algo *algo, శూన్య *ctx,
					    శూన్య *dest_buf, పూర్ణాంకం కొలత)
{
	ఒకవేళ (కొలత < algo->digest_కొలత)
		తిరిగి -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	విడుదల(ctx);
	తిరిగి 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
స్థిర నిర్మాణం hash_algo hash_algo[] = {
#ఒకవేళ CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_కొలత	= MD5_కూడిక_LEN,
		.chunk_కొలత	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_కొలత	= SHA1_కూడిక_LEN,
		.chunk_కొలత	= CHUNKSZ_SHA1,
#ఒకవేళ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#లేకపోతే
		.hash_func_ws	= sha1_csum_wd,
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ముగింపు	= hw_sha_ముగింపు,
#లేకపోతే
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_ముగింపు	= hash_ముగింపు_sha1,
#అయితే_ముగింపు
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_కొలత	= SHA256_కూడిక_LEN,
		.chunk_కొలత	= CHUNKSZ_SHA256,
#ఒకవేళ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#లేకపోతే
		.hash_func_ws	= sha256_csum_wd,
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ముగింపు	= hw_sha_ముగింపు,
#లేకపోతే
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_ముగింపు	= hash_ముగింపు_sha256,
#అయితే_ముగింపు
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_కొలత	= SHA384_కూడిక_LEN,
		.chunk_కొలత	= CHUNKSZ_SHA384,
#ఒకవేళ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#లేకపోతే
		.hash_func_ws	= sha384_csum_wd,
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ముగింపు	= hw_sha_ముగింపు,
#లేకపోతే
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_ముగింపు	= hash_ముగింపు_sha384,
#అయితే_ముగింపు
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_కొలత	= SHA512_కూడిక_LEN,
		.chunk_కొలత	= CHUNKSZ_SHA512,
#ఒకవేళ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#లేకపోతే
		.hash_func_ws	= sha512_csum_wd,
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ముగింపు	= hw_sha_ముగింపు,
#లేకపోతే
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_ముగింపు	= hash_ముగింపు_sha512,
#అయితే_ముగింపు
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_కొలత	= SM3_DIGEST_కొలత,
		.chunk_కొలత	= SM3_ఖండం_కొలత,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_ముగింపు	= hash_ముగింపు_sm3,
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_కొలత	= 2,
		.chunk_కొలత	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_ముగింపు	= hash_ముగింపు_crc16_ccitt,
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_కొలత	= 1,
		.chunk_కొలత	= CHUNKSZ_చక్రీయ_తనిఖీ_32,
		.hash_func_ws	= crc8_wd_buf,
	},
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(చక్రీయ_తనిఖీ_32_2)
	{
		.name		= "crc32",
		.digest_కొలత	= 4,
		.chunk_కొలత	= CHUNKSZ_చక్రీయ_తనిఖీ_32,
		.hash_func_ws	= చక్రీయ_తనిఖీ_32_wd_buf,
		.hash_init	= hash_init_చక్రీయ_తనిఖీ_32,
		.hash_update	= hash_update_చక్రీయ_తనిఖీ_32,
		.hash_ముగింపు	= hash_ముగింపు_చక్రీయ_తనిఖీ_32,
	},
#అయితే_ముగింపు
};

/* Try to minimize code size for boards that don't want much hashing */
#ఒకవేళ CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1_కూడిక) || \
	CONFIG_IS_ENABLED(చక్రీయ_తనిఖీ_32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5_కూడిక) || CONFIG_IS_ENABLED(SM3)
#నిర్వచించు multi_hash()	1
#లేకపోతే
#నిర్వచించు multi_hash()	0
#అయితే_ముగింపు

పూర్ణాంకం hash_lookup_algo(స్థిరరాశి అక్షరం *algo_name, నిర్మాణం hash_algo **algop)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < ARRAY_కొలత(hash_algo); i++) {
		ఒకవేళ (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			తిరిగి 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	తిరిగి -EPROTONOSUPPORT;
}

పూర్ణాంకం hash_progressive_lookup_algo(స్థిరరాశి అక్షరం *algo_name,
				 నిర్మాణం hash_algo **algop)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < ARRAY_కొలత(hash_algo); i++) {
		ఒకవేళ (!strcmp(algo_name, hash_algo[i].name)) {
			ఒకవేళ (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				తిరిగి 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	తిరిగి -EPROTONOSUPPORT;
}

#నిర్వచితం_కాకపోతే USE_HOSTCC
పూర్ణాంకం hash_parse_string(స్థిరరాశి అక్షరం *algo_name, స్థిరరాశి అక్షరం *str, uint8_t *ఫలితం)
{
	నిర్మాణం hash_algo *algo;
	పూర్ణాంకం ret;
	పూర్ణాంకం i;

	ret = hash_lookup_algo(algo_name, &algo);
	ఒకవేళ (ret)
		తిరిగి ret;

	కొరకు (i = 0; i < algo->digest_కొలత; i++) {
		అక్షరం chr[3];

		strlcpy(chr, &str[i * 2], 3);
		ఫలితం[i] = hextoul(chr, ఖాళీ);
	}

	తిరిగి 0;
}

పూర్ణాంకం hash_ఖండం(స్థిరరాశి అక్షరం *algo_name, స్థిరరాశి శూన్య *దత్తాంశం, గుర్తులేని పూర్ణాంకం len,
	       uint8_t *output, పూర్ణాంకం *output_కొలత)
{
	నిర్మాణం hash_algo *algo;
	పూర్ణాంకం ret;

	ret = hash_lookup_algo(algo_name, &algo);
	ఒకవేళ (ret)
		తిరిగి ret;

	ఒకవేళ (output_కొలత && *output_కొలత < algo->digest_కొలత) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_కొలత, algo->digest_కొలత);
		తిరిగి -ENOSPC;
	}
	ఒకవేళ (output_కొలత)
		*output_కొలత = algo->digest_కొలత;
	algo->hash_func_ws(దత్తాంశం, len, output, algo->chunk_కొలత);

	తిరిగి 0;
}

#ఒకవేళ !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1_కూడిక) || defined(CONFIG_CMD_చక్రీయ_తనిఖీ_32)) || \
	defined(CONFIG_CMD_MD5_కూడిక)
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
స్థిర శూన్య store_ఫలితం(నిర్మాణం hash_algo *algo, స్థిరరాశి uint8_t *కూడిక,
			 స్థిరరాశి అక్షరం *dest, పూర్ణాంకం allow_env_vars)
{
	గుర్తులేని పూర్ణాంకం i;
	పూర్ణాంకం env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	ఒకవేళ (allow_env_vars) {
		ఒకవేళ (*dest == '*')
			dest++;
		లేకపోతే
			env_var = 1;
	}

	ఒకవేళ (env_var) {
		అక్షరం str_output[HASH_MAX_DIGEST_కొలత * 2 + 1];
		అక్షరం *str_ptr = str_output;

		కొరకు (i = 0; i < algo->digest_కొలత; i++) {
			sprintf(str_ptr, "%02x", కూడిక[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_సమితి(dest, str_output);
	} లేకపోతే {
		ulong addr;
		శూన్య *buf;

		addr = hextoul(dest, ఖాళీ);
		buf = map_sysmem(addr, algo->digest_కొలత);
		జ్ఞాపకం_నకలు(buf, కూడిక, algo->digest_కొలత);
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
స్థిర పూర్ణాంకం parse_verify_కూడిక(నిర్మాణం hash_algo *algo, అక్షరం *verify_str,
			    uint8_t *vsum, పూర్ణాంకం allow_env_vars)
{
	పూర్ణాంకం env_var = 0;

	/* See comment above in store_result() */
	ఒకవేళ (allow_env_vars) {
		ఒకవేళ (*verify_str == '*')
			verify_str++;
		లేకపోతే
			env_var = 1;
	}

	ఒకవేళ (!env_var) {
		ulong addr;
		శూన్య *buf;

		addr = hextoul(verify_str, ఖాళీ);
		buf = map_sysmem(addr, algo->digest_కొలత);
		జ్ఞాపకం_నకలు(vsum, buf, algo->digest_కొలత);
	} లేకపోతే {
		అక్షరం *vsum_str;
		పూర్ణాంకం digits = algo->digest_కొలత * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		ఒకవేళ (అక్షరశ్రేణి_పొడవు(verify_str) == digits)
			vsum_str = verify_str;
		లేకపోతే {
			vsum_str = env_get(verify_str);
			ఒకవేళ (vsum_str == ఖాళీ || అక్షరశ్రేణి_పొడవు(vsum_str) != digits) {
				రాయి("Expected %d hex digits in env var\n",
				       digits);
				తిరిగి 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	తిరిగి 0;
}

స్థిర శూన్య hash_show(నిర్మాణం hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	పూర్ణాంకం i;

	రాయి("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	కొరకు (i = 0; i < algo->digest_కొలత; i++)
		రాయి("%02x", output[i]);
}

పూర్ణాంకం hash_command(స్థిరరాశి అక్షరం *algo_name, పూర్ణాంకం flags, నిర్మాణం cmd_tbl *cmdtp,
		 పూర్ణాంకం flag, పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[])
{
	ulong addr, len;

	ఒకవేళ ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		తిరిగి CMD_RET_USAGE;

	addr = hextoul(*argv++, ఖాళీ);
	len = hextoul(*argv++, ఖాళీ);

	ఒకవేళ (multi_hash()) {
		నిర్మాణం hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_కొలత];
		శూన్య *buf;

		ఒకవేళ (hash_lookup_algo(algo_name, &algo)) {
			రాయి("Unknown hash algorithm '%s'\n", algo_name);
			తిరిగి CMD_RET_USAGE;
		}
		argc -= 2;

		ఒకవేళ (algo->digest_కొలత > HASH_MAX_DIGEST_కొలత) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			తిరిగి 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  పరిమాణం(uint32_t) * HASH_MAX_DIGEST_కొలత);
		ఒకవేళ (!output)
			తిరిగి CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		ఒకవేళ (CONFIG_IS_ENABLED(DM_HASH)) {
			గణన HASH_ALGO hash_algo;
			పూర్ణాంకం ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			ఒకవేళ (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_కొలత);
				ఒకవేళ (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					విడుదల(output);
					తిరిగి CMD_RET_FAILURE;
				}
				ఒకవేళ (!ret)
					వెళ్ళు పూర్తి;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_కొలత);
పూర్తి:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#ఒకవేళ defined(CONFIG_చక్రీయ_తనిఖీ_32_VERIFY) || defined(CONFIG_SHA1_కూడిక_VERIFY) || \
	defined(CONFIG_MD5_కూడిక_VERIFY) || defined(CONFIG_HASH_VERIFY)
		ఒకవేళ (flags & HASH_FLAG_VERIFY) {
#లేకపోతే
		ఒకవేళ (0) {
#అయితే_ముగింపు
			ఒకవేళ (parse_verify_కూడిక(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				రాయి("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				విడుదల(output);
				తిరిగి 1;
			}
			ఒకవేళ (memcmp(output, vsum, algo->digest_కొలత) != 0) {
				పూర్ణాంకం i;

				hash_show(algo, addr, len, output);
				రాయి(" != ");
				కొరకు (i = 0; i < algo->digest_కొలత; i++)
					రాయి("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				విడుదల(output);
				తిరిగి 1;
			}
		} లేకపోతే {
			hash_show(algo, addr, len, output);
			రాయి("\n");

			ఒకవేళ (argc) {
				store_ఫలితం(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		విడుదల(output);

	/* Horrible code size hack for boards that just want crc32 */
	} లేకపోతే {
		ulong చక్రీయ_తనిఖీ;
		ulong *ptr;

		చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_wd(0, (స్థిరరాశి uchar *)addr, len, CHUNKSZ_చక్రీయ_తనిఖీ_32);

		రాయి("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, చక్రీయ_తనిఖీ);

		ఒకవేళ (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ఖాళీ);
			*ptr = చక్రీయ_తనిఖీ;
		}
	}

	తిరిగి 0;
}
#అయితే_ముగింపు /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#అయితే_ముగింపు /* !USE_HOSTCC */
