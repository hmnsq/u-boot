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

#यदि CONFIG_IS_ENABLED(AVB_मूल_कुंजी_BUILTIN)
स्थिर ध्रुव अचिन्हित वर्ण avb_मूल_pub[1032] = {
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
		प्रत्यागम "Requested operation was successful";
	पक्ष AVB_IO_RESULT_ERROR_IO:
		प्रत्यागम "Underlying hardware encountered an I/O error";
	पक्ष AVB_IO_RESULT_ERROR_OOM:
		प्रत्यागम "Unable to allocate memory";
	पक्ष AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION:
		प्रत्यागम "Requested partition does not exist";
	पक्ष AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION:
		प्रत्यागम "Bytes requested is outside the range of partition";
	पक्ष AVB_IO_RESULT_ERROR_NO_SUCH_मूल्य:
		प्रत्यागम "Named persistent value does not exist";
	पक्ष AVB_IO_RESULT_ERROR_INVALID_मूल्य_SIZE:
		प्रत्यागम "Named persistent value size is not supported";
	पक्ष AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE:
		प्रत्यागम "Buffer is too small for the requested operation";
	स्वभाव:
		प्रत्यागम "Unknown AVB error";
	}
}

ध्रुव वर्ण *str_avb_slot_error(AvbSlotVerifyResult res)
{
	विकल्प (res) {
	पक्ष AVB_SLOT_VERIFY_RESULT_OK:
		प्रत्यागम "Verification passed successfully";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_OOM:
		प्रत्यागम "Allocation of memory failed";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_IO:
		प्रत्यागम "I/O error occurred while trying to load data";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION:
		प्रत्यागम "Digest didn't match or signature checks failed";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_ROLLBACK_INDEX:
		प्रत्यागम "Rollback index is less than its stored value";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_कुंजी_REJECTED:
		प्रत्यागम "Public keys are not accepted";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_METADATA:
		प्रत्यागम "Metadata is invalid or inconsistent";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_UNSUPPORTED_VERSION:
		प्रत्यागम "Metadata requires a newer version of libavb";
	पक्ष AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_ARGUMENT:
		प्रत्यागम "Invalid arguments are used";
	स्वभाव:
		प्रत्यागम "Unknown AVB slot verification error";
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

	यदि (!ops)
		प्रत्यागम रिक्त;

	data = (संरचना AvbOpsData *)ops->user_data;
	यदि (!data)
		प्रत्यागम रिक्त;

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

	प्रत्यागम cmdline;
}

वर्ण *append_cmd_पंक्ति(वर्ण *cmdline_orig, वर्ण *cmdline_new)
{
	वर्ण *cmd_पंक्ति;

	यदि (!cmdline_new)
		प्रत्यागम cmdline_orig;

	यदि (cmdline_orig)
		cmd_पंक्ति = cmdline_orig;
	अन्यथा
		cmd_पंक्ति = " ";

	cmd_पंक्ति = avb_strdupv(cmd_पंक्ति, " ", cmdline_new, रिक्त);

	प्रत्यागम cmd_पंक्ति;
}

स्थिर पूर्णाङ्क avb_find_dm_args(वर्ण **args, वर्ण *str)
{
	पूर्णाङ्क i;

	यदि (!str)
		प्रत्यागम -1;

	प्रति (i = 0; i < AVB_MAX_ARGS && args[i]; ++i) {
		यदि (म_माला(args[i], str))
			प्रत्यागम i;
	}

	प्रत्यागम -1;
}

स्थिर वर्ण *avb_set_enforce_option(ध्रुव वर्ण *cmdline, ध्रुव वर्ण *option)
{
	वर्ण *cmdarg[AVB_MAX_ARGS];
	वर्ण *newargs = रिक्त;
	पूर्णाङ्क i = 0;
	पूर्णाङ्क total_args;

	स_रखो(cmdarg, 0, परिमाण(cmdarg));
	cmdarg[i++] = म_मोहर((वर्ण *)cmdline, " ");

	कुरु {
		cmdarg[i] = म_मोहर(रिक्त, " ");
		यदि (!cmdarg[i])
			भङ्ग;

		यदि (++i >= AVB_MAX_ARGS) {
			म_लिखो("%s: Can't handle more then %d args\n",
			       __func__, i);
			प्रत्यागम रिक्त;
		}
	} यावत् (true);

	total_args = i;
	i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_LOGGING);
	यदि (i >= 0) {
		cmdarg[i] = (वर्ण *)option;
	} अन्यथा {
		i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_RESTART);
		यदि (i < 0) {
			म_लिखो("%s: No verity options found\n", __func__);
			प्रत्यागम रिक्त;
		}

		cmdarg[i] = (वर्ण *)option;
	}

	प्रति (i = 0; i <= total_args; i++)
		newargs = append_cmd_पंक्ति(newargs, cmdarg[i]);

	प्रत्यागम newargs;
}

