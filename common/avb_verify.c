// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#অন্তর্ভাব <avb_verify.h>
#অন্তর্ভাব <blk.h>
#অন্তর্ভাব <cpu_func.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <part.h>
#অন্তর্ভাব <tee.h>
#অন্তর্ভাব <tee/optee_ta_avb.h>
#অন্তর্ভাব <u-boot/sha256.h>

#যদি CONFIG_IS_ENABLED(AVB_মূল_চাবি_BUILTIN)
স্থির ধ্রুবক unsigned অক্ষর avb_মূল_pub[1032] = {
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

ধ্রুবক অক্ষর *str_avb_io_error(AvbIOResult res)
{
	নির্বাচন (res) {
	ক্ষেত্র AVB_IO_ফলাফল_OK:
		ফেরত "Requested operation was successful";
	ক্ষেত্র AVB_IO_ফলাফল_ERROR_IO:
		ফেরত "Underlying hardware encountered an I/O error";
	ক্ষেত্র AVB_IO_ফলাফল_ERROR_OOM:
		ফেরত "Unable to allocate memory";
	ক্ষেত্র AVB_IO_ফলাফল_ERROR_NO_SUCH_PARTITION:
		ফেরত "Requested partition does not exist";
	ক্ষেত্র AVB_IO_ফলাফল_ERROR_RANGE_OUTSIDE_PARTITION:
		ফেরত "Bytes requested is outside the range of partition";
	ক্ষেত্র AVB_IO_ফলাফল_ERROR_NO_SUCH_মান:
		ফেরত "Named persistent value does not exist";
	ক্ষেত্র AVB_IO_ফলাফল_ERROR_INVALID_মান_আকার:
		ফেরত "Named persistent value size is not supported";
	ক্ষেত্র AVB_IO_ফলাফল_ERROR_INSUFFICIENT_SPACE:
		ফেরত "Buffer is too small for the requested operation";
	পূর্বনির্ধারিত:
		ফেরত "Unknown AVB error";
	}
}

ধ্রুবক অক্ষর *str_avb_slot_error(Avb_Slot_Verify_ফলাফল res)
{
	নির্বাচন (res) {
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_OK:
		ফেরত "Verification passed successfully";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_OOM:
		ফেরত "Allocation of memory failed";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_IO:
		ফেরত "I/O error occurred while trying to load data";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_VERIFICATION:
		ফেরত "Digest didn't match or signature checks failed";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_ROLLBACK_সূচক:
		ফেরত "Rollback index is less than its stored value";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_PUBLIC_চাবি_REJECTED:
		ফেরত "Public keys are not accepted";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_INVALID_METADATA:
		ফেরত "Metadata is invalid or inconsistent";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_UNSUPPORTED_VERSION:
		ফেরত "Metadata requires a newer version of libavb";
	ক্ষেত্র AVB_SLOT_VERIFY_ফলাফল_ERROR_INVALID_ARGUMENT:
		ফেরত "Invalid arguments are used";
	পূর্বনির্ধারিত:
		ফেরত "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
অক্ষর *avb_set_state(AvbOps *ops, গণনা avb_boot_state boot_state)
{
	কাঠামো Avb_Ops_তথ্য *তথ্য;
	অক্ষর *cmdline = ফাঁকা;

	যদি (!ops)
		ফেরত ফাঁকা;

	তথ্য = (কাঠামো Avb_Ops_তথ্য *)ops->user_তথ্য;
	যদি (!তথ্য)
		ফেরত ফাঁকা;

	তথ্য->boot_state = boot_state;
	নির্বাচন (boot_state) {
	ক্ষেত্র AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		বিরতি;
	ক্ষেত্র AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		বিরতি;
	ক্ষেত্র AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		বিরতি;
	ক্ষেত্র AVB_RED:
		বিরতি;
	}

	ফেরত cmdline;
}

অক্ষর *append_cmd_সারি(অক্ষর *cmdline_orig, অক্ষর *cmdline_new)
{
	অক্ষর *cmd_সারি;

	যদি (!cmdline_new)
		ফেরত cmdline_orig;

	যদি (cmdline_orig)
		cmd_সারি = cmdline_orig;
	অন্যথা
		cmd_সারি = " ";

	cmd_সারি = avb_strdupv(cmd_সারি, " ", cmdline_new, ফাঁকা);

	ফেরত cmd_সারি;
}

স্থির পূর্ণসংখ্যা avb_find_dm_args(অক্ষর **args, অক্ষর *str)
{
	পূর্ণসংখ্যা i;

	যদি (!str)
		ফেরত -1;

	জন্য (i = 0; i < AVB_MAX_ARGS && args[i]; ++i) {
		যদি (strstr(args[i], str))
			ফেরত i;
	}

	ফেরত -1;
}

স্থির অক্ষর *avb_set_enforce_option(ধ্রুবক অক্ষর *cmdline, ধ্রুবক অক্ষর *option)
{
	অক্ষর *cmdarg[AVB_MAX_ARGS];
	অক্ষর *newargs = ফাঁকা;
	পূর্ণসংখ্যা i = 0;
	পূর্ণসংখ্যা যোগ_args;

	memset(cmdarg, 0, আকার(cmdarg));
	cmdarg[i++] = strtok((অক্ষর *)cmdline, " ");

	করো {
		cmdarg[i] = strtok(ফাঁকা, " ");
		যদি (!cmdarg[i])
			বিরতি;

		যদি (++i >= AVB_MAX_ARGS) {
			printf("%s: Can't handle more then %d args\n",
			       __func__, i);
			ফেরত ফাঁকা;
		}
	} যখন (true);

	যোগ_args = i;
	i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_LOGGING);
	যদি (i >= 0) {
		cmdarg[i] = (অক্ষর *)option;
	} অন্যথা {
		i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_RESTART);
		যদি (i < 0) {
			printf("%s: No verity options found\n", __func__);
			ফেরত ফাঁকা;
		}

		cmdarg[i] = (অক্ষর *)option;
	}

	জন্য (i = 0; i <= যোগ_args; i++)
		newargs = append_cmd_সারি(newargs, cmdarg[i]);

	ফেরত newargs;
}

অক্ষর *avb_set_ignore_corruption(ধ্রুবক অক্ষর *cmdline)
{
	অক্ষর *newargs = ফাঁকা;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_LOGGING);
	যদি (newargs)
		newargs = append_cmd_সারি(newargs,
					  "androidboot.veritymode=eio");

	ফেরত newargs;
}

