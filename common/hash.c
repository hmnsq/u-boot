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
#अन्तर्भाव <command.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <hw_sha.h>
#अन्तर्भाव <asm/cache.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <linux/errno.h>
#अन्तर्भाव <u-boot/hash.h>
#अन्यथा
#अन्तर्भाव "mkimage.h"
#अन्तर्भाव <linux/compiler_attributes.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#अन्तर्भाव <hash.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <u-boot/crc.h>
#अन्तर्भाव <u-boot/sha1.h>
#अन्तर्भाव <u-boot/sha256.h>
#अन्तर्भाव <u-boot/sha512.h>
#अन्तर्भाव <u-boot/md5.h>
#अन्तर्भाव <u-boot/sm3.h>

स्थिर पूर्णाङ्क __maybe_unused hash_init_sha1(संरचना hash_algo *algo, शून्य **ctxp)
{
	sha1_context *ctx = दो_स्मृति(परिमाण(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_update_sha1(संरचना hash_algo *algo, शून्य *ctx,
					   ध्रुव शून्य *buf, अचिन्हित पूर्णाङ्क size,
					   पूर्णाङ्क is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_finish_sha1(संरचना hash_algo *algo, शून्य *ctx,
					   शून्य *dest_buf, पूर्णाङ्क size)
{
	यदि (size < algo->digest_size)
		प्रत्यागम -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	मुक्त(ctx);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_init_sha256(संरचना hash_algo *algo, शून्य **ctxp)
{
	sha256_context *ctx = दो_स्मृति(परिमाण(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_update_sha256(संरचना hash_algo *algo, शून्य *ctx,
					     ध्रुव शून्य *buf, uint size,
					     पूर्णाङ्क is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_finish_sha256(संरचना hash_algo *algo, शून्य *ctx,
					     शून्य *dest_buf, पूर्णाङ्क size)
{
	यदि (size < algo->digest_size)
		प्रत्यागम -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	मुक्त(ctx);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_init_sha384(संरचना hash_algo *algo, शून्य **ctxp)
{
	sha512_context *ctx = दो_स्मृति(परिमाण(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_update_sha384(संरचना hash_algo *algo, शून्य *ctx,
					     ध्रुव शून्य *buf, uint size,
					     पूर्णाङ्क is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_finish_sha384(संरचना hash_algo *algo, शून्य *ctx,
					     शून्य *dest_buf, पूर्णाङ्क size)
{
	यदि (size < algo->digest_size)
		प्रत्यागम -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	मुक्त(ctx);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_init_sha512(संरचना hash_algo *algo, शून्य **ctxp)
{
	sha512_context *ctx = दो_स्मृति(परिमाण(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_update_sha512(संरचना hash_algo *algo, शून्य *ctx,
					     ध्रुव शून्य *buf, uint size,
					     पूर्णाङ्क is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_finish_sha512(संरचना hash_algo *algo, शून्य *ctx,
					     शून्य *dest_buf, पूर्णाङ्क size)
{
	यदि (size < algo->digest_size)
		प्रत्यागम -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	मुक्त(ctx);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_init_sm3(संरचना hash_algo *algo, शून्य **ctxp)
{
	संरचना sm3_context *ctx = दो_स्मृति(परिमाण(संरचना sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_update_sm3(संरचना hash_algo *algo, शून्य *ctx,
					  ध्रुव शून्य *buf, uint size,
					  पूर्णाङ्क is_last)
{
	sm3_update((संरचना sm3_context *)ctx, buf, size);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_finish_sm3(संरचना hash_algo *algo, शून्य *ctx,
					  शून्य *dest_buf, पूर्णाङ्क size)
{
	यदि (size < algo->digest_size)
		प्रत्यागम -1;

	sm3_final((संरचना sm3_context *)ctx, dest_buf);
	मुक्त(ctx);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_init_crc16_ccitt(संरचना hash_algo *algo,
						शून्य **ctxp)
{
	uint16_t *ctx = दो_स्मृति(परिमाण(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_update_crc16_ccitt(संरचना hash_algo *algo,
						  शून्य *ctx, ध्रुव शून्य *buf,
						  अचिन्हित पूर्णाङ्क size,
						  पूर्णाङ्क is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_finish_crc16_ccitt(संरचना hash_algo *algo,
						  शून्य *ctx, शून्य *dest_buf,
						  पूर्णाङ्क size)
{
	यदि (size < algo->digest_size)
		प्रत्यागम -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	मुक्त(ctx);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_init_crc32(संरचना hash_algo *algo, शून्य **ctxp)
{
	uint32_t *ctx = दो_स्मृति(परिमाण(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_update_crc32(संरचना hash_algo *algo, शून्य *ctx,
					    ध्रुव शून्य *buf, अचिन्हित पूर्णाङ्क size,
					    पूर्णाङ्क is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क __maybe_unused hash_finish_crc32(संरचना hash_algo *algo, शून्य *ctx,
					    शून्य *dest_buf, पूर्णाङ्क size)
{
	यदि (size < algo->digest_size)
		प्रत्यागम -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	मुक्त(ctx);
	प्रत्यागम 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
स्थिर संरचना hash_algo hash_algo[] = {
#यदि CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#यदि CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#यदि CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#अन्यथा
		.hash_func_ws	= sha1_csum_wd,
#endif
#यदि CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#यदि CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#यदि CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#अन्यथा
		.hash_func_ws	= sha256_csum_wd,
#endif
#यदि CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#यदि CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#यदि CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#अन्यथा
		.hash_func_ws	= sha384_csum_wd,
#endif
#यदि CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#यदि CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#यदि CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#अन्यथा
		.hash_func_ws	= sha512_csum_wd,
#endif
#यदि CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#यदि CONFIG_IS_ENABLED(SM3)
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
#यदि CONFIG_IS_ENABLED(CRC16)
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
#यदि CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#यदि CONFIG_IS_ENABLED(CRC32)
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
#यदि CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#अन्यथा
#define multi_hash()	0
#endif

पूर्णाङ्क hash_lookup_algo(ध्रुव वर्ण *algo_name, संरचना hash_algo **algop)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		यदि (!म_भेद(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			प्रत्यागम 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	प्रत्यागम -EPROTONOSUPPORT;
}

पूर्णाङ्क hash_progressive_lookup_algo(ध्रुव वर्ण *algo_name,
				 संरचना hash_algo **algop)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		यदि (!म_भेद(algo_name, hash_algo[i].name)) {
			यदि (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				प्रत्यागम 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	प्रत्यागम -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
पूर्णाङ्क hash_parse_string(ध्रुव वर्ण *algo_name, ध्रुव वर्ण *str, uint8_t *result)
{
	संरचना hash_algo *algo;
	पूर्णाङ्क ret;
	पूर्णाङ्क i;

	ret = hash_lookup_algo(algo_name, &algo);
	यदि (ret)
		प्रत्यागम ret;

	प्रति (i = 0; i < algo->digest_size; i++) {
		वर्ण chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, रिक्त);
	}

	प्रत्यागम 0;
}

पूर्णाङ्क hash_block(ध्रुव वर्ण *algo_name, ध्रुव शून्य *data, अचिन्हित पूर्णाङ्क len,
	       uint8_t *output, पूर्णाङ्क *output_size)
{
	संरचना hash_algo *algo;
	पूर्णाङ्क ret;

	ret = hash_lookup_algo(algo_name, &algo);
	यदि (ret)
		प्रत्यागम ret;

	यदि (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		प्रत्यागम -ENOSPC;
	}
	यदि (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	प्रत्यागम 0;
}

#यदि !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
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
स्थिर शून्य store_result(संरचना hash_algo *algo, ध्रुव uint8_t *sum,
			 ध्रुव वर्ण *dest, पूर्णाङ्क allow_env_vars)
{
	अचिन्हित पूर्णाङ्क i;
	पूर्णाङ्क env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	यदि (allow_env_vars) {
		यदि (*dest == '*')
			dest++;
		अन्यथा
			env_var = 1;
	}

	यदि (env_var) {
		वर्ण str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		वर्ण *str_ptr = str_output;

		प्रति (i = 0; i < algo->digest_size; i++) {
			प्र_लिखो(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} अन्यथा {
		ulong addr;
		शून्य *buf;

		addr = hextoul(dest, रिक्त);
		buf = map_sysmem(addr, algo->digest_size);
		स_नकल(buf, sum, algo->digest_size);
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
स्थिर पूर्णाङ्क parse_verify_sum(संरचना hash_algo *algo, वर्ण *verify_str,
			    uint8_t *vsum, पूर्णाङ्क allow_env_vars)
{
	पूर्णाङ्क env_var = 0;

	/* See comment above in store_result() */
	यदि (allow_env_vars) {
		यदि (*verify_str == '*')
			verify_str++;
		अन्यथा
			env_var = 1;
	}

	यदि (!env_var) {
		ulong addr;
		शून्य *buf;

		addr = hextoul(verify_str, रिक्त);
		buf = map_sysmem(addr, algo->digest_size);
		स_नकल(vsum, buf, algo->digest_size);
	} अन्यथा {
		वर्ण *vsum_str;
		पूर्णाङ्क digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		यदि (म_माप(verify_str) == digits)
			vsum_str = verify_str;
		अन्यथा {
			vsum_str = env_get(verify_str);
			यदि (vsum_str == रिक्त || म_माप(vsum_str) != digits) {
				म_लिखो("Expected %d hex digits in env var\n",
				       digits);
				प्रत्यागम 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	प्रत्यागम 0;
}

स्थिर शून्य hash_show(संरचना hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	पूर्णाङ्क i;

	म_लिखो("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	प्रति (i = 0; i < algo->digest_size; i++)
		म_लिखो("%02x", output[i]);
}

पूर्णाङ्क hash_command(ध्रुव वर्ण *algo_name, पूर्णाङ्क flags, संरचना cmd_tbl *cmdtp,
		 पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[])
{
	ulong addr, len;

	यदि ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		प्रत्यागम CMD_RET_USAGE;

	addr = hextoul(*argv++, रिक्त);
	len = hextoul(*argv++, रिक्त);

	यदि (multi_hash()) {
		संरचना hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		शून्य *buf;

		यदि (hash_lookup_algo(algo_name, &algo)) {
			म_लिखो("Unknown hash algorithm '%s'\n", algo_name);
			प्रत्यागम CMD_RET_USAGE;
		}
		argc -= 2;

		यदि (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			माला_दो("HASH_MAX_DIGEST_SIZE exceeded\n");
			प्रत्यागम 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  परिमाण(uint32_t) * HASH_MAX_DIGEST_SIZE);
		यदि (!output)
			प्रत्यागम CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		यदि (CONFIG_IS_ENABLED(DM_HASH)) {
			गणना HASH_ALGO hash_algo;
			पूर्णाङ्क ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			यदि (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				यदि (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					मुक्त(output);
					प्रत्यागम CMD_RET_FAILURE;
				}
				यदि (!ret)
					गच्छ done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#यदि defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		यदि (flags & HASH_FLAG_VERIFY) {
#अन्यथा
		यदि (0) {
#endif
			यदि (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				म_लिखो("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				मुक्त(output);
				प्रत्यागम 1;
			}
			यदि (स_भेद(output, vsum, algo->digest_size) != 0) {
				पूर्णाङ्क i;

				hash_show(algo, addr, len, output);
				म_लिखो(" != ");
				प्रति (i = 0; i < algo->digest_size; i++)
					म_लिखो("%02x", vsum[i]);
				माला_दो(" ** ERROR **\n");
				मुक्त(output);
				प्रत्यागम 1;
			}
		} अन्यथा {
			hash_show(algo, addr, len, output);
			म_लिखो("\n");

			यदि (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		मुक्त(output);

	/* Horrible code size hack for boards that just want crc32 */
	} अन्यथा {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (ध्रुव uchar *)addr, len, CHUNKSZ_CRC32);

		म_लिखो("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		यदि (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], रिक्त);
			*ptr = crc;
		}
	}

	प्रत्यागम 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
