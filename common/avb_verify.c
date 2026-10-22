// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#अन्तर्भाव <avb_verify.h>
#अन्तर्भाव <blk.h>
#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <part.h>
#अन्तर्भाव <tee.h>
#अन्तर्भाव <tee/optee_ta_avb.h>
#अन्तर्भाव <u-boot/sha256.h>

#सचे CONFIG_IS_ENABLED(AVB_मूलम्_कुञ्चिका_BUILTIN)
स्थिर ध्रुव unsigned वर्ण avb_मूलम्_pub[1032] = {
	0x0, 0x0, 0x10, 0x0, 0x55, 0xd9, 0x4, 0xad, 0xd8, 0x4,
	0xaf, 0xe3, 0xd3, 0x84, 0x6c, 0x7e, 0xd, 0x89, 0x3d, 0xc2,
	0x8c, 0xd3, 0x12, 0x55, 0xe9, 0x62, 0xc9, 0xf1, 0xf, 0x5e,
	0xcc, 0x16, 0x72, 0xab, 0x44, 0x7c, 0x2c, 0x65, 0x4a, 0x94,
	0xb5, 0x16, 0x2b, 0x0, 0xbb, 0x6, 0xef, 0x13, 0x7, 0x53,
	0x4c, 0xf9, 0x64, 0xb9, 0x28, 0x7a, 0x1b, 0x84, 0x98, 0x88,
	0xd8, 0x67, 0xa4, 0x23, 0xf9, 0xa7, 0x4b, 0xdc, 0x4a, 0xf,
	0xf7, 0x3a, 0x18, 0xae, 0x54, 0xa8, 0x15, 0xfe, 0xb0, 0xad,
	0xac, 0x35, 0xda, 0x3b, 0xad, 0x27, 0xbc, 0xaf, 0xe8, 0xd3,
	0x2f, 0x37, 0x34, 0xd6, 0x51, 0x2b, 0x6c, 0x5a, 0x27, 0xd7,
	0x96, 0x6, 0xaf, 0x6b, 0xb8, 0x80, 0xca, 0xfa, 0x30, 0xb4,
	0xb1, 0x85, 0xb3, 0x4d, 0xaa, 0xaa, 0xc3, 0x16, 0x34, 0x1a,
	0xb8, 0xe7, 0xc7, 0xfa, 0xf9, 0x9, 0x77, 0xab, 0x97, 0x93,
	0xeb, 0x44, 0xae, 0xcf, 0x20, 0xbc, 0xf0, 0x80, 0x11, 0xdb,
	0x23, 0xc, 0x47, 0x71, 0xb9, 0x6d, 0xd6, 0x7b, 0x60, 0x47,
	0x87, 0x16, 0x56, 0x93, 0xb7, 0xc2, 0x2a, 0x9a, 0xb0, 0x4c,
	0x1, 0xc, 0x30, 0xd8, 0x93, 0x87, 0xf0, 0xed, 0x6e, 0x8b,
	0xbe, 0x30, 0x5b, 0xf6, 0xa6, 0xaf, 0xdd, 0x80, 0x7c, 0x45,
	0x5e, 0x8f, 0x91, 0x93, 0x5e, 0x44, 0xfe, 0xb8, 0x82, 0x7,
	0xee, 0x79, 0xca, 0xbf, 0x31, 0x73, 0x62, 0x58, 0xe3, 0xcd,
	0xc4, 0xbc, 0xc2, 0x11, 0x1d, 0xa1, 0x4a, 0xbf, 0xfe, 0x27,
	0x7d, 0xa1, 0xf6, 0x35, 0xa3, 0x5e, 0xca, 0xdc, 0x57, 0x2f,
	0x3e, 0xf0, 0xc9, 0x5d, 0x86, 0x6a, 0xf8, 0xaf, 0x66, 0xa7,
	0xed, 0xcd, 0xb8, 0xed, 0xa1, 0x5f, 0xba, 0x9b, 0x85, 0x1a,
	0xd5, 0x9, 0xae, 0x94, 0x4e, 0x3b, 0xcf, 0xcb, 0x5c, 0xc9,
	0x79, 0x80, 0xf7, 0xcc, 0xa6, 0x4a, 0xa8, 0x6a, 0xd8, 0xd3,
	0x31, 0x11, 0xf9, 0xf6, 0x2, 0x63, 0x2a, 0x1a, 0x2d, 0xd1,
	0x1a, 0x66, 0x1b, 0x16, 0x41, 0xbd, 0xbd, 0xf7, 0x4d, 0xc0,
	0x4a, 0xe5, 0x27, 0x49, 0x5f, 0x7f, 0x58, 0xe3, 0x27, 0x2d,
	0xe5, 0xc9, 0x66, 0xe, 0x52, 0x38, 0x16, 0x38, 0xfb, 0x16,
	0xeb, 0x53, 0x3f, 0xe6, 0xfd, 0xe9, 0xa2, 0x5e, 0x25, 0x59,
	0xd8, 0x79, 0x45, 0xff, 0x3, 0x4c, 0x26, 0xa2, 0x0, 0x5a,
	0x8e, 0xc2, 0x51, 0xa1, 0x15, 0xf9, 0x7b, 0xf4, 0x5c, 0x81,
	0x9b, 0x18, 0x47, 0x35, 0xd8, 0x2d, 0x5, 0xe9, 0xad, 0xf,
	0x35, 0x74, 0x15, 0xa3, 0x8e, 0x8b, 0xcc, 0x27, 0xda, 0x7c,
	0x5d, 0xe4, 0xfa, 0x4, 0xd3, 0x5, 0xb, 0xba, 0x3a, 0xb2,
	0x49, 0x45, 0x2f, 0x47, 0xc7, 0xd, 0x41, 0x3f, 0x97, 0x80,
	0x4d, 0x3f, 0xc1, 0xb5, 0xbb, 0x70, 0x5f, 0xa7, 0x37, 0xaf,
	0x48, 0x22, 0x12, 0x45, 0x2e, 0xf5, 0xf, 0x87, 0x92, 0xe2,
	0x84, 0x1, 0xf9, 0x12, 0xf, 0x14, 0x15, 0x24, 0xce, 0x89,
	0x99, 0xee, 0xb9, 0xc4, 0x17, 0x70, 0x70, 0x15, 0xea, 0xbe,
	0xc6, 0x6c, 0x1f, 0x62, 0xb3, 0xf4, 0x2d, 0x16, 0x87, 0xfb,
	0x56, 0x1e, 0x45, 0xab, 0xae, 0x32, 0xe4, 0x5e, 0x91, 0xed,
	0x53, 0x66, 0x5e, 0xbd, 0xed, 0xad, 0xe6, 0x12, 0x39, 0xd,
	0x83, 0xc9, 0xe8, 0x6b, 0x6c, 0x2d, 0xa5, 0xee, 0xc4, 0x5a,
	0x66, 0xae, 0x8c, 0x97, 0xd7, 0xd, 0x6c, 0x49, 0xc7, 0xf5,
	0xc4, 0x92, 0x31, 0x8b, 0x9, 0xee, 0x33, 0xda, 0xa9, 0x37,
	0xb6, 0x49, 0x18, 0xf8, 0xe, 0x60, 0x45, 0xc8, 0x33, 0x91,
	0xef, 0x20, 0x57, 0x10, 0xbe, 0x78, 0x2d, 0x83, 0x26, 0xd6,
	0xca, 0x61, 0xf9, 0x2f, 0xe0, 0xbf, 0x5, 0x30, 0x52, 0x5a,
	0x12, 0x1c, 0x0, 0xa7, 0x5d, 0xcc, 0x7c, 0x2e, 0xc5, 0x95,
	0x8b, 0xa3, 0x3b, 0xf0, 0x43, 0x2e, 0x5e, 0xdd, 0x0, 0xdb,
	0xd, 0xb3, 0x37, 0x99, 0xa9, 0xcd, 0x9c, 0xb7, 0x43, 0xf7,
	0x35, 0x44, 0x21, 0xc2, 0x82, 0x71, 0xab, 0x8d, 0xaa, 0xb4,
	0x41, 0x11, 0xec, 0x1e, 0x8d, 0xfc, 0x14, 0x82, 0x92, 0x4e,
	0x83, 0x6a, 0xa, 0x6b, 0x35, 0x5e, 0x5d, 0xe9, 0x5c, 0xcc,
	0x8c, 0xde, 0x39, 0xd1, 0x4a, 0x5b, 0x5f, 0x63, 0xa9, 0x64,
	0xe0, 0xa, 0xcb, 0xb, 0xb8, 0x5a, 0x7c, 0xc3, 0xb, 0xe6,
	0xbe, 0xfe, 0x8b, 0xf, 0x7d, 0x34, 0x8e, 0x2, 0x66, 0x74,
	0x1, 0x6c, 0xca, 0x76, 0xac, 0x7c, 0x67, 0x8, 0x2f, 0x3f,
	0x1a, 0xa6, 0x2c, 0x60, 0xb3, 0xff, 0xda, 0x8d, 0xb8, 0x12,
	0xc, 0x0, 0x7f, 0xcc, 0x50, 0xa1, 0x5c, 0x64, 0xa1, 0xe2,
	0x5f, 0x32, 0x65, 0xc9, 0x9c, 0xbe, 0xd6, 0xa, 0x13, 0x87,
	0x3c, 0x2a, 0x45, 0x47, 0xc, 0xca, 0x42, 0x82, 0xfa, 0x89,
	0x65, 0xe7, 0x89, 0xb4, 0x8f, 0xf7, 0x1e, 0xe6, 0x23, 0xa5,
	0xd0, 0x59, 0x37, 0x79, 0x92, 0xd7, 0xce, 0x3d, 0xfd, 0xe3,
	0xa1, 0xb, 0xcf, 0x6c, 0x85, 0xa0, 0x65, 0xf3, 0x5c, 0xc6,
	0x4a, 0x63, 0x5f, 0x6e, 0x3a, 0x3a, 0x2a, 0x8b, 0x6a, 0xb6,
	0x2f, 0xbb, 0xf8, 0xb2, 0x4b, 0x62, 0xbc, 0x1a, 0x91, 0x25,
	0x66, 0xe3, 0x69, 0xca, 0x60, 0x49, 0xb, 0xf6, 0x8a, 0xbe,
	0x3e, 0x76, 0x53, 0xc2, 0x7a, 0xa8, 0x4, 0x17, 0x75, 0xf1,
	0xf3, 0x3, 0x62, 0x1b, 0x85, 0xb2, 0xb0, 0xef, 0x80, 0x15,
	0xb6, 0xd4, 0x4e, 0xdf, 0x71, 0xac, 0xdb, 0x2a, 0x4, 0xd4,
	0xb4, 0x21, 0xba, 0x65, 0x56, 0x57, 0xe8, 0xfa, 0x84, 0xa2,
	0x7d, 0x13, 0xe, 0xaf, 0xd7, 0x9a, 0x58, 0x2a, 0xa3, 0x81,
	0x84, 0x8d, 0x9, 0xa0, 0x6a, 0xc1, 0xbb, 0xd9, 0xf5, 0x86,
	0xac, 0xbd, 0x75, 0x61, 0x9, 0xe6, 0x8c, 0x3d, 0x77, 0xb2,
	0xed, 0x30, 0x20, 0xe4, 0x0, 0x1d, 0x97, 0xe8, 0xbf, 0xc7,
	0x0, 0x1b, 0x21, 0xb1, 0x16, 0xe7, 0x41, 0x67, 0x2e, 0xec,
	0x38, 0xbc, 0xe5, 0x1b, 0xb4, 0x6, 0x23, 0x31, 0x71, 0x1c,
	0x49, 0xcd, 0x76, 0x4a, 0x76, 0x36, 0x8d, 0xa3, 0x89, 0x8b,
	0x4a, 0x7a, 0xf4, 0x87, 0xc8, 0x15, 0xf, 0x37, 0x39, 0xf6,
	0x6d, 0x80, 0x19, 0xef, 0x5c, 0xa8, 0x66, 0xce, 0x1b, 0x16,
	0x79, 0x21, 0xdf, 0xd7, 0x31, 0x30, 0xc4, 0x21, 0xdd, 0x34,
	0x5b, 0xd2, 0x1a, 0x2b, 0x3e, 0x5d, 0xf7, 0xea, 0xca, 0x5,
	0x8e, 0xb7, 0xcb, 0x49, 0x2e, 0xa0, 0xe3, 0xf4, 0xa7, 0x48,
	0x19, 0x10, 0x9c, 0x4, 0xa7, 0xf4, 0x28, 0x74, 0xc8, 0x6f,
	0x63, 0x20, 0x2b, 0x46, 0x24, 0x26, 0x19, 0x1d, 0xd1, 0x2c,
	0x31, 0x6d, 0x5a, 0x29, 0xa2, 0x6, 0xa6, 0xb2, 0x41, 0xcc,
	0xa, 0x27, 0x96, 0x9, 0x96, 0xac, 0x47, 0x65, 0x78, 0x68,
	0x51, 0x98, 0xd6, 0xd8, 0xa6, 0x2d, 0xa0, 0xcf, 0xec, 0xe2,
	0x74, 0xf2, 0x82, 0xe3, 0x97, 0xd9, 0x7e, 0xd4, 0xf8, 0xb,
	0x70, 0x43, 0x3d, 0xb1, 0x7b, 0x97, 0x80, 0xd6, 0xcb, 0xd7,
	0x19, 0xbc, 0x63, 0xb, 0xfd, 0x4d, 0x88, 0xfe, 0x67, 0xac,
	0xb8, 0xcc, 0x50, 0xb7, 0x68, 0xb3, 0x5b, 0xd6, 0x1e, 0x25,
	0xfc, 0x5f, 0x3c, 0x8d, 0xb1, 0x33, 0x7c, 0xb3, 0x49, 0x1,
	0x3f, 0x71, 0x55, 0xe, 0x51, 0xba, 0x61, 0x26, 0xfa, 0xea,
	0xe5, 0xb5, 0xe8, 0xaa, 0xcf, 0xcd, 0x96, 0x9f, 0xd6, 0xc1,
	0x5f, 0x53, 0x91, 0xad, 0x5, 0xde, 0x20, 0xe7, 0x51, 0xda,
	0x5b, 0x95, 0x67, 0xed, 0xf4, 0xee, 0x42, 0x65, 0x70, 0x13,
	0xb, 0x70, 0x14, 0x1c, 0xc9, 0xe0, 0x19, 0xca, 0x5f, 0xf5,
	0x1d, 0x70, 0x4b, 0x6c, 0x6, 0x74, 0xec, 0xb5, 0x2e, 0x77,
	0xe1, 0x74, 0xa1, 0xa3, 0x99, 0xa0, 0x85, 0x9e, 0xf1, 0xac,
	0xd8, 0x7e,
};
#endif /* AVB_ROOT_KEY_BUILTIN */