वर्ण *avb_set_ignore_corruption(ध्रुव वर्ण *cmdline)
{
	वर्ण *newargs = रिक्त;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_LOGGING);
	यदि (newargs)
		newargs = append_cmd_पंक्ति(newargs,
					  "androidboot.veritymode=eio");

	प्रत्यागम newargs;
}

वर्ण *avb_set_enforce_verity(ध्रुव वर्ण *cmdline)
{
	वर्ण *newargs;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_RESTART);
	यदि (newargs)
		newargs = append_cmd_पंक्ति(newargs,
					  "androidboot.veritymode=enforcing");
	प्रत्यागम newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
स्थिर अचिन्हित दीर्घ mmc_read_and_flush(संरचना mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					शून्य *buffer)
{
	अचिन्हित दीर्घ blks;
	शून्य *tmp_buf;
	माप_प्रकार buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	यदि (start < part->info.start) {
		म_लिखो("%s: partition start out of bounds\n", __func__);
		प्रत्यागम 0;
	}
	यदि ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		म_लिखो("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	यदि (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		यदि (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;
	} अन्यथा {
		tmp_buf = buffer;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	यदि (unaligned)
		स_नकल(buffer, tmp_buf, sectors * part->info.blksz);

	प्रत्यागम blks;
}

स्थिर अचिन्हित दीर्घ mmc_write(संरचना mmc_part *part, lbaint_t start,
			       lbaint_t sectors, शून्य *buffer)
{
	शून्य *tmp_buf;
	माप_प्रकार buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	यदि (start < part->info.start) {
		म_लिखो("%s: partition start out of bounds\n", __func__);
		प्रत्यागम 0;
	}
	यदि ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		म_लिखो("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	यदि (unaligned) {
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		यदि (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;

		स_नकल(tmp_buf, buffer, sectors * part->info.blksz);
	} अन्यथा {
		tmp_buf = buffer;
	}

	प्रत्यागम blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

स्थिर संरचना mmc_part *get_partition(AvbOps *ops, ध्रुव वर्ण *partition)
{
	पूर्णाङ्क ret;
	u8 dev_अंक;
	पूर्णाङ्क part_अंक = 0;
	संरचना mmc_part *part;
	संरचना blk_desc *mmc_blk;

	part = दो_स्मृति(परिमाण(संरचना mmc_part));
	यदि (!part)
		प्रत्यागम रिक्त;

	dev_अंक = get_boot_device(ops);
	part->mmc = find_mmc_device(dev_अंक);
	यदि (!part->mmc) {
		म_लिखो("%s: no MMC device at slot %x\n", __func__, dev_अंक);
		गच्छ err;
	}

	ret = mmc_init(part->mmc);
	यदि (ret) {
		म_लिखो("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		गच्छ err;
	}

	यदि (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_अंक);
		यदि (ret) {
			म_लिखो("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			गच्छ err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	यदि (!mmc_blk) {
		म_लिखो("%s: failed to obtain block descriptor\n", __func__);
		गच्छ err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	यदि (ret < 0) {
		म_लिखो("%s: can't find partition '%s'\n", __func__, partition);
		गच्छ err;
	}

	part->dev_अंक = dev_अंक;
	part->mmc_blk = mmc_blk;

	प्रत्यागम part;
err:
	मुक्त(part);
	प्रत्यागम रिक्त;
}

स्थिर AvbIOResult mmc_byte_io(AvbOps *ops,
			       ध्रुव वर्ण *partition,
			       s64 offset,
			       माप_प्रकार अंक_bytes,
			       शून्य *buffer,
			       माप_प्रकार *out_अंक_read,
			       गणना mmc_io_type io_type)
{
	ulong ret;
	संरचना mmc_part *part;
	u64 start_offset, start_sector, sectors, residue;
	u8 *tmp_buf;
	माप_प्रकार io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_RESULT_OK;

	यदि (!partition || !buffer || io_type > IO_WRITE)
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;

	part = get_partition(ops, partition);
	यदि (!part)
		प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	यदि (!part->info.blksz) {
		io_ret = AVB_IO_RESULT_ERROR_IO;
		गच्छ out;
	}

	start_offset = calc_offset(part, offset);
	यावत् (अंक_bytes) {
		start_sector = start_offset / part->info.blksz;
		sectors = अंक_bytes / part->info.blksz;
		/* handle non block-aligned reads */
		यदि (start_offset % part->info.blksz ||
		    अंक_bytes < part->info.blksz) {
			tmp_buf = get_sector_buf();
			यदि (start_offset % part->info.blksz) {
				residue = part->info.blksz -
					(start_offset % part->info.blksz);
				यदि (residue > अंक_bytes)
					residue = अंक_bytes;
			} अन्यथा {
				residue = अंक_bytes;
			}

			यदि (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				यदि (ret != 1) {
					म_लिखो("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					गच्छ out;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_offset % part->info.blksz);
				स_नकल(buffer, (शून्य *)tmp_buf, residue);
			} अन्यथा {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				यदि (ret != 1) {
					म_लिखो("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					गच्छ out;
				}
				स_नकल((शून्य *)tmp_buf +
					start_offset % part->info.blksz,
					buffer, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				यदि (ret != 1) {
					म_लिखो("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					गच्छ out;
				}
			}

			io_cnt += residue;
			buffer += residue;
			start_offset += residue;
			अंक_bytes -= residue;
			अनुवर्त;
		}

		यदि (sectors) {
			यदि (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, buffer);
			} अन्यथा {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, buffer);
			}

			यदि (!ret) {
				म_लिखो("%s: sector read error\n", __func__);
				io_ret = AVB_IO_RESULT_ERROR_IO;
				गच्छ out;
			}

			io_cnt += ret * part->info.blksz;
			buffer += ret * part->info.blksz;
			start_offset += ret * part->info.blksz;
			अंक_bytes -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	यदि (io_type == IO_READ && out_अंक_read)
		*out_अंक_read = io_cnt;

out:
	मुक्त(part);
	प्रत्यागम io_ret;
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
				       माप_प्रकार अंक_bytes,
				       शून्य *buffer,
				       माप_प्रकार *out_अंक_read)
{
	प्रत्यागम mmc_byte_io(ops, partition_name, offset_from_partition,
			   अंक_bytes, buffer, out_अंक_read, IO_READ);
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
				      माप_प्रकार अंक_bytes,
				      ध्रुव शून्य *buffer)
{
	प्रत्यागम mmc_byte_io(ops, partition_name, offset_from_partition,
			   अंक_bytes, (शून्य *)buffer, रिक्त, IO_WRITE);
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
स्थिर AvbIOResult validate_vbmeta_public_कुंजी(AvbOps *ops,
					      ध्रुव u8 *public_कुंजी_data,
					      माप_प्रकार public_कुंजी_length,
					      ध्रुव u8
					      *public_कुंजी_metadata,
					      माप_प्रकार
					      public_कुंजी_metadata_length,
					      bool *out_कुंजी_is_trusted)
{
	u8 कुंजी_digest[SHA256_SUM_LEN];
	u8 trusted_digest[SHA256_SUM_LEN];
	AvbIOResult rc;

	यदि (!public_कुंजी_length || !public_कुंजी_data || !out_कुंजी_is_trusted)
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*out_कुंजी_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(public_कुंजी_data, public_कुंजी_length, कुंजी_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_मूल_कुंजी_digest(ops, trusted_digest);
	यदि (rc != AVB_IO_RESULT_OK)
		प्रत्यागम rc;

	यदि (avb_safe_memcmp(कुंजी_digest, trusted_digest, परिमाण(कुंजी_digest)) == 0)
		*out_कुंजी_is_trusted = true;

	प्रत्यागम AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
स्थिर पूर्णाङ्क get_open_session(संरचना AvbOpsData *ops_data)
{
	संरचना udevice *tee = रिक्त;

	यावत् (!ops_data->tee) {
		ध्रुव संरचना tee_optee_ta_uuid uuid = TA_AVB_UUID;
		संरचना tee_open_session_arg arg;
		पूर्णाङ्क rc;

		tee = tee_find_device(tee, रिक्त, रिक्त, रिक्त);
		यदि (!tee)
			प्रत्यागम -ENODEV;

		स_रखो(&arg, 0, परिमाण(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, रिक्त);
		यदि (rc || arg.ret)
			अनुवर्त;

		ops_data->tee = tee;
		ops_data->session = arg.session;
	}

	प्रत्यागम 0;
}

स्थिर AvbIOResult invoke_func(संरचना AvbOpsData *ops_data, u32 func,
			       ulong अंक_param, संरचना tee_param *param)
{
	संरचना tee_invoke_arg arg;

	यदि (get_open_session(ops_data))
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;

	स_रखो(&arg, 0, परिमाण(arg));
	arg.func = func;
	arg.session = ops_data->session;

	यदि (tee_invoke_func(ops_data->tee, &arg, अंक_param, param))
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;
	विकल्प (arg.ret) {
	पक्ष TEE_SUCCESS:
		प्रत्यागम AVB_IO_RESULT_OK;
	पक्ष TEE_ERROR_OUT_OF_MEMORY:
		प्रत्यागम AVB_IO_RESULT_ERROR_OOM;
	पक्ष TEE_ERROR_STORAGE_NO_SPACE:
		प्रत्यागम AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	पक्ष TEE_ERROR_ITEM_NOT_FOUND:
		प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_मूल्य;
	पक्ष TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_data->tee, ops_data->session);
		ops_data->tee = रिक्त;
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;
	स्वभाव:
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;
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
				       माप_प्रकार rollback_index_slot,
				       u64 *out_rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	यदि (out_rollback_index)
		*out_rollback_index = 0;

	प्रत्यागम AVB_IO_RESULT_OK;
#अन्यथा
	AvbIOResult rc;
	संरचना tee_param param[2];

	यदि (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_मूल्य;

	स_रखो(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_मूल्य_निवेश;
	param[0].u.मूल्य.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_मूल्य_OUTPUT;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_ROLLBACK_INDEX,
			 ARRAY_SIZE(param), param);
	यदि (rc)
		प्रत्यागम rc;

	*out_rollback_index = (u64)param[1].u.मूल्य.a << 32 |
			      (u32)param[1].u.मूल्य.b;
	प्रत्यागम AVB_IO_RESULT_OK;
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
					माप_प्रकार rollback_index_slot,
					u64 rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	प्रत्यागम AVB_IO_RESULT_OK;
#अन्यथा
	संरचना tee_param param[2];

	यदि (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_मूल्य;

	स_रखो(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_मूल्य_निवेश;
	param[0].u.मूल्य.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_मूल्य_निवेश;
	param[1].u.मूल्य.a = (u32)(rollback_index >> 32);
	param[1].u.मूल्य.b = (u32)rollback_index;

	प्रत्यागम invoke_func(ops->user_data, TA_AVB_CMD_WRITE_ROLLBACK_INDEX,
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

	प्रत्यागम AVB_IO_RESULT_OK;
#अन्यथा
	AvbIOResult rc;
	संरचना tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_मूल्य_OUTPUT };

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	यदि (rc)
		प्रत्यागम rc;
	*out_is_unlocked = !param.u.मूल्य.a;
	प्रत्यागम AVB_IO_RESULT_OK;
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
						 माप_प्रकार guid_buf_size)
{
	संरचना mmc_part *part;
	माप_प्रकार uuid_size;

	part = get_partition(ops, partition);
	यदि (!part)
		प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	uuid_size = परिमाण(part->info.uuid);
	यदि (uuid_size > guid_buf_size) {
		मुक्त(part);
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;
	}

	स_नकल(guid_buf, part->info.uuid, uuid_size);
	guid_buf[uuid_size - 1] = 0;

	मुक्त(part);
	प्रत्यागम AVB_IO_RESULT_OK;
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
					 u64 *out_size_अंक_bytes)
{
	संरचना mmc_part *part;

	यदि (!out_size_अंक_bytes)
		प्रत्यागम AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	यदि (!part)
		प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	*out_size_अंक_bytes = part->info.blksz * part->info.size;
	मुक्त(part);

	प्रत्यागम AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
स्थिर AvbIOResult read_persistent_मूल्य(AvbOps *ops,
					 ध्रुव वर्ण *name,
					 माप_प्रकार buffer_size,
					 u8 *out_buffer,
					 माप_प्रकार *out_अंक_bytes_read)
{
	AvbIOResult rc;
	संरचना tee_shm *shm_name;
	संरचना tee_shm *shm_buf;
	संरचना tee_param param[2];
	संरचना udevice *tee;
	माप_प्रकार name_size = म_माप(name) + 1;

	यदि (get_open_session(ops->user_data))
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;

	tee = ((संरचना AvbOpsData *)ops->user_data)->tee;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	यदि (rc)
		प्रत्यागम AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, buffer_size,
			   TEE_SHM_ALLOC, &shm_buf);
	यदि (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		गच्छ free_name;
	}

	स_नकल(shm_name->addr, name, name_size);

	स_रखो(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेश;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buffer_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_PERSIST_मूल्य,
			 2, param);
	यदि (rc)
		गच्छ out;

	यदि (param[1].u.memref.size > buffer_size) {
		rc = AVB_IO_RESULT_ERROR_NO_SUCH_मूल्य;
		गच्छ out;
	}

	*out_अंक_bytes_read = param[1].u.memref.size;

	स_नकल(out_buffer, shm_buf->addr, *out_अंक_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	प्रत्यागम rc;
}

स्थिर AvbIOResult write_persistent_मूल्य(AvbOps *ops,
					  ध्रुव वर्ण *name,
					  माप_प्रकार मूल्य_size,
					  ध्रुव u8 *मूल्य)
{
	AvbIOResult rc;
	संरचना tee_shm *shm_name;
	संरचना tee_shm *shm_buf;
	संरचना tee_param param[2];
	संरचना udevice *tee;
	माप_प्रकार name_size = म_माप(name) + 1;

	यदि (get_open_session(ops->user_data))
		प्रत्यागम AVB_IO_RESULT_ERROR_IO;

	tee = ((संरचना AvbOpsData *)ops->user_data)->tee;

	यदि (!मूल्य_size)
		प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_मूल्य;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	यदि (rc)
		प्रत्यागम AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, मूल्य_size,
			   TEE_SHM_ALLOC, &shm_buf);
	यदि (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		गच्छ free_name;
	}

	स_नकल(shm_name->addr, name, name_size);
	स_नकल(shm_buf->addr, मूल्य, मूल्य_size);

	स_रखो(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेश;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेश;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = मूल्य_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_WRITE_PERSIST_मूल्य,
			 2, param);
	यदि (rc)
		गच्छ out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	प्रत्यागम rc;
}
#endif

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#यदि CONFIG_IS_ENABLED(AVB_मूल_कुंजी_BUILTIN)
AvbIOResult avb_read_मूल_कुंजी_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_मूल_pub, परिमाण(avb_मूल_pub), digest,
		       CHUNKSZ_SHA256);

	प्रत्यागम AVB_IO_RESULT_OK;
}
#नोचेद्यदि CONFIG_IS_ENABLED(AVB_मूल_कुंजी_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_मूल_कुंजी_digest(AvbOps *ops, uint8_t *digest)
{
	माप_प्रकार अंक_read = 0;
	AvbIOResult rc;

	rc = read_persistent_मूल्य(ops, CONFIG_AVB_मूल_कुंजी_TEE_NAME,
				   SHA256_SUM_LEN, digest, &अंक_read);
	यदि (rc != AVB_IO_RESULT_OK)
		प्रत्यागम rc;

	यदि (अंक_read != SHA256_SUM_LEN)
		प्रत्यागम AVB_IO_RESULT_ERROR_INVALID_मूल्य_SIZE;

	प्रत्यागम AVB_IO_RESULT_OK;
}
#अन्यथा /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_मूल_कुंजी_digest(AvbOps *ops, uint8_t *digest)
{
	म_लिखो("%s: board root key provider not implemented\n", __func__);

	प्रत्यागम AVB_IO_RESULT_ERROR_NO_SUCH_मूल्य;
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
	यदि (!ops_data)
		प्रत्यागम रिक्त;

	ops_data->ops.user_data = ops_data;

	ops_data->ops.read_from_partition = read_from_partition;
	ops_data->ops.write_to_partition = write_to_partition;
	ops_data->ops.validate_vbmeta_public_कुंजी = validate_vbmeta_public_कुंजी;
	ops_data->ops.read_rollback_index = read_rollback_index;
	ops_data->ops.write_rollback_index = write_rollback_index;
	ops_data->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_data->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
#ifdef CONFIG_OPTEE_TA_AVB
	ops_data->ops.write_persistent_मूल्य = write_persistent_मूल्य;
	ops_data->ops.read_persistent_मूल्य = read_persistent_मूल्य;
#endif
	ops_data->ops.get_size_of_partition = get_size_of_partition;
	ops_data->mmc_dev = boot_device;

	प्रत्यागम &ops_data->ops;
}

शून्य avb_ops_free(AvbOps *ops)
{
	संरचना AvbOpsData *ops_data;

	यदि (!ops)
		प्रत्यागम;

	ops_data = ops->user_data;

	यदि (ops_data) {
#ifdef CONFIG_OPTEE_TA_AVB
		यदि (ops_data->tee)
			tee_close_session(ops_data->tee, ops_data->session);
#endif
		avb_free(ops_data);
	}
}