অক্ষর *avb_set_enforce_verity(ধ্রুবক অক্ষর *cmdline)
{
	অক্ষর *newargs;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_RESTART);
	যদি (newargs)
		newargs = append_cmd_সারি(newargs,
					  "androidboot.veritymode=enforcing");
	ফেরত newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
স্থির unsigned long mmc_read_and_flush(কাঠামো mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					শূন্য *বাফার)
{
	unsigned long blks;
	শূন্য *tmp_buf;
	size_t buf_আকার;
	bool unaligned = is_buf_unaligned(বাফার);

	যদি (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		ফেরত 0;
	}
	যদি ((start + sectors) > (part->info.start + part->info.আকার_2)) {
		sectors = part->info.start + part->info.আকার_2 - start;
		printf("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	যদি (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, বাফার);
		tmp_buf = get_sector_buf();
		buf_আকার = get_sector_buf_আকার();
		যদি (sectors > buf_আকার / part->info.blksz)
			sectors = buf_আকার / part->info.blksz;
	} অন্যথা {
		tmp_buf = বাফার;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	যদি (unaligned)
		memcpy(বাফার, tmp_buf, sectors * part->info.blksz);

	ফেরত blks;
}

স্থির unsigned long mmc_write(কাঠামো mmc_part *part, lbaint_t start,
			       lbaint_t sectors, শূন্য *বাফার)
{
	শূন্য *tmp_buf;
	size_t buf_আকার;
	bool unaligned = is_buf_unaligned(বাফার);

	যদি (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		ফেরত 0;
	}
	যদি ((start + sectors) > (part->info.start + part->info.আকার_2)) {
		sectors = part->info.start + part->info.আকার_2 - start;
		printf("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	যদি (unaligned) {
		tmp_buf = get_sector_buf();
		buf_আকার = get_sector_buf_আকার();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, বাফার);
		যদি (sectors > buf_আকার / part->info.blksz)
			sectors = buf_আকার / part->info.blksz;

		memcpy(tmp_buf, বাফার, sectors * part->info.blksz);
	} অন্যথা {
		tmp_buf = বাফার;
	}

	ফেরত blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

স্থির কাঠামো mmc_part *get_partition(AvbOps *ops, ধ্রুবক অক্ষর *partition)
{
	পূর্ণসংখ্যা ret;
	u8 dev_অঙ্ক;
	পূর্ণসংখ্যা part_অঙ্ক = 0;
	কাঠামো mmc_part *part;
	কাঠামো blk_desc *mmc_blk;

	part = malloc(আকার(কাঠামো mmc_part));
	যদি (!part)
		ফেরত ফাঁকা;

	dev_অঙ্ক = get_boot_device(ops);
	part->mmc = find_mmc_device(dev_অঙ্ক);
	যদি (!part->mmc) {
		printf("%s: no MMC device at slot %x\n", __func__, dev_অঙ্ক);
		যাও err;
	}

	ret = mmc_init(part->mmc);
	যদি (ret) {
		printf("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		যাও err;
	}

	যদি (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_অঙ্ক);
		যদি (ret) {
			printf("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			যাও err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	যদি (!mmc_blk) {
		printf("%s: failed to obtain block descriptor\n", __func__);
		যাও err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	যদি (ret < 0) {
		printf("%s: can't find partition '%s'\n", __func__, partition);
		যাও err;
	}

	part->dev_অঙ্ক = dev_অঙ্ক;
	part->mmc_blk = mmc_blk;

	ফেরত part;
err:
	মুক্ত(part);
	ফেরত ফাঁকা;
}

স্থির AvbIOResult mmc_byte_io(AvbOps *ops,
			       ধ্রুবক অক্ষর *partition,
			       s64 offset,
			       size_t অঙ্ক_bytes,
			       শূন্য *বাফার,
			       size_t *out_অঙ্ক_read,
			       গণনা mmc_io_type io_type)
{
	ulong ret;
	কাঠামো mmc_part *part;
	u64 start_offset, start_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_ফলাফল_OK;

	যদি (!partition || !বাফার || io_type > IO_WRITE)
		ফেরত AVB_IO_ফলাফল_ERROR_IO;

	part = get_partition(ops, partition);
	যদি (!part)
		ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_PARTITION;

	যদি (!part->info.blksz) {
		io_ret = AVB_IO_ফলাফল_ERROR_IO;
		যাও out;
	}

	start_offset = calc_offset(part, offset);
	যখন (অঙ্ক_bytes) {
		start_sector = start_offset / part->info.blksz;
		sectors = অঙ্ক_bytes / part->info.blksz;
		/* handle non block-aligned reads */
		যদি (start_offset % part->info.blksz ||
		    অঙ্ক_bytes < part->info.blksz) {
			tmp_buf = get_sector_buf();
			যদি (start_offset % part->info.blksz) {
				residue = part->info.blksz -
					(start_offset % part->info.blksz);
				যদি (residue > অঙ্ক_bytes)
					residue = অঙ্ক_bytes;
			} অন্যথা {
				residue = অঙ্ক_bytes;
			}

			যদি (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				যদি (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_ফলাফল_ERROR_IO;
					যাও out;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_offset % part->info.blksz);
				memcpy(বাফার, (শূন্য *)tmp_buf, residue);
			} অন্যথা {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				যদি (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_ফলাফল_ERROR_IO;
					যাও out;
				}
				memcpy((শূন্য *)tmp_buf +
					start_offset % part->info.blksz,
					বাফার, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				যদি (ret != 1) {
					printf("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_ফলাফল_ERROR_IO;
					যাও out;
				}
			}

			io_cnt += residue;
			বাফার += residue;
			start_offset += residue;
			অঙ্ক_bytes -= residue;
			চালু;
		}

		যদি (sectors) {
			যদি (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, বাফার);
			} অন্যথা {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, বাফার);
			}

			যদি (!ret) {
				printf("%s: sector read error\n", __func__);
				io_ret = AVB_IO_ফলাফল_ERROR_IO;
				যাও out;
			}

			io_cnt += ret * part->info.blksz;
			বাফার += ret * part->info.blksz;
			start_offset += ret * part->info.blksz;
			অঙ্ক_bytes -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	যদি (io_type == IO_READ && out_অঙ্ক_read)
		*out_অঙ্ক_read = io_cnt;

out:
	মুক্ত(part);
	ফেরত io_ret;
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
স্থির AvbIOResult read_from_partition(AvbOps *ops,
				       ধ্রুবক অক্ষর *partition_name,
				       s64 offset_from_partition,
				       size_t অঙ্ক_bytes,
				       শূন্য *বাফার,
				       size_t *out_অঙ্ক_read)
{
	ফেরত mmc_byte_io(ops, partition_name, offset_from_partition,
			   অঙ্ক_bytes, বাফার, out_অঙ্ক_read, IO_READ);
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
স্থির AvbIOResult write_to_partition(AvbOps *ops,
				      ধ্রুবক অক্ষর *partition_name,
				      s64 offset_from_partition,
				      size_t অঙ্ক_bytes,
				      ধ্রুবক শূন্য *বাফার)
{
	ফেরত mmc_byte_io(ops, partition_name, offset_from_partition,
			   অঙ্ক_bytes, (শূন্য *)বাফার, ফাঁকা, IO_WRITE);
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
স্থির AvbIOResult validate_vbmeta_public_চাবি(AvbOps *ops,
					      ধ্রুবক u8 *public_চাবি_তথ্য,
					      size_t public_চাবি_দৈর্ঘ্য,
					      ধ্রুবক u8
					      *public_চাবি_metadata,
					      size_t
					      public_চাবি_metadata_দৈর্ঘ্য,
					      bool *out_চাবি_is_trusted)
{
	u8 চাবি_digest[SHA256_যোগ_LEN];
	u8 trusted_digest[SHA256_যোগ_LEN];
	AvbIOResult rc;

	যদি (!public_চাবি_দৈর্ঘ্য || !public_চাবি_তথ্য || !out_চাবি_is_trusted)
		ফেরত AVB_IO_ফলাফল_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*out_চাবি_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(public_চাবি_তথ্য, public_চাবি_দৈর্ঘ্য, চাবি_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_মূল_চাবি_digest(ops, trusted_digest);
	যদি (rc != AVB_IO_ফলাফল_OK)
		ফেরত rc;

	যদি (avb_safe_memcmp(চাবি_digest, trusted_digest, আকার(চাবি_digest)) == 0)
		*out_চাবি_is_trusted = true;

	ফেরত AVB_IO_ফলাফল_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
স্থির পূর্ণসংখ্যা get_open_session(কাঠামো Avb_Ops_তথ্য *ops_তথ্য)
{
	কাঠামো udevice *tee = ফাঁকা;

	যখন (!ops_তথ্য->tee) {
		ধ্রুবক কাঠামো tee_optee_ta_uuid uuid = TA_AVB_UUID;
		কাঠামো tee_open_session_arg arg;
		পূর্ণসংখ্যা rc;

		tee = tee_find_device(tee, ফাঁকা, ফাঁকা, ফাঁকা);
		যদি (!tee)
			ফেরত -ENODEV;

		memset(&arg, 0, আকার(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, ফাঁকা);
		যদি (rc || arg.ret)
			চালু;

		ops_তথ্য->tee = tee;
		ops_তথ্য->session = arg.session;
	}

	ফেরত 0;
}

স্থির AvbIOResult invoke_func(কাঠামো Avb_Ops_তথ্য *ops_তথ্য, u32 func,
			       ulong অঙ্ক_param, কাঠামো tee_param *param)
{
	কাঠামো tee_invoke_arg arg;

	যদি (get_open_session(ops_তথ্য))
		ফেরত AVB_IO_ফলাফল_ERROR_IO;

	memset(&arg, 0, আকার(arg));
	arg.func = func;
	arg.session = ops_তথ্য->session;

	যদি (tee_invoke_func(ops_তথ্য->tee, &arg, অঙ্ক_param, param))
		ফেরত AVB_IO_ফলাফল_ERROR_IO;
	নির্বাচন (arg.ret) {
	ক্ষেত্র TEE_SUCCESS:
		ফেরত AVB_IO_ফলাফল_OK;
	ক্ষেত্র TEE_ERROR_OUT_OF_MEMORY:
		ফেরত AVB_IO_ফলাফল_ERROR_OOM;
	ক্ষেত্র TEE_ERROR_STORAGE_NO_SPACE:
		ফেরত AVB_IO_ফলাফল_ERROR_INSUFFICIENT_SPACE;
	ক্ষেত্র TEE_ERROR_ITEM_NOT_FOUND:
		ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_মান;
	ক্ষেত্র TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_তথ্য->tee, ops_তথ্য->session);
		ops_তথ্য->tee = ফাঁকা;
		ফেরত AVB_IO_ফলাফল_ERROR_IO;
	পূর্বনির্ধারিত:
		ফেরত AVB_IO_ফলাফল_ERROR_IO;
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
স্থির AvbIOResult read_rollback_সূচক(AvbOps *ops,
				       size_t rollback_সূচক_slot,
				       u64 *out_rollback_সূচক)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	যদি (out_rollback_সূচক)
		*out_rollback_সূচক = 0;

	ফেরত AVB_IO_ফলাফল_OK;
#অন্যথা
	AvbIOResult rc;
	কাঠামো tee_param param[2];

	যদি (rollback_সূচক_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_মান;

	memset(param, 0, আকার(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_মান_প্রবেশ;
	param[0].u.মান.a = rollback_সূচক_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_মান_OUTPUT;

	rc = invoke_func(ops->user_তথ্য, TA_AVB_CMD_READ_ROLLBACK_সূচক,
			 ARRAY_আকার(param), param);
	যদি (rc)
		ফেরত rc;

	*out_rollback_সূচক = (u64)param[1].u.মান.a << 32 |
			      (u32)param[1].u.মান.b;
	ফেরত AVB_IO_ফলাফল_OK;
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
স্থির AvbIOResult write_rollback_সূচক(AvbOps *ops,
					size_t rollback_সূচক_slot,
					u64 rollback_সূচক)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ফেরত AVB_IO_ফলাফল_OK;
#অন্যথা
	কাঠামো tee_param param[2];

	যদি (rollback_সূচক_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_মান;

	memset(param, 0, আকার(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_মান_প্রবেশ;
	param[0].u.মান.a = rollback_সূচক_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_মান_প্রবেশ;
	param[1].u.মান.a = (u32)(rollback_সূচক >> 32);
	param[1].u.মান.b = (u32)rollback_সূচক;

	ফেরত invoke_func(ops->user_তথ্য, TA_AVB_CMD_WRITE_ROLLBACK_সূচক,
			   ARRAY_আকার(param), param);
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
স্থির AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*out_is_unlocked = true;

	ফেরত AVB_IO_ফলাফল_OK;
#অন্যথা
	AvbIOResult rc;
	কাঠামো tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_মান_OUTPUT };

	rc = invoke_func(ops->user_তথ্য, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	যদি (rc)
		ফেরত rc;
	*out_is_unlocked = !param.u.মান.a;
	ফেরত AVB_IO_ফলাফল_OK;
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
স্থির AvbIOResult get_unique_guid_for_partition(AvbOps *ops,
						 ধ্রুবক অক্ষর *partition,
						 অক্ষর *guid_buf,
						 size_t guid_buf_আকার)
{
	কাঠামো mmc_part *part;
	size_t uuid_আকার;

	part = get_partition(ops, partition);
	যদি (!part)
		ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_PARTITION;

	uuid_আকার = আকার(part->info.uuid);
	যদি (uuid_আকার > guid_buf_আকার) {
		মুক্ত(part);
		ফেরত AVB_IO_ফলাফল_ERROR_IO;
	}

	memcpy(guid_buf, part->info.uuid, uuid_আকার);
	guid_buf[uuid_আকার - 1] = 0;

	মুক্ত(part);
	ফেরত AVB_IO_ফলাফল_OK;
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
স্থির AvbIOResult get_আকার_of_partition(AvbOps *ops,
					 ধ্রুবক অক্ষর *partition,
					 u64 *out_আকার_অঙ্ক_bytes)
{
	কাঠামো mmc_part *part;

	যদি (!out_আকার_অঙ্ক_bytes)
		ফেরত AVB_IO_ফলাফল_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	যদি (!part)
		ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_PARTITION;

	*out_আকার_অঙ্ক_bytes = part->info.blksz * part->info.আকার_2;
	মুক্ত(part);

	ফেরত AVB_IO_ফলাফল_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
স্থির AvbIOResult read_persistent_মান(AvbOps *ops,
					 ধ্রুবক অক্ষর *name,
					 size_t বাফার_আকার,
					 u8 *out_বাফার,
					 size_t *out_অঙ্ক_bytes_read)
{
	AvbIOResult rc;
	কাঠামো tee_shm *shm_name;
	কাঠামো tee_shm *shm_buf;
	কাঠামো tee_param param[2];
	কাঠামো udevice *tee;
	size_t name_আকার = strlen(name) + 1;

	যদি (get_open_session(ops->user_তথ্য))
		ফেরত AVB_IO_ফলাফল_ERROR_IO;

	tee = ((কাঠামো Avb_Ops_তথ্য *)ops->user_তথ্য)->tee;

	rc = tee_shm_alloc(tee, name_আকার,
			   TEE_SHM_ALLOC, &shm_name);
	যদি (rc)
		ফেরত AVB_IO_ফলাফল_ERROR_OOM;

	rc = tee_shm_alloc(tee, বাফার_আকার,
			   TEE_SHM_ALLOC, &shm_buf);
	যদি (rc) {
		rc = AVB_IO_ফলাফল_ERROR_OOM;
		যাও free_name;
	}

	memcpy(shm_name->addr, name, name_আকার);

	memset(param, 0, আকার(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_প্রবেশ;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.আকার_2 = name_আকার;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.আকার_2 = বাফার_আকার;

	rc = invoke_func(ops->user_তথ্য, TA_AVB_CMD_READ_PERSIST_মান,
			 2, param);
	যদি (rc)
		যাও out;

	যদি (param[1].u.memref.আকার_2 > বাফার_আকার) {
		rc = AVB_IO_ফলাফল_ERROR_NO_SUCH_মান;
		যাও out;
	}

	*out_অঙ্ক_bytes_read = param[1].u.memref.আকার_2;

	memcpy(out_বাফার, shm_buf->addr, *out_অঙ্ক_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ফেরত rc;
}

স্থির AvbIOResult write_persistent_মান(AvbOps *ops,
					  ধ্রুবক অক্ষর *name,
					  size_t মান_আকার,
					  ধ্রুবক u8 *মান)
{
	AvbIOResult rc;
	কাঠামো tee_shm *shm_name;
	কাঠামো tee_shm *shm_buf;
	কাঠামো tee_param param[2];
	কাঠামো udevice *tee;
	size_t name_আকার = strlen(name) + 1;

	যদি (get_open_session(ops->user_তথ্য))
		ফেরত AVB_IO_ফলাফল_ERROR_IO;

	tee = ((কাঠামো Avb_Ops_তথ্য *)ops->user_তথ্য)->tee;

	যদি (!মান_আকার)
		ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_মান;

	rc = tee_shm_alloc(tee, name_আকার,
			   TEE_SHM_ALLOC, &shm_name);
	যদি (rc)
		ফেরত AVB_IO_ফলাফল_ERROR_OOM;

	rc = tee_shm_alloc(tee, মান_আকার,
			   TEE_SHM_ALLOC, &shm_buf);
	যদি (rc) {
		rc = AVB_IO_ফলাফল_ERROR_OOM;
		যাও free_name;
	}

	memcpy(shm_name->addr, name, name_আকার);
	memcpy(shm_buf->addr, মান, মান_আকার);

	memset(param, 0, আকার(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_প্রবেশ;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.আকার_2 = name_আকার;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_প্রবেশ;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.আকার_2 = মান_আকার;

	rc = invoke_func(ops->user_তথ্য, TA_AVB_CMD_WRITE_PERSIST_মান,
			 2, param);
	যদি (rc)
		যাও out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ফেরত rc;
}
#endif

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#যদি CONFIG_IS_ENABLED(AVB_মূল_চাবি_BUILTIN)
AvbIOResult avb_read_মূল_চাবি_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_মূল_pub, আকার(avb_মূল_pub), digest,
		       CHUNKSZ_SHA256);

	ফেরত AVB_IO_ফলাফল_OK;
}
#নতুবা_যদি CONFIG_IS_ENABLED(AVB_মূল_চাবি_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_মূল_চাবি_digest(AvbOps *ops, uint8_t *digest)
{
	size_t অঙ্ক_read = 0;
	AvbIOResult rc;

	rc = read_persistent_মান(ops, CONFIG_AVB_মূল_চাবি_TEE_NAME,
				   SHA256_যোগ_LEN, digest, &অঙ্ক_read);
	যদি (rc != AVB_IO_ফলাফল_OK)
		ফেরত rc;

	যদি (অঙ্ক_read != SHA256_যোগ_LEN)
		ফেরত AVB_IO_ফলাফল_ERROR_INVALID_মান_আকার;

	ফেরত AVB_IO_ফলাফল_OK;
}
#অন্যথা /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_মূল_চাবি_digest(AvbOps *ops, uint8_t *digest)
{
	printf("%s: board root key provider not implemented\n", __func__);

	ফেরত AVB_IO_ফলাফল_ERROR_NO_SUCH_মান;
}
#endif

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(পূর্ণসংখ্যা boot_device)
{
	কাঠামো Avb_Ops_তথ্য *ops_তথ্য;

	ops_তথ্য = avb_calloc(আকার(কাঠামো Avb_Ops_তথ্য));
	যদি (!ops_তথ্য)
		ফেরত ফাঁকা;

	ops_তথ্য->ops.user_তথ্য = ops_তথ্য;

	ops_তথ্য->ops.read_from_partition = read_from_partition;
	ops_তথ্য->ops.write_to_partition = write_to_partition;
	ops_তথ্য->ops.validate_vbmeta_public_চাবি = validate_vbmeta_public_চাবি;
	ops_তথ্য->ops.read_rollback_সূচক = read_rollback_সূচক;
	ops_তথ্য->ops.write_rollback_সূচক = write_rollback_সূচক;
	ops_তথ্য->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_তথ্য->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
#ifdef CONFIG_OPTEE_TA_AVB
	ops_তথ্য->ops.write_persistent_মান = write_persistent_মান;
	ops_তথ্য->ops.read_persistent_মান = read_persistent_মান;
#endif
	ops_তথ্য->ops.get_আকার_of_partition = get_আকার_of_partition;
	ops_তথ্য->mmc_dev = boot_device;

	ফেরত &ops_তথ্য->ops;
}

শূন্য avb_ops_free(AvbOps *ops)
{
	কাঠামো Avb_Ops_তথ্য *ops_তথ্য;

	যদি (!ops)
		ফেরত;

	ops_তথ্য = ops->user_তথ্য;

	যদি (ops_তথ্য) {
#ifdef CONFIG_OPTEE_TA_AVB
		যদি (ops_তথ্য->tee)
			tee_close_session(ops_তথ্য->tee, ops_তথ্য->session);
#endif
		avb_free(ops_তথ্য);
	}
}