ध्रुव वर्ण *str_avb_io_error(AvbIOResult res)
{
	विकल्प (res) {
	पक्ष AVB_IO_RESULT_OK:
		पटिनिवत्त "Requested operation was successful";
	पक्ष AVB_IO_RESULT_ERROR_IO:
		पटिनिवत्त "Underlying hardware encountered an I/O error";
	पक्ष AVB_IO_RESULT_ERROR_OOM:
		पटिनिवत्त "Unable to allocate memory";
	पक्ष AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION:
		पटिनिवत्त "Requested partition does not exist";
	पक्ष AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION:
		पटिनिवत्त "Bytes requested is outside the range of partition";
	पक्ष AVB_IO_RESULT_ERROR_NO_SUCH_मानम्:
		पटिनिवत्त "Named persistent value does not exist";
	पक्ष AVB_IO_RESULT_ERROR_INVALID_मानम्_SIZE:
		पटिनिवत्त "Named persistent value size is not supported";
	पक्ष AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE:
		पटिनिवत्त "Buffer is too small for the requested operation";
	स्वभाव:
		पटिनिवत्त "Unknown AVB error";
	}
}

ध्रुव वर्ण *str_avb_slot_error(AvbSlotVerifyResult res)
{
	विकल्प (res) {
	पक्ष AVB_SLOT_VERIFY_RESULT_OK:
		पटिनिवत्त "Verification passed successfully";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_OOM:
		पटिनिवत्त "Allocation of memory failed";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_IO:
		पटिनिवत्त "I/O error occurred while trying to load data";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION:
		पटिनिवत्त "Digest didn't match or signature checks failed";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_ROLLBACK_INDEX:
		पटिनिवत्त "Rollback index is less than its stored value";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_कुञ्चिका_REJECTED:
		पटिनिवत्त "Public keys are not accepted";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_METADATA:
		पटिनिवत्त "Metadata is invalid or inconsistent";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_UNSUPPORTED_VERSION:
		पटिनिवत्त "Metadata requires a newer version of libavb";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_ARGUMENT:
		पटिनिवत्त "Invalid arguments are used";
	स्वभाव:
		पटिनिवत्त "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
वर्ण *avb_set_state(AvbOps *ops, गणना avb_boot_state boot_state)
{
	संरचना AvbOpsData *data;
	वर्ण *cmdline = रिक्त;

	सचे (!ops)
		पटिनिवत्त रिक्त;

	data = (संरचना AvbOpsData *)ops->user_data;
	सचे (!data)
		पटिनिवत्त रिक्त;

	data->boot_state = boot_state;
	विकल्प (boot_state) {
	पक्ष AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		भङ्ग;
	पक्ष AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		भङ्ग;
	पक्ष AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		भङ्ग;
	पक्ष AVB_RED:
		भङ्ग;
	}

	पटिनिवत्त cmdline;
}

वर्ण *append_cmd_पङ्क्तिः(वर्ण *cmdline_orig, वर्ण *cmdline_new)
{
	वर्ण *cmd_पङ्क्तिः;

	सचे (!cmdline_new)
		पटिनिवत्त cmdline_orig;

	सचे (cmdline_orig)
		cmd_पङ्क्तिः = cmdline_orig;
	अञ्ञथा
		cmd_पङ्क्तिः = " ";

	cmd_पङ्क्तिः = avb_strdupv(cmd_पङ्क्तिः, " ", cmdline_new, रिक्त);

	पटिनिवत्त cmd_पङ्क्तिः;
}

स्थिर पूर्णाङ्क avb_find_dm_args(वर्ण **args, वर्ण *str)
{
	पूर्णाङ्क i;

	सचे (!str)
		पटिनिवत्त -1;

	अत्थाय (i = 0; i < AVB_MAX_ARGS && args[i]; ++i) {
		सचे (strstr(args[i], str))
			पटिनिवत्त i;
	}

	पटिनिवत्त -1;
}

स्थिर वर्ण *avb_set_enforce_option(ध्रुव वर्ण *cmdline, ध्रुव वर्ण *option)
{
	वर्ण *cmdarg[AVB_MAX_ARGS];
	वर्ण *newargs = रिक्त;
	पूर्णाङ्क i = 0;
	पूर्णाङ्क total_args;

	memset(cmdarg, 0, परिमाण(cmdarg));
	cmdarg[i++] = strtok((वर्ण *)cmdline, " ");

	करोहि {
		cmdarg[i] = strtok(रिक्त, " ");
		सचे (!cmdarg[i])
			भङ्ग;

		सचे (++i >= AVB_MAX_ARGS) {
			printf("%s: Can't handle more then %d args\n",
			       __func__, i);
			पटिनिवत्त रिक्त;
		}
	} याव (true);

	total_args = i;
	i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_LOGGING);
	सचे (i >= 0) {
		cmdarg[i] = (वर्ण *)option;
	} अञ्ञथा {
		i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_RESTART);
		सचे (i < 0) {
			printf("%s: No verity options found\n", __func__);
			पटिनिवत्त रिक्त;
		}

		cmdarg[i] = (वर्ण *)option;
	}

	अत्थाय (i = 0; i <= total_args; i++)
		newargs = append_cmd_पङ्क्तिः(newargs, cmdarg[i]);

	पटिनिवत्त newargs;
}

