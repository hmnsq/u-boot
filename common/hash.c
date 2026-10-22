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

#अगर_अघोषित USE_HOSTCC
#समावेश <command.h>
#समावेश <dm.h>
#समावेश <env.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <hw_sha.h>
#समावेश <asm/cache.h>
#समावेश <asm/global_data.h>
#समावेश <asm/io.h>
#समावेश <linux/errno.h>
#समावेश <u-boot/hash.h>
#अन्यथा
#समावेश "mkimage.h"
#समावेश <linux/compiler_attributes.h>
#समावेश <time.h>
#समावेश <linux/kconfig.h>
#पूर्ण_अगर /* !USE_HOSTCC*/

#समावेश <hash.h>
#समावेश <image.h>
#समावेश <u-boot/crc.h>
#समावेश <u-boot/sha1.h>
#समावेश <u-boot/sha256.h>
#समावेश <u-boot/sha512.h>
#समावेश <u-boot/md5.h>
#समावेश <u-boot/sm3.h>

अटल पूर्णांक __maybe_unused hash_init_sha1(काष्ठा hash_algo *algo, व्योम **ctxp)
{
	sha1_context *ctx = दो_स्मृति(माप(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_update_sha1(काष्ठा hash_algo *algo, व्योम *ctx,
					   स्थिर व्योम *buf, अचिन्हित पूर्णांक आकार,
					   पूर्णांक is_last)
{
	sha1_update((sha1_context *)ctx, buf, आकार);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_finish_sha1(काष्ठा hash_algo *algo, व्योम *ctx,
					   व्योम *dest_buf, पूर्णांक आकार)
{
	अगर (आकार < algo->digest_आकार)
		वापस -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	मुक्त(ctx);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_init_sha256(काष्ठा hash_algo *algo, व्योम **ctxp)
{
	sha256_context *ctx = दो_स्मृति(माप(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_update_sha256(काष्ठा hash_algo *algo, व्योम *ctx,
					     स्थिर व्योम *buf, uint आकार,
					     पूर्णांक is_last)
{
	sha256_update((sha256_context *)ctx, buf, आकार);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_finish_sha256(काष्ठा hash_algo *algo, व्योम *ctx,
					     व्योम *dest_buf, पूर्णांक आकार)
{
	अगर (आकार < algo->digest_आकार)
		वापस -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	मुक्त(ctx);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_init_sha384(काष्ठा hash_algo *algo, व्योम **ctxp)
{
	sha512_context *ctx = दो_स्मृति(माप(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_update_sha384(काष्ठा hash_algo *algo, व्योम *ctx,
					     स्थिर व्योम *buf, uint आकार,
					     पूर्णांक is_last)
{
	sha384_update((sha512_context *)ctx, buf, आकार);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_finish_sha384(काष्ठा hash_algo *algo, व्योम *ctx,
					     व्योम *dest_buf, पूर्णांक आकार)
{
	अगर (आकार < algo->digest_आकार)
		वापस -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	मुक्त(ctx);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_init_sha512(काष्ठा hash_algo *algo, व्योम **ctxp)
{
	sha512_context *ctx = दो_स्मृति(माप(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_update_sha512(काष्ठा hash_algo *algo, व्योम *ctx,
					     स्थिर व्योम *buf, uint आकार,
					     पूर्णांक is_last)
{
	sha512_update((sha512_context *)ctx, buf, आकार);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_finish_sha512(काष्ठा hash_algo *algo, व्योम *ctx,
					     व्योम *dest_buf, पूर्णांक आकार)
{
	अगर (आकार < algo->digest_आकार)
		वापस -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	मुक्त(ctx);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_init_sm3(काष्ठा hash_algo *algo, व्योम **ctxp)
{
	काष्ठा sm3_context *ctx = दो_स्मृति(माप(काष्ठा sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_update_sm3(काष्ठा hash_algo *algo, व्योम *ctx,
					  स्थिर व्योम *buf, uint आकार,
					  पूर्णांक is_last)
{
	sm3_update((काष्ठा sm3_context *)ctx, buf, आकार);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_finish_sm3(काष्ठा hash_algo *algo, व्योम *ctx,
					  व्योम *dest_buf, पूर्णांक आकार)
{
	अगर (आकार < algo->digest_आकार)
		वापस -1;

	sm3_final((काष्ठा sm3_context *)ctx, dest_buf);
	मुक्त(ctx);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_init_crc16_ccitt(काष्ठा hash_algo *algo,
						व्योम **ctxp)
{
	uint16_t *ctx = दो_स्मृति(माप(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_update_crc16_ccitt(काष्ठा hash_algo *algo,
						  व्योम *ctx, स्थिर व्योम *buf,
						  अचिन्हित पूर्णांक आकार,
						  पूर्णांक is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, आकार);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_finish_crc16_ccitt(काष्ठा hash_algo *algo,
						  व्योम *ctx, व्योम *dest_buf,
						  पूर्णांक आकार)
{
	अगर (आकार < algo->digest_आकार)
		वापस -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	मुक्त(ctx);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_init_crc32(काष्ठा hash_algo *algo, व्योम **ctxp)
{
	uint32_t *ctx = दो_स्मृति(माप(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_update_crc32(काष्ठा hash_algo *algo, व्योम *ctx,
					    स्थिर व्योम *buf, अचिन्हित पूर्णांक आकार,
					    पूर्णांक is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, आकार);
	वापस 0;
}

अटल पूर्णांक __maybe_unused hash_finish_crc32(काष्ठा hash_algo *algo, व्योम *ctx,
					    व्योम *dest_buf, पूर्णांक आकार)
{
	अगर (आकार < algo->digest_आकार)
		वापस -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	मुक्त(ctx);
	वापस 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
अटल काष्ठा hash_algo hash_algo[] = {
#अगर CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_आकार	= MD5_योग_LEN,
		.chunk_आकार	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_आकार	= SHA1_योग_LEN,
		.chunk_आकार	= CHUNKSZ_SHA1,
#अगर CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#अन्यथा
		.hash_func_ws	= sha1_csum_wd,
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#पूर्ण_अगर
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_आकार	= SHA256_योग_LEN,
		.chunk_आकार	= CHUNKSZ_SHA256,
#अगर CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#अन्यथा
		.hash_func_ws	= sha256_csum_wd,
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#पूर्ण_अगर
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_आकार	= SHA384_योग_LEN,
		.chunk_आकार	= CHUNKSZ_SHA384,
#अगर CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#अन्यथा
		.hash_func_ws	= sha384_csum_wd,
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#पूर्ण_अगर
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_आकार	= SHA512_योग_LEN,
		.chunk_आकार	= CHUNKSZ_SHA512,
#अगर CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#अन्यथा
		.hash_func_ws	= sha512_csum_wd,
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#अन्यथा
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#पूर्ण_अगर
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_आकार	= SM3_DIGEST_आकार,
		.chunk_आकार	= SM3_BLOCK_आकार,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_finish	= hash_finish_sm3,
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_आकार	= 2,
		.chunk_आकार	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_finish	= hash_finish_crc16_ccitt,
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_आकार	= 1,
		.chunk_आकार	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(CRC32)
	{
		.name		= "crc32",
		.digest_आकार	= 4,
		.chunk_आकार	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc32_wd_buf,
		.hash_init	= hash_init_crc32,
		.hash_update	= hash_update_crc32,
		.hash_finish	= hash_finish_crc32,
	},
#पूर्ण_अगर
};

/* Try to minimize code size for boards that don't want much hashing */
#अगर CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1_योग) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5_योग) || CONFIG_IS_ENABLED(SM3)
#घोषणा multi_hash()	1
#अन्यथा
#घोषणा multi_hash()	0
#पूर्ण_अगर

पूर्णांक hash_lookup_algo(स्थिर अक्षर *algo_name, काष्ठा hash_algo **algop)
{
	पूर्णांक i;

	क्रम (i = 0; i < ARRAY_आकार(hash_algo); i++) {
		अगर (!म_भेद(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			वापस 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	वापस -EPROTONOSUPPORT;
}

पूर्णांक hash_progressive_lookup_algo(स्थिर अक्षर *algo_name,
				 काष्ठा hash_algo **algop)
{
	पूर्णांक i;

	क्रम (i = 0; i < ARRAY_आकार(hash_algo); i++) {
		अगर (!म_भेद(algo_name, hash_algo[i].name)) {
			अगर (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				वापस 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	वापस -EPROTONOSUPPORT;
}

#अगर_अघोषित USE_HOSTCC
पूर्णांक hash_parse_string(स्थिर अक्षर *algo_name, स्थिर अक्षर *str, uint8_t *परिणाम)
{
	काष्ठा hash_algo *algo;
	पूर्णांक ret;
	पूर्णांक i;

	ret = hash_lookup_algo(algo_name, &algo);
	अगर (ret)
		वापस ret;

	क्रम (i = 0; i < algo->digest_आकार; i++) {
		अक्षर chr[3];

		strlcpy(chr, &str[i * 2], 3);
		परिणाम[i] = hextoul(chr, शून्य);
	}

	वापस 0;
}

पूर्णांक hash_block(स्थिर अक्षर *algo_name, स्थिर व्योम *तथ्य, अचिन्हित पूर्णांक len,
	       uint8_t *output, पूर्णांक *output_आकार)
{
	काष्ठा hash_algo *algo;
	पूर्णांक ret;

	ret = hash_lookup_algo(algo_name, &algo);
	अगर (ret)
		वापस ret;

	अगर (output_आकार && *output_आकार < algo->digest_आकार) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_आकार, algo->digest_आकार);
		वापस -ENOSPC;
	}
	अगर (output_आकार)
		*output_आकार = algo->digest_आकार;
	algo->hash_func_ws(तथ्य, len, output, algo->chunk_आकार);

	वापस 0;
}

#अगर !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1_योग) || defined(CONFIG_CMD_CRC32)) || \
	defined(CONFIG_CMD_MD5_योग)
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
अटल व्योम store_परिणाम(काष्ठा hash_algo *algo, स्थिर uint8_t *योग,
			 स्थिर अक्षर *dest, पूर्णांक allow_env_vars)
{
	अचिन्हित पूर्णांक i;
	पूर्णांक env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	अगर (allow_env_vars) {
		अगर (*dest == '*')
			dest++;
		अन्यथा
			env_var = 1;
	}

	अगर (env_var) {
		अक्षर str_output[HASH_MAX_DIGEST_आकार * 2 + 1];
		अक्षर *str_ptr = str_output;

		क्रम (i = 0; i < algo->digest_आकार; i++) {
			प्र_लिखो(str_ptr, "%02x", योग[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} अन्यथा {
		ulong addr;
		व्योम *buf;

		addr = hextoul(dest, शून्य);
		buf = map_sysmem(addr, algo->digest_आकार);
		स_नकल(buf, योग, algo->digest_आकार);
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
अटल पूर्णांक parse_verify_योग(काष्ठा hash_algo *algo, अक्षर *verify_str,
			    uint8_t *vsum, पूर्णांक allow_env_vars)
{
	पूर्णांक env_var = 0;

	/* See comment above in store_result() */
	अगर (allow_env_vars) {
		अगर (*verify_str == '*')
			verify_str++;
		अन्यथा
			env_var = 1;
	}

	अगर (!env_var) {
		ulong addr;
		व्योम *buf;

		addr = hextoul(verify_str, शून्य);
		buf = map_sysmem(addr, algo->digest_आकार);
		स_नकल(vsum, buf, algo->digest_आकार);
	} अन्यथा {
		अक्षर *vsum_str;
		पूर्णांक digits = algo->digest_आकार * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		अगर (म_माप(verify_str) == digits)
			vsum_str = verify_str;
		अन्यथा {
			vsum_str = env_get(verify_str);
			अगर (vsum_str == शून्य || म_माप(vsum_str) != digits) {
				म_लिखो("Expected %d hex digits in env var\n",
				       digits);
				वापस 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	वापस 0;
}

अटल व्योम hash_show(काष्ठा hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	पूर्णांक i;

	म_लिखो("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	क्रम (i = 0; i < algo->digest_आकार; i++)
		म_लिखो("%02x", output[i]);
}

पूर्णांक hash_command(स्थिर अक्षर *algo_name, पूर्णांक flags, काष्ठा cmd_tbl *cmdtp,
		 पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिर argv[])
{
	ulong addr, len;

	अगर ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		वापस CMD_RET_USAGE;

	addr = hextoul(*argv++, शून्य);
	len = hextoul(*argv++, शून्य);

	अगर (multi_hash()) {
		काष्ठा hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_आकार];
		व्योम *buf;

		अगर (hash_lookup_algo(algo_name, &algo)) {
			म_लिखो("Unknown hash algorithm '%s'\n", algo_name);
			वापस CMD_RET_USAGE;
		}
		argc -= 2;

		अगर (algo->digest_आकार > HASH_MAX_DIGEST_आकार) {
			माला_दो("HASH_MAX_DIGEST_SIZE exceeded\n");
			वापस 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  माप(uint32_t) * HASH_MAX_DIGEST_आकार);
		अगर (!output)
			वापस CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		अगर (CONFIG_IS_ENABLED(DM_HASH)) {
			क्रमागत HASH_ALGO hash_algo;
			पूर्णांक ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			अगर (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_आकार);
				अगर (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					मुक्त(output);
					वापस CMD_RET_FAILURE;
				}
				अगर (!ret)
					जाओ done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_आकार);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#अगर defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1_योग_VERIFY) || \
	defined(CONFIG_MD5_योग_VERIFY) || defined(CONFIG_HASH_VERIFY)
		अगर (flags & HASH_FLAG_VERIFY) {
#अन्यथा
		अगर (0) {
#पूर्ण_अगर
			अगर (parse_verify_योग(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				म_लिखो("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				मुक्त(output);
				वापस 1;
			}
			अगर (स_भेद(output, vsum, algo->digest_आकार) != 0) {
				पूर्णांक i;

				hash_show(algo, addr, len, output);
				म_लिखो(" != ");
				क्रम (i = 0; i < algo->digest_आकार; i++)
					म_लिखो("%02x", vsum[i]);
				माला_दो(" ** ERROR **\n");
				मुक्त(output);
				वापस 1;
			}
		} अन्यथा {
			hash_show(algo, addr, len, output);
			म_लिखो("\n");

			अगर (argc) {
				store_परिणाम(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		मुक्त(output);

	/* Horrible code size hack for boards that just want crc32 */
	} अन्यथा {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (स्थिर uchar *)addr, len, CHUNKSZ_CRC32);

		म_लिखो("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		अगर (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], शून्य);
			*ptr = crc;
		}
	}

	वापस 0;
}
#पूर्ण_अगर /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#पूर्ण_अगर /* !USE_HOSTCC */
