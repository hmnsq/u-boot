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

#यदि CONFIG_IS_ENABLED(AVB_मूलम्_कुञ्चिका_BUILTIN)
स्थिर ध्रुव अचिह्नित वर्ण avb_मूलम्_pub[1032] = {
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
#यदि_अन्तः /* AVB_ROOT_KEY_BUILTIN */

ध्रुव वर्ण *str_avb_io_error(AvbIOResult res)
{
	विकल्प (res) {
	पक्ष AVB_IO_परिणाम_OK:
		प्रत्यागम "Requested operation was successful";
	पक्ष AVB_IO_परिणाम_ERROR_IO:
		प्रत्यागम "Underlying hardware encountered an I/O error";
	पक्ष AVB_IO_परिणाम_ERROR_OOM:
		प्रत्यागम "Unable to allocate memory";
	पक्ष AVB_IO_परिणाम_ERROR_NO_SUCH_PARTITION:
		प्रत्यागम "Requested partition does not exist";
	पक्ष AVB_IO_परिणाम_ERROR_RANGE_OUTSIDE_PARTITION:
		प्रत्यागम "Bytes requested is outside the range of partition";
	पक्ष AVB_IO_परिणाम_ERROR_NO_SUCH_मानम्:
		प्रत्यागम "Named persistent value does not exist";
	पक्ष AVB_IO_परिणाम_ERROR_INVALID_मानम्_आकार:
		प्रत्यागम "Named persistent value size is not supported";
	पक्ष AVB_IO_परिणाम_ERROR_INSUFFICIENT_SPACE:
		प्रत्यागम "Buffer is too small for the requested operation";
	स्वभाव:
		प्रत्यागम "Unknown AVB error";
	}
}

ध्रुव वर्ण *str_avb_slot_error(Avb_Slot_Verify_परिणाम res)
{
	विकल्प (res) {
	पक्ष AVB_SLOT_VERIFY_परिणाम_OK:
		प्रत्यागम "Verification passed successfully";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_OOM:
		प्रत्यागम "Allocation of memory failed";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_IO:
		प्रत्यागम "I/O error occurred while trying to load data";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_VERIFICATION:
		प्रत्यागम "Digest didn't match or signature checks failed";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_ROLLBACK_सूचकांक:
		प्रत्यागम "Rollback index is less than its stored value";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_लौकिकम्_कुञ्चिका_REJECTED:
		प्रत्यागम "Public keys are not accepted";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_INVALID_METADATA:
		प्रत्यागम "Metadata is invalid or inconsistent";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_UNSUPPORTED_संस्करणम्:
		प्रत्यागम "Metadata requires a newer version of libavb";
	पक्ष AVB_SLOT_VERIFY_परिणाम_ERROR_INVALID_तर्कः:
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
वर्ण *avb_समुच्चयः_state(AvbOps *ops, गणना avb_boot_state boot_state)
{
	संरचना Avb_Ops_दत्त *दत्त;
	वर्ण *cmdline = रिक्त;

	यदि (!ops)
		प्रत्यागम रिक्त;

	दत्त = (संरचना Avb_Ops_दत्त *)ops->user_दत्त;
	यदि (!दत्त)
		प्रत्यागम रिक्त;

	दत्त->boot_state = boot_state;
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

वर्ण *append_cmd_पङ्क्तिः(वर्ण *cmdline_orig, वर्ण *cmdline_new)
{
	वर्ण *cmd_पङ्क्तिः;

	यदि (!cmdline_new)
		प्रत्यागम cmdline_orig;

	यदि (cmdline_orig)
		cmd_पङ्क्तिः = cmdline_orig;
	अन्यथा
		cmd_पङ्क्तिः = " ";

	cmd_पङ्क्तिः = avb_strdupv(cmd_पङ्क्तिः, " ", cmdline_new, रिक्त);

	प्रत्यागम cmd_पङ्क्तिः;
}

स्थिर पूर्णाङ्क avb_find_dm_तर्काः(वर्ण **तर्काः, वर्ण *str)
{
	पूर्णाङ्क i;

	यदि (!str)
		प्रत्यागम -1;

	प्रति (i = 0; i < AVB_MAX_तर्काः && तर्काः[i]; ++i) {
		यदि (वाक्ये_अन्विष(तर्काः[i], str))
			प्रत्यागम i;
	}

	प्रत्यागम -1;
}

स्थिर वर्ण *avb_समुच्चयः_enforce_option(ध्रुव वर्ण *cmdline, ध्रुव वर्ण *option)
{
	वर्ण *cmdarg[AVB_MAX_तर्काः];
	वर्ण *newargs = रिक्त;
	पूर्णाङ्क i = 0;
	पूर्णाङ्क योग_तर्काः;

	स्मृतिं_पूरय(cmdarg, 0, परिमाण(cmdarg));
	cmdarg[i++] = strtok((वर्ण *)cmdline, " ");

	कुरु {
		cmdarg[i] = strtok(रिक्त, " ");
		यदि (!cmdarg[i])
			भङ्ग;

		यदि (++i >= AVB_MAX_तर्काः) {
			प्रारूप_लिख("%s: Can't handle more then %d args\n",
			       __func__, i);
			प्रत्यागम रिक्त;
		}
	} यावत् (true);

	योग_तर्काः = i;
	i = avb_find_dm_तर्काः(&cmdarg[0], VERITY_सारणी_OPT_LOGGING);
	यदि (i >= 0) {
		cmdarg[i] = (वर्ण *)option;
	} अन्यथा {
		i = avb_find_dm_तर्काः(&cmdarg[0], VERITY_सारणी_OPT_RESTART);
		यदि (i < 0) {
			प्रारूप_लिख("%s: No verity options found\n", __func__);
			प्रत्यागम रिक्त;
		}

		cmdarg[i] = (वर्ण *)option;
	}

	प्रति (i = 0; i <= योग_तर्काः; i++)
		newargs = append_cmd_पङ्क्तिः(newargs, cmdarg[i]);

	प्रत्यागम newargs;
}

वर्ण *avb_समुच्चयः_ignore_corruption(ध्रुव वर्ण *cmdline)
{
	वर्ण *newargs = रिक्त;

	newargs = avb_समुच्चयः_enforce_option(cmdline, VERITY_सारणी_OPT_LOGGING);
	यदि (newargs)
		newargs = append_cmd_पङ्क्तिः(newargs,
					  "androidboot.veritymode=eio");

	प्रत्यागम newargs;
}

वर्ण *avb_समुच्चयः_enforce_verity(ध्रुव वर्ण *cmdline)
{
	वर्ण *newargs;

	newargs = avb_समुच्चयः_enforce_option(cmdline, VERITY_सारणी_OPT_RESTART);
	यदि (newargs)
		newargs = append_cmd_पङ्क्तिः(newargs,
					  "androidboot.veritymode=enforcing");
	प्रत्यागम newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
स्थिर अचिह्नित दीर्घ mmc_read_and_flush(संरचना mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					शून्य *बफर)
{
	अचिह्नित दीर्घ blks;
	शून्य *tmp_buf;
	size_t buf_आकार;
	bool unaligned = is_buf_unaligned(बफर);

	यदि (start < part->info.start) {
		प्रारूप_लिख("%s: partition start out of bounds\n", __func__);
		प्रत्यागम 0;
	}
	यदि ((start + sectors) > (part->info.start + part->info.आकार)) {
		sectors = part->info.start + part->info.आकार - start;
		प्रारूप_लिख("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	यदि (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, बफर);
		tmp_buf = get_sector_buf();
		buf_आकार = get_sector_buf_आकार();
		यदि (sectors > buf_आकार / part->info.blksz)
			sectors = buf_आकार / part->info.blksz;
	} अन्यथा {
		tmp_buf = बफर;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	यदि (unaligned)
		स्मृतिं_प्रतिलिख(बफर, tmp_buf, sectors * part->info.blksz);

	प्रत्यागम blks;
}

स्थिर अचिह्नित दीर्घ mmc_write(संरचना mmc_part *part, lbaint_t start,
			       lbaint_t sectors, शून्य *बफर)
{
	शून्य *tmp_buf;
	size_t buf_आकार;
	bool unaligned = is_buf_unaligned(बफर);

	यदि (start < part->info.start) {
		प्रारूप_लिख("%s: partition start out of bounds\n", __func__);
		प्रत्यागम 0;
	}
	यदि ((start + sectors) > (part->info.start + part->info.आकार)) {
		sectors = part->info.start + part->info.आकार - start;
		प्रारूप_लिख("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	यदि (unaligned) {
		tmp_buf = get_sector_buf();
		buf_आकार = get_sector_buf_आकार();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, बफर);
		यदि (sectors > buf_आकार / part->info.blksz)
			sectors = buf_आकार / part->info.blksz;

		स्मृतिं_प्रतिलिख(tmp_buf, बफर, sectors * part->info.blksz);
	} अन्यथा {
		tmp_buf = बफर;
	}

	प्रत्यागम blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

स्थिर संरचना mmc_part *get_partition(AvbOps *ops, ध्रुव वर्ण *partition)
{
	पूर्णाङ्क ret;
	u8 dev_अङ्कः;
	पूर्णाङ्क part_अङ्कः = 0;
	संरचना mmc_part *part;
	संरचना blk_desc *mmc_blk;

	part = स्मृतिं_देहि(परिमाण(संरचना mmc_part));
	यदि (!part)
		प्रत्यागम रिक्त;

	dev_अङ्कः = get_boot_device(ops);
	part->mmc = find_mmc_device(dev_अङ्कः);
	यदि (!part->mmc) {
		प्रारूप_लिख("%s: no MMC device at slot %x\n", __func__, dev_अङ्कः);
		गच्छ err;
	}

	ret = mmc_init(part->mmc);
	यदि (ret) {
		प्रारूप_लिख("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		गच्छ err;
	}

	यदि (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_अङ्कः);
		यदि (ret) {
			प्रारूप_लिख("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			गच्छ err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	यदि (!mmc_blk) {
		प्रारूप_लिख("%s: failed to obtain block descriptor\n", __func__);
		गच्छ err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	यदि (ret < 0) {
		प्रारूप_लिख("%s: can't find partition '%s'\n", __func__, partition);
		गच्छ err;
	}

	part->dev_अङ्कः = dev_अङ्कः;
	part->mmc_blk = mmc_blk;

	प्रत्यागम part;
err:
	मुक्त(part);
	प्रत्यागम रिक्त;
}

स्थिर AvbIOResult mmc_byte_io(AvbOps *ops,
			       ध्रुव वर्ण *partition,
			       s64 विस्थापनम्,
			       size_t अङ्कः_अष्टकानि,
			       शून्य *बफर,
			       size_t *बहिर्गमनम्_अङ्कः_read,
			       गणना mmc_io_type io_type)
{
	ulong ret;
	संरचना mmc_part *part;
	u64 start_विस्थापनम्, start_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_परिणाम_OK;

	यदि (!partition || !बफर || io_type > IO_WRITE)
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;

	part = get_partition(ops, partition);
	यदि (!part)
		प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_PARTITION;

	यदि (!part->info.blksz) {
		io_ret = AVB_IO_परिणाम_ERROR_IO;
		गच्छ बहिर्गमनम्;
	}

	start_विस्थापनम् = calc_विस्थापनम्(part, विस्थापनम्);
	यावत् (अङ्कः_अष्टकानि) {
		start_sector = start_विस्थापनम् / part->info.blksz;
		sectors = अङ्कः_अष्टकानि / part->info.blksz;
		/* handle non block-aligned reads */
		यदि (start_विस्थापनम् % part->info.blksz ||
		    अङ्कः_अष्टकानि < part->info.blksz) {
			tmp_buf = get_sector_buf();
			यदि (start_विस्थापनम् % part->info.blksz) {
				residue = part->info.blksz -
					(start_विस्थापनम् % part->info.blksz);
				यदि (residue > अङ्कः_अष्टकानि)
					residue = अङ्कः_अष्टकानि;
			} अन्यथा {
				residue = अङ्कः_अष्टकानि;
			}

			यदि (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				यदि (ret != 1) {
					प्रारूप_लिख("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_परिणाम_ERROR_IO;
					गच्छ बहिर्गमनम्;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_विस्थापनम् % part->info.blksz);
				स्मृतिं_प्रतिलिख(बफर, (शून्य *)tmp_buf, residue);
			} अन्यथा {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				यदि (ret != 1) {
					प्रारूप_लिख("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_परिणाम_ERROR_IO;
					गच्छ बहिर्गमनम्;
				}
				स्मृतिं_प्रतिलिख((शून्य *)tmp_buf +
					start_विस्थापनम् % part->info.blksz,
					बफर, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				यदि (ret != 1) {
					प्रारूप_लिख("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_परिणाम_ERROR_IO;
					गच्छ बहिर्गमनम्;
				}
			}

			io_cnt += residue;
			बफर += residue;
			start_विस्थापनम् += residue;
			अङ्कः_अष्टकानि -= residue;
			अनुवर्त;
		}

		यदि (sectors) {
			यदि (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, बफर);
			} अन्यथा {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, बफर);
			}

			यदि (!ret) {
				प्रारूप_लिख("%s: sector read error\n", __func__);
				io_ret = AVB_IO_परिणाम_ERROR_IO;
				गच्छ बहिर्गमनम्;
			}

			io_cnt += ret * part->info.blksz;
			बफर += ret * part->info.blksz;
			start_विस्थापनम् += ret * part->info.blksz;
			अङ्कः_अष्टकानि -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	यदि (io_type == IO_READ && बहिर्गमनम्_अङ्कः_read)
		*बहिर्गमनम्_अङ्कः_read = io_cnt;

बहिर्गमनम्:
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
				       s64 विस्थापनम्_from_partition,
				       size_t अङ्कः_अष्टकानि,
				       शून्य *बफर,
				       size_t *बहिर्गमनम्_अङ्कः_read)
{
	प्रत्यागम mmc_byte_io(ops, partition_name, विस्थापनम्_from_partition,
			   अङ्कः_अष्टकानि, बफर, बहिर्गमनम्_अङ्कः_read, IO_READ);
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
स्थिर AvbIOResult write_गन्तव्यम्_partition(AvbOps *ops,
				      ध्रुव वर्ण *partition_name,
				      s64 विस्थापनम्_from_partition,
				      size_t अङ्कः_अष्टकानि,
				      ध्रुव शून्य *बफर)
{
	प्रत्यागम mmc_byte_io(ops, partition_name, विस्थापनम्_from_partition,
			   अङ्कः_अष्टकानि, (शून्य *)बफर, रिक्त, IO_WRITE);
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
स्थिर AvbIOResult validate_vbmeta_लौकिकम्_कुञ्चिका(AvbOps *ops,
					      ध्रुव u8 *लौकिकम्_कुञ्चिका_दत्त,
					      size_t लौकिकम्_कुञ्चिका_दीर्घता,
					      ध्रुव u8
					      *लौकिकम्_कुञ्चिका_metadata,
					      size_t
					      लौकिकम्_कुञ्चिका_metadata_दीर्घता,
					      bool *बहिर्गमनम्_कुञ्चिका_is_trusted)
{
	u8 कुञ्चिका_digest[SHA256_योग_LEN];
	u8 trusted_digest[SHA256_योग_LEN];
	AvbIOResult rc;

	यदि (!लौकिकम्_कुञ्चिका_दीर्घता || !लौकिकम्_कुञ्चिका_दत्त || !बहिर्गमनम्_कुञ्चिका_is_trusted)
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*बहिर्गमनम्_कुञ्चिका_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(लौकिकम्_कुञ्चिका_दत्त, लौकिकम्_कुञ्चिका_दीर्घता, कुञ्चिका_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_मूलम्_कुञ्चिका_digest(ops, trusted_digest);
	यदि (rc != AVB_IO_परिणाम_OK)
		प्रत्यागम rc;

	यदि (avb_safe_memcmp(कुञ्चिका_digest, trusted_digest, परिमाण(कुञ्चिका_digest)) == 0)
		*बहिर्गमनम्_कुञ्चिका_is_trusted = true;

	प्रत्यागम AVB_IO_परिणाम_OK;
}

#यदि_परिभाषितम् CONFIG_OPTEE_TA_AVB
स्थिर पूर्णाङ्क get_open_session(संरचना Avb_Ops_दत्त *ops_दत्त)
{
	संरचना udevice *tee = रिक्त;

	यावत् (!ops_दत्त->tee) {
		ध्रुव संरचना tee_optee_ta_uuid uuid = TA_AVB_UUID;
		संरचना tee_open_session_arg arg;
		पूर्णाङ्क rc;

		tee = tee_find_device(tee, रिक्त, रिक्त, रिक्त);
		यदि (!tee)
			प्रत्यागम -ENODEV;

		स्मृतिं_पूरय(&arg, 0, परिमाण(arg));
		tee_optee_ta_uuid_गन्तव्यम्_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, रिक्त);
		यदि (rc || arg.ret)
			अनुवर्त;

		ops_दत्त->tee = tee;
		ops_दत्त->session = arg.session;
	}

	प्रत्यागम 0;
}

स्थिर AvbIOResult invoke_func(संरचना Avb_Ops_दत्त *ops_दत्त, u32 func,
			       ulong अङ्कः_param, संरचना tee_param *param)
{
	संरचना tee_invoke_arg arg;

	यदि (get_open_session(ops_दत्त))
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;

	स्मृतिं_पूरय(&arg, 0, परिमाण(arg));
	arg.func = func;
	arg.session = ops_दत्त->session;

	यदि (tee_invoke_func(ops_दत्त->tee, &arg, अङ्कः_param, param))
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;
	विकल्प (arg.ret) {
	पक्ष TEE_SUCCESS:
		प्रत्यागम AVB_IO_परिणाम_OK;
	पक्ष TEE_ERROR_बहिर्गमनम्_सम्बन्धि_स्मृतिः:
		प्रत्यागम AVB_IO_परिणाम_ERROR_OOM;
	पक्ष TEE_ERROR_STORAGE_NO_SPACE:
		प्रत्यागम AVB_IO_परिणाम_ERROR_INSUFFICIENT_SPACE;
	पक्ष TEE_ERROR_ITEM_NOT_प्राप्तम्:
		प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_मानम्;
	पक्ष TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_दत्त->tee, ops_दत्त->session);
		ops_दत्त->tee = रिक्त;
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;
	स्वभाव:
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;
	}
}
#यदि_अन्तः

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
स्थिर AvbIOResult read_rollback_सूचकांक(AvbOps *ops,
				       size_t rollback_सूचकांक_slot,
				       u64 *बहिर्गमनम्_rollback_सूचकांक)
{
#यदि_अपरिभाषितम् CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	यदि (बहिर्गमनम्_rollback_सूचकांक)
		*बहिर्गमनम्_rollback_सूचकांक = 0;

	प्रत्यागम AVB_IO_परिणाम_OK;
#अन्यथा
	AvbIOResult rc;
	संरचना tee_param param[2];

	यदि (rollback_सूचकांक_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_मानम्;

	स्मृतिं_पूरय(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_मानम्_निवेशः;
	param[0].u.मानम्.a = rollback_सूचकांक_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_मानम्_OUTPUT;

	rc = invoke_func(ops->user_दत्त, TA_AVB_CMD_READ_ROLLBACK_सूचकांक,
			 ARRAY_आकार(param), param);
	यदि (rc)
		प्रत्यागम rc;

	*बहिर्गमनम्_rollback_सूचकांक = (u64)param[1].u.मानम्.a << 32 |
			      (u32)param[1].u.मानम्.b;
	प्रत्यागम AVB_IO_परिणाम_OK;
#यदि_अन्तः
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
स्थिर AvbIOResult write_rollback_सूचकांक(AvbOps *ops,
					size_t rollback_सूचकांक_slot,
					u64 rollback_सूचकांक)
{
#यदि_अपरिभाषितम् CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	प्रत्यागम AVB_IO_परिणाम_OK;
#अन्यथा
	संरचना tee_param param[2];

	यदि (rollback_सूचकांक_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_मानम्;

	स्मृतिं_पूरय(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_मानम्_निवेशः;
	param[0].u.मानम्.a = rollback_सूचकांक_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_मानम्_निवेशः;
	param[1].u.मानम्.a = (u32)(rollback_सूचकांक >> 32);
	param[1].u.मानम्.b = (u32)rollback_सूचकांक;

	प्रत्यागम invoke_func(ops->user_दत्त, TA_AVB_CMD_WRITE_ROLLBACK_सूचकांक,
			   ARRAY_आकार(param), param);
#यदि_अन्तः
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
स्थिर AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *बहिर्गमनम्_is_unlocked)
{
#यदि_अपरिभाषितम् CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*बहिर्गमनम्_is_unlocked = true;

	प्रत्यागम AVB_IO_परिणाम_OK;
#अन्यथा
	AvbIOResult rc;
	संरचना tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_मानम्_OUTPUT };

	rc = invoke_func(ops->user_दत्त, TA_AVB_CMD_READ_तालकम्_STATE, 1, &param);
	यदि (rc)
		प्रत्यागम rc;
	*बहिर्गमनम्_is_unlocked = !param.u.मानम्.a;
	प्रत्यागम AVB_IO_परिणाम_OK;
#यदि_अन्तः
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
						 size_t guid_buf_आकार)
{
	संरचना mmc_part *part;
	size_t uuid_आकार;

	part = get_partition(ops, partition);
	यदि (!part)
		प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_PARTITION;

	uuid_आकार = परिमाण(part->info.uuid);
	यदि (uuid_आकार > guid_buf_आकार) {
		मुक्त(part);
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;
	}

	स्मृतिं_प्रतिलिख(guid_buf, part->info.uuid, uuid_आकार);
	guid_buf[uuid_आकार - 1] = 0;

	मुक्त(part);
	प्रत्यागम AVB_IO_परिणाम_OK;
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
स्थिर AvbIOResult get_आकार_सम्बन्धि_partition(AvbOps *ops,
					 ध्रुव वर्ण *partition,
					 u64 *बहिर्गमनम्_आकार_अङ्कः_अष्टकानि)
{
	संरचना mmc_part *part;

	यदि (!बहिर्गमनम्_आकार_अङ्कः_अष्टकानि)
		प्रत्यागम AVB_IO_परिणाम_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	यदि (!part)
		प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_PARTITION;

	*बहिर्गमनम्_आकार_अङ्कः_अष्टकानि = part->info.blksz * part->info.आकार;
	मुक्त(part);

	प्रत्यागम AVB_IO_परिणाम_OK;
}

#यदि_परिभाषितम् CONFIG_OPTEE_TA_AVB
स्थिर AvbIOResult read_persistent_मानम्(AvbOps *ops,
					 ध्रुव वर्ण *name,
					 size_t बफर_आकार,
					 u8 *बहिर्गमनम्_बफर,
					 size_t *बहिर्गमनम्_अङ्कः_अष्टकानि_read)
{
	AvbIOResult rc;
	संरचना tee_shm *shm_name;
	संरचना tee_shm *shm_buf;
	संरचना tee_param param[2];
	संरचना udevice *tee;
	size_t name_आकार = वाक्यदैर्घ्यम्(name) + 1;

	यदि (get_open_session(ops->user_दत्त))
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;

	tee = ((संरचना Avb_Ops_दत्त *)ops->user_दत्त)->tee;

	rc = tee_shm_alloc(tee, name_आकार,
			   TEE_SHM_ALLOC, &shm_name);
	यदि (rc)
		प्रत्यागम AVB_IO_परिणाम_ERROR_OOM;

	rc = tee_shm_alloc(tee, बफर_आकार,
			   TEE_SHM_ALLOC, &shm_buf);
	यदि (rc) {
		rc = AVB_IO_परिणाम_ERROR_OOM;
		गच्छ free_name;
	}

	स्मृतिं_प्रतिलिख(shm_name->addr, name, name_आकार);

	स्मृतिं_पूरय(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेशः;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.आकार = name_आकार;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.आकार = बफर_आकार;

	rc = invoke_func(ops->user_दत्त, TA_AVB_CMD_READ_PERSIST_मानम्,
			 2, param);
	यदि (rc)
		गच्छ बहिर्गमनम्;

	यदि (param[1].u.memref.आकार > बफर_आकार) {
		rc = AVB_IO_परिणाम_ERROR_NO_SUCH_मानम्;
		गच्छ बहिर्गमनम्;
	}

	*बहिर्गमनम्_अङ्कः_अष्टकानि_read = param[1].u.memref.आकार;

	स्मृतिं_प्रतिलिख(बहिर्गमनम्_बफर, shm_buf->addr, *बहिर्गमनम्_अङ्कः_अष्टकानि_read);

बहिर्गमनम्:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	प्रत्यागम rc;
}

स्थिर AvbIOResult write_persistent_मानम्(AvbOps *ops,
					  ध्रुव वर्ण *name,
					  size_t मानम्_आकार,
					  ध्रुव u8 *मानम्)
{
	AvbIOResult rc;
	संरचना tee_shm *shm_name;
	संरचना tee_shm *shm_buf;
	संरचना tee_param param[2];
	संरचना udevice *tee;
	size_t name_आकार = वाक्यदैर्घ्यम्(name) + 1;

	यदि (get_open_session(ops->user_दत्त))
		प्रत्यागम AVB_IO_परिणाम_ERROR_IO;

	tee = ((संरचना Avb_Ops_दत्त *)ops->user_दत्त)->tee;

	यदि (!मानम्_आकार)
		प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_मानम्;

	rc = tee_shm_alloc(tee, name_आकार,
			   TEE_SHM_ALLOC, &shm_name);
	यदि (rc)
		प्रत्यागम AVB_IO_परिणाम_ERROR_OOM;

	rc = tee_shm_alloc(tee, मानम्_आकार,
			   TEE_SHM_ALLOC, &shm_buf);
	यदि (rc) {
		rc = AVB_IO_परिणाम_ERROR_OOM;
		गच्छ free_name;
	}

	स्मृतिं_प्रतिलिख(shm_name->addr, name, name_आकार);
	स्मृतिं_प्रतिलिख(shm_buf->addr, मानम्, मानम्_आकार);

	स्मृतिं_पूरय(param, 0, परिमाण(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेशः;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.आकार = name_आकार;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_निवेशः;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.आकार = मानम्_आकार;

	rc = invoke_func(ops->user_दत्त, TA_AVB_CMD_WRITE_PERSIST_मानम्,
			 2, param);
	यदि (rc)
		गच्छ बहिर्गमनम्;

बहिर्गमनम्:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	प्रत्यागम rc;
}
#यदि_अन्तः

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#यदि CONFIG_IS_ENABLED(AVB_मूलम्_कुञ्चिका_BUILTIN)
AvbIOResult avb_read_मूलम्_कुञ्चिका_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_मूलम्_pub, परिमाण(avb_मूलम्_pub), digest,
		       CHUNKSZ_SHA256);

	प्रत्यागम AVB_IO_परिणाम_OK;
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
				   SHA256_योग_LEN, digest, &अङ्कः_read);
	यदि (rc != AVB_IO_परिणाम_OK)
		प्रत्यागम rc;

	यदि (अङ्कः_read != SHA256_योग_LEN)
		प्रत्यागम AVB_IO_परिणाम_ERROR_INVALID_मानम्_आकार;

	प्रत्यागम AVB_IO_परिणाम_OK;
}
#अन्यथा /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_मूलम्_कुञ्चिका_digest(AvbOps *ops, uint8_t *digest)
{
	प्रारूप_लिख("%s: board root key provider not implemented\n", __func__);

	प्रत्यागम AVB_IO_परिणाम_ERROR_NO_SUCH_मानम्;
}
#यदि_अन्तः

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(पूर्णाङ्क boot_device)
{
	संरचना Avb_Ops_दत्त *ops_दत्त;

	ops_दत्त = avb_calloc(परिमाण(संरचना Avb_Ops_दत्त));
	यदि (!ops_दत्त)
		प्रत्यागम रिक्त;

	ops_दत्त->ops.user_दत्त = ops_दत्त;

	ops_दत्त->ops.read_from_partition = read_from_partition;
	ops_दत्त->ops.write_गन्तव्यम्_partition = write_गन्तव्यम्_partition;
	ops_दत्त->ops.validate_vbmeta_लौकिकम्_कुञ्चिका = validate_vbmeta_लौकिकम्_कुञ्चिका;
	ops_दत्त->ops.read_rollback_सूचकांक = read_rollback_सूचकांक;
	ops_दत्त->ops.write_rollback_सूचकांक = write_rollback_सूचकांक;
	ops_दत्त->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_दत्त->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
#यदि_परिभाषितम् CONFIG_OPTEE_TA_AVB
	ops_दत्त->ops.write_persistent_मानम् = write_persistent_मानम्;
	ops_दत्त->ops.read_persistent_मानम् = read_persistent_मानम्;
#यदि_अन्तः
	ops_दत्त->ops.get_आकार_सम्बन्धि_partition = get_आकार_सम्बन्धि_partition;
	ops_दत्त->mmc_dev = boot_device;

	प्रत्यागम &ops_दत्त->ops;
}

शून्य avb_ops_free(AvbOps *ops)
{
	संरचना Avb_Ops_दत्त *ops_दत्त;

	यदि (!ops)
		प्रत्यागम;

	ops_दत्त = ops->user_दत्त;

	यदि (ops_दत्त) {
#यदि_परिभाषितम् CONFIG_OPTEE_TA_AVB
		यदि (ops_दत्त->tee)
			tee_close_session(ops_दत्त->tee, ops_दत्त->session);
#यदि_अन्तः
		avb_free(ops_दत्त);
	}
}