वर्ण *avb_set_ignore_corruption(ध्रुव वर्ण *cmdline)
{
	वर्ण *newargs = रिक्त;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_LOGGING);
	सचे (newargs)
		newargs = append_cmd_पङ्क्तिः(newargs,
					  "androidboot.veritymode=eio");

	पटिनिवत्त newargs;
}

वर्ण *avb_set_enforce_verity(ध्रुव वर्ण *cmdline)
{
	वर्ण *newargs;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_RESTART);
	सचे (newargs)
		newargs = append_cmd_पङ्क्तिः(newargs,
					  "androidboot.veritymode=enforcing");
	पटिनिवत्त newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
स्थिर unsigned long mmc_read_and_flush(संरचना mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					शून्य *buffer)
{
	unsigned long blks;
	शून्य *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	सचे (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		पटिनिवत्त 0;
	}
	सचे ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	सचे (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		सचे (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;
	} अञ्ञथा {
		tmp_buf = buffer;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	सचे (unaligned)
		memcpy(buffer, tmp_buf, sectors * part->info.blksz);

	पटिनिवत्त blks;
}

स्थिर unsigned long mmc_write(संरचना mmc_part *part, lbaint_t start,
			       lbaint_t sectors, शून्य *buffer)
{
	शून्य *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	सचे (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		पटिनिवत्त 0;
	}
	सचे ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	सचे (unaligned) {
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		सचे (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;

		memcpy(tmp_buf, buffer, sectors * part->info.blksz);
	} अञ्ञथा {
		tmp_buf = buffer;
	}

	पटिनिवत्त blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

स्थिर संरचना mmc_part *get_partition(AvbOps *ops, ध्रुव वर्ण *partition)
{
	पूर्णाङ्क ret;
	u8 dev_अङ्कः;
	पूर्णाङ्क part_अङ्कः = 0;
	संरचना mmc_part *part;
	संरचना blk_desc *mmc_blk;

	part = malloc(परिमाण(संरचना mmc_part));
	सचे (!part)
		पटिनिवत्त रिक्त;

	dev_अङ्कः = get_boot_device(ops);
	part->mmc = find_mmc_device(dev_अङ्कः);
	सचे (!part->mmc) {
		printf("%s: no MMC device at slot %x\n", __func__, dev_अङ्कः);
		गच्छ err;
	}

	ret = mmc_init(part->mmc);
	सचे (ret) {
		printf("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		गच्छ err;
	}

	सचे (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_अङ्कः);
		सचे (ret) {
			printf("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			गच्छ err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	सचे (!mmc_blk) {
		printf("%s: failed to obtain block descriptor\n", __func__);
		गच्छ err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	सचे (ret < 0) {
		printf("%s: can't find partition '%s'\n", __func__, partition);
		गच्छ err;
	}

	part->dev_अङ्कः = dev_अङ्कः;
	part->mmc_blk = mmc_blk;

	पटिनिवत्त part;
err:
	मुक्त(part);
	पटिनिवत्त रिक्त;
}

स्थिर AvbIOResult mmc_byte_io(AvbOps *ops,
			       ध्रुव वर्ण *partition,
			       s64 offset,
			       size_t अङ्कः_bytes,
			       शून्य *buffer,
			       size_t *out_अङ्कः_read,
			       गणना mmc_io_type io_type)
{
	ulong ret;
	संरचना mmc_part *part;
	u64 start_offset, start_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_RESULT_OK;

	सचे (!partition || !buffer || io_type > IO_WRITE)
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;

	part = get_partition(ops, partition);
	सचे (!part)
		पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	सचे (!part->info.blksz) {
		io_ret = AVB_IO_RESULT_ERROR_IO;
		गच्छ out;
	}

	start_offset = calc_offset(part, offset);
	याव (अङ्कः_bytes) {
		start_sector = start_offset / part->info.blksz;
		sectors = अङ्कः_bytes / part->info.blksz;
		/* handle non block-aligned reads */
		सचे (start_offset % part->info.blksz ||
		    अङ्कः_bytes < part->info.blksz) {
			tmp_buf = get_sector_buf();
			सचे (start_offset % part->info.blksz) {
				residue = part->info.blksz -
					(start_offset % part->info.blksz);
				सचे (residue > अङ्कः_bytes)
					residue = अङ्कः_bytes;
			} अञ्ञथा {
				residue = अङ्कः_bytes;
			}

			सचे (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				सचे (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					गच्छ out;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_offset % part->info.blksz);
				memcpy(buffer, (शून्य *)tmp_buf, residue);
			} अञ्ञथा {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				सचे (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					गच्छ out;
				}
				memcpy((शून्य *)tmp_buf +
					start_offset % part->info.blksz,
					buffer, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				सचे (ret != 1) {
					printf("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					गच्छ out;
				}
			}

			io_cnt += residue;
			buffer += residue;
			start_offset += residue;
			अङ्कः_bytes -= residue;
			अनुवर्त;
		}

		सचे (sectors) {
			सचे (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, buffer);
			} अञ्ञथा {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, buffer);
			}

			सचे (!ret) {
				printf("%s: sector read error\n", __func__);
				io_ret = AVB_IO_RESULT_ERROR_IO;
				गच्छ out;
			}

			io_cnt += ret * part->info.blksz;
			buffer += ret * part->info.blksz;
			start_offset += ret * part->info.blksz;
			अङ्कः_bytes -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	सचे (io_type == IO_READ && out_अङ्कः_read)
		*out_अङ्कः_read = io_cnt;

out:
	मुक्त(part);
	पटिनिवत्त io_ret;
}

/**
 * ============================================================================
 * AVB 2.0 operations
 * ============================================================================
 */

/**
 * read_from_partition() - reads @num_bytes from  @offset from partition
 * identified by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition_name: partition name, NUL-terminated UTF-8 string
 * @offset: offset from the beginning of partition
 * @num_bytes: amount of bytes to read
 * @buffer: destination buffer to store data
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if i/o error occurred from the underlying i/o
 *            subsystem
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if there is no partition with
 *      the given name
 */
स्थिर AvbIOResult read_from_partition(AvbOps *ops,
				       ध्रुव वर्ण *partition_name,
				       s64 offset_from_partition,
				       size_t अङ्कः_bytes,
				       शून्य *buffer,
				       size_t *out_अङ्कः_read)
{
	पटिनिवत्त mmc_byte_io(ops, partition_name, offset_from_partition,
			   अङ्कः_bytes, buffer, out_अङ्कः_read, IO_READ);
}

/**
 * write_to_partition() - writes N bytes to a partition identified by a string
 * name
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @partition_name: partition name
 * @offset_from_partition: offset from the beginning of partition
 * @num_bytes: amount of bytes to write
 * @buf: data to write
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if input/output error occurred
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition, specified in
 *            @partition_name was not found
 */
स्थिर AvbIOResult write_to_partition(AvbOps *ops,
				      ध्रुव वर्ण *partition_name,
				      s64 offset_from_partition,
				      size_t अङ्कः_bytes,
				      ध्रुव शून्य *buffer)
{
	पटिनिवत्त mmc_byte_io(ops, partition_name, offset_from_partition,
			   अङ्कः_bytes, (शून्य *)buffer, रिक्त, IO_WRITE);
}

/**
 * validate_vmbeta_public_key() - checks if the given public key used to sign
 * the vbmeta partition is trusted
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @public_key_data: public key for verifying vbmeta partition signature
 * @public_key_length: length of public key
 * @public_key_metadata:
 * @public_key_metadata_length:
 * @out_key_is_trusted:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 */
स्थिर AvbIOResult validate_vbmeta_public_कुञ्चिका(AvbOps *ops,
					      ध्रुव u8 *public_कुञ्चिका_data,
					      size_t public_कुञ्चिका_length,
					      ध्रुव u8
					      *public_कुञ्चिका_metadata,
					      size_t
					      public_कुञ्चिका_metadata_length,
					      bool *out_कुञ्चिका_is_trusted)
{
	u8 कुञ्चिका_digest[SHA256_SUM_LEN];
	u8 trusted_digest[SHA256_SUM_LEN];
	AvbIOResult rc;

	सचे (!public_कुञ्चिका_length || !public_कुञ्चिका_data || !out_कुञ्चिका_is_trusted)
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*out_कुञ्चिका_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(public_कुञ्चिका_data, public_कुञ्चिका_length, कुञ्चिका_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_मूलम्_कुञ्चिका_digest(ops, trusted_digest);
	सचे (rc != AVB_IO_RESULT_OK)
		पटिनिवत्त rc;

	सचे (avb_safe_memcmp(कुञ्चिका_digest, trusted_digest, परिमाण(कुञ्चिका_digest)) == 0)
		*out_कुञ्चिका_is_trusted = true;

	पटिनिवत्त AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
स्थिर पूर्णाङ्क get_open_session(संरचना AvbOpsData *ops_data)
{
	संरचना udevice *tee = रिक्त;

	याव (!ops_data->tee) {
		ध्रुव संरचना tee_optee_ta_uuid uuid = TA_AVB_UUID;
		संरचना tee_open_session_arg arg;
		पूर्णाङ्क rc;

		tee = tee_find_device(tee, रिक्त, रिक्त, रिक्त);
		सचे (!tee)
			पटिनिवत्त -ENODEV;

		memset(&arg, 0, परिमाण(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, रिक्त);
		सचे (rc || arg.ret)
			अनुवर्त;

		ops_data->tee = tee;
		ops_data->session = arg.session;
	}

	पटिनिवत्त 0;
}

स्थिर AvbIOResult invoke_func(संरचना AvbOpsData *ops_data, u32 func,
			       ulong अङ्कः_param, संरचना tee_param *param)
{
	संरचना tee_invoke_arg arg;

	सचे (get_open_session(ops_data))
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;

	memset(&arg, 0, परिमाण(arg));
	arg.func = func;
	arg.session = ops_data->session;

	सचे (tee_invoke_func(ops_data->tee, &arg, अङ्कः_param, param))
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;
	विकल्प (arg.ret) {
	पक्ष TEE_SUCCESS:
		पटिनिवत्त AVB_IO_RESULT_OK;
	पक्ष TEE_ERROR_OUT_OF_MEMORY:
		पटिनिवत्त AVB_IO_RESULT_ERROR_OOM;
	पक्ष TEE_ERROR_STORAGE_NO_SPACE:
		पटिनिवत्त AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	पक्ष TEE_ERROR_ITEM_NOT_FOUND:
		पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_मानम्;
	पक्ष TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_data->tee, ops_data->session);
		ops_data->tee = रिक्त;
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;
	स्वभाव:
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;
	}
}
#endif

/**
 * read_rollback_index() - gets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @out_rollback_index: used to write a retrieved rollback index.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
स्थिर AvbIOResult read_rollback_index(AvbOps *ops,
				       size_t rollback_index_slot,
				       u64 *out_rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	सचे (out_rollback_index)
		*out_rollback_index = 0;

	पटिनिवत्त AVB_IO_RESULT_OK;
#अञ्ञथा
	AvbIOResult rc;
	संरचना tee_param param[2];

	सचे (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_मानम्;

	memset(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_मानम्_निवेशः;
	param[0].u.मानम्.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_मानम्_OUTPUT;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_ROLLBACK_INDEX,
			 ARRAY_SIZE(param), param);
	सचे (rc)
		पटिनिवत्त rc;

	*out_rollback_index = (u64)param[1].u.मानम्.a << 32 |
			      (u32)param[1].u.मानम्.b;
	पटिनिवत्त AVB_IO_RESULT_OK;
#endif
}

/**
 * write_rollback_index() - sets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @rollback_index: rollback index to write.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
स्थिर AvbIOResult write_rollback_index(AvbOps *ops,
					size_t rollback_index_slot,
					u64 rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	पटिनिवत्त AVB_IO_RESULT_OK;
#अञ्ञथा
	संरचना tee_param param[2];

	सचे (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_मानम्;

	memset(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_मानम्_निवेशः;
	param[0].u.मानम्.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_मानम्_निवेशः;
	param[1].u.मानम्.a = (u32)(rollback_index >> 32);
	param[1].u.मानम्.b = (u32)rollback_index;

	पटिनिवत्त invoke_func(ops->user_data, TA_AVB_CMD_WRITE_ROLLBACK_INDEX,
			   ARRAY_SIZE(param), param);
#endif
}

/**
 * read_is_device_unlocked() - gets whether the device is unlocked
 *
 * @ops: contains AVB ops handlers
 * @out_is_unlocked: device unlock state is stored here, true if unlocked,
 *       false otherwise
 *
 * @return:
 *       AVB_IO_RESULT_OK: state is retrieved successfully
 *       AVB_IO_RESULT_ERROR_IO: an error occurred
 */
स्थिर AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*out_is_unlocked = true;

	पटिनिवत्त AVB_IO_RESULT_OK;
#अञ्ञथा
	AvbIOResult rc;
	संरचना tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_मानम्_OUTPUT };

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	सचे (rc)
		पटिनिवत्त rc;
	*out_is_unlocked = !param.u.मानम्.a;
	पटिनिवत्त AVB_IO_RESULT_OK;
#endif
}

/**
 * get_unique_guid_for_partition() - gets the GUID for a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @guid_buf: buf, used to copy in GUID string. Example of value:
 *      527c1c6d-6361-4593-8842-3c78fcd39219
 * @guid_buf_size: @guid_buf buffer size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_IO, if incorrect buffer size (@guid_buf_size) was
 *             provided
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
स्थिर AvbIOResult get_unique_guid_for_partition(AvbOps *ops,
						 ध्रुव वर्ण *partition,
						 वर्ण *guid_buf,
						 size_t guid_buf_size)
{
	संरचना mmc_part *part;
	size_t uuid_size;

	part = get_partition(ops, partition);
	सचे (!part)
		पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	uuid_size = परिमाण(part->info.uuid);
	सचे (uuid_size > guid_buf_size) {
		मुक्त(part);
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;
	}

	memcpy(guid_buf, part->info.uuid, uuid_size);
	guid_buf[uuid_size - 1] = 0;

	मुक्त(part);
	पटिनिवत्त AVB_IO_RESULT_OK;
}

/**
 * get_size_of_partition() - gets the size of a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @out_size_num_bytes: returns the value of a partition size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE, out_size_num_bytes is NULL
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
स्थिर AvbIOResult get_size_of_partition(AvbOps *ops,
					 ध्रुव वर्ण *partition,
					 u64 *out_size_अङ्कः_bytes)
{
	संरचना mmc_part *part;

	सचे (!out_size_अङ्कः_bytes)
		पटिनिवत्त AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	सचे (!part)
		पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	*out_size_अङ्कः_bytes = part->info.blksz * part->info.size;
	मुक्त(part);

	पटिनिवत्त AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
स्थिर AvbIOResult read_persistent_मानम्(AvbOps *ops,
					 ध्रुव वर्ण *name,
					 size_t buffer_size,
					 u8 *out_buffer,
					 size_t *out_अङ्कः_bytes_read)
{
	AvbIOResult rc;
	संरचना tee_shm *shm_name;
	संरचना tee_shm *shm_buf;
	संरचना tee_param param[2];
	संरचना udevice *tee;
	size_t name_size = strlen(name) + 1;

	सचे (get_open_session(ops->user_data))
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;

	tee = ((संरचना AvbOpsData *)ops->user_data)->tee;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	सचे (rc)
		पटिनिवत्त AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, buffer_size,
			   TEE_SHM_ALLOC, &shm_buf);
	सचे (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		गच्छ free_name;
	}

	memcpy(shm_name->addr, name, name_size);

	memset(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेशः;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buffer_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_PERSIST_मानम्,
			 2, param);
	सचे (rc)
		गच्छ out;

	सचे (param[1].u.memref.size > buffer_size) {
		rc = AVB_IO_RESULT_ERROR_NO_SUCH_मानम्;
		गच्छ out;
	}

	*out_अङ्कः_bytes_read = param[1].u.memref.size;

	memcpy(out_buffer, shm_buf->addr, *out_अङ्कः_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	पटिनिवत्त rc;
}

स्थिर AvbIOResult write_persistent_मानम्(AvbOps *ops,
					  ध्रुव वर्ण *name,
					  size_t मानम्_size,
					  ध्रुव u8 *मानम्)
{
	AvbIOResult rc;
	संरचना tee_shm *shm_name;
	संरचना tee_shm *shm_buf;
	संरचना tee_param param[2];
	संरचना udevice *tee;
	size_t name_size = strlen(name) + 1;

	सचे (get_open_session(ops->user_data))
		पटिनिवत्त AVB_IO_RESULT_ERROR_IO;

	tee = ((संरचना AvbOpsData *)ops->user_data)->tee;

	सचे (!मानम्_size)
		पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_मानम्;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	सचे (rc)
		पटिनिवत्त AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, मानम्_size,
			   TEE_SHM_ALLOC, &shm_buf);
	सचे (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		गच्छ free_name;
	}

	memcpy(shm_name->addr, name, name_size);
	memcpy(shm_buf->addr, मानम्, मानम्_size);

	memset(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेशः;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेशः;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = मानम्_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_WRITE_PERSIST_मानम्,
			 2, param);
	सचे (rc)
		गच्छ out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	पटिनिवत्त rc;
}
#endif

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#सचे CONFIG_IS_ENABLED(AVB_मूलम्_कुञ्चिका_BUILTIN)
AvbIOResult avb_read_मूलम्_कुञ्चिका_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_मूलम्_pub, परिमाण(avb_मूलम्_pub), digest,
		       CHUNKSZ_SHA256);

	पटिनिवत्त AVB_IO_RESULT_OK;
}
#नोचेद्यदि CONFIG_IS_ENABLED(AVB_मूलम्_कुञ्चिका_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_मूलम्_कुञ्चिका_digest(AvbOps *ops, uint8_t *digest)
{
	size_t अङ्कः_read = 0;
	AvbIOResult rc;

	rc = read_persistent_मानम्(ops, CONFIG_AVB_मूलम्_कुञ्चिका_TEE_NAME,
				   SHA256_SUM_LEN, digest, &अङ्कः_read);
	सचे (rc != AVB_IO_RESULT_OK)
		पटिनिवत्त rc;

	सचे (अङ्कः_read != SHA256_SUM_LEN)
		पटिनिवत्त AVB_IO_RESULT_ERROR_INVALID_मानम्_SIZE;

	पटिनिवत्त AVB_IO_RESULT_OK;
}
#अञ्ञथा /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_मूलम्_कुञ्चिका_digest(AvbOps *ops, uint8_t *digest)
{
	printf("%s: board root key provider not implemented\n", __func__);

	पटिनिवत्त AVB_IO_RESULT_ERROR_NO_SUCH_मानम्;
}
#endif

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(पूर्णाङ्क boot_device)
{
	संरचना AvbOpsData *ops_data;

	ops_data = avb_calloc(परिमाण(संरचना AvbOpsData));
	सचे (!ops_data)
		पटिनिवत्त रिक्त;

	ops_data->ops.user_data = ops_data;

	ops_data->ops.read_from_partition = read_from_partition;
	ops_data->ops.write_to_partition = write_to_partition;
	ops_data->ops.validate_vbmeta_public_कुञ्चिका = validate_vbmeta_public_कुञ्चिका;
	ops_data->ops.read_rollback_index = read_rollback_index;
	ops_data->ops.write_rollback_index = write_rollback_index;
	ops_data->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_data->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
#ifdef CONFIG_OPTEE_TA_AVB
	ops_data->ops.write_persistent_मानम् = write_persistent_मानम्;
	ops_data->ops.read_persistent_मानम् = read_persistent_मानम्;
#endif
	ops_data->ops.get_size_of_partition = get_size_of_partition;
	ops_data->mmc_dev = boot_device;

	पटिनिवत्त &ops_data->ops;
}

शून्य avb_ops_free(AvbOps *ops)
{
	संरचना AvbOpsData *ops_data;

	सचे (!ops)
		पटिनिवत्त;

	ops_data = ops->user_data;

	सचे (ops_data) {
#ifdef CONFIG_OPTEE_TA_AVB
		सचे (ops_data->tee)
			tee_close_session(ops_data->tee, ops_data->session);
#endif
		avb_free(ops_data);
	}
}
