// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#ಅನ್ತರ್ಭಾವ <avb_verify.h>
#ಅನ್ತರ್ಭಾವ <blk.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <part.h>
#ಅನ್ತರ್ಭಾವ <tee.h>
#ಅನ್ತರ್ಭಾವ <tee/optee_ta_avb.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha256.h>

#ಯದಿ CONFIG_IS_ENABLED(AVB_ಬೇರು_ಕೀಲಿ_BUILTIN)
ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ avb_ಬೇರು_pub[1032] = {
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

ಧ್ರುವ ವರ್ಣ *str_avb_io_error(AvbIOResult res)
{
	ವಿಕಲ್ಪ (res) {
	ಪಕ್ಷ AVB_IO_RESULT_OK:
		ಪ್ರತ್ಯಾಗಮ "Requested operation was successful";
	ಪಕ್ಷ AVB_IO_RESULT_ERROR_IO:
		ಪ್ರತ್ಯಾಗಮ "Underlying hardware encountered an I/O error";
	ಪಕ್ಷ AVB_IO_RESULT_ERROR_OOM:
		ಪ್ರತ್ಯಾಗಮ "Unable to allocate memory";
	ಪಕ್ಷ AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION:
		ಪ್ರತ್ಯಾಗಮ "Requested partition does not exist";
	ಪಕ್ಷ AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION:
		ಪ್ರತ್ಯಾಗಮ "Bytes requested is outside the range of partition";
	ಪಕ್ಷ AVB_IO_RESULT_ERROR_NO_SUCH_ಮೌಲ್ಯ:
		ಪ್ರತ್ಯಾಗಮ "Named persistent value does not exist";
	ಪಕ್ಷ AVB_IO_RESULT_ERROR_INVALID_ಮೌಲ್ಯ_SIZE:
		ಪ್ರತ್ಯಾಗಮ "Named persistent value size is not supported";
	ಪಕ್ಷ AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE:
		ಪ್ರತ್ಯಾಗಮ "Buffer is too small for the requested operation";
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ "Unknown AVB error";
	}
}

ಧ್ರುವ ವರ್ಣ *str_avb_slot_error(AvbSlotVerifyResult res)
{
	ವಿಕಲ್ಪ (res) {
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_OK:
		ಪ್ರತ್ಯಾಗಮ "Verification passed successfully";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_OOM:
		ಪ್ರತ್ಯಾಗಮ "Allocation of memory failed";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_IO:
		ಪ್ರತ್ಯಾಗಮ "I/O error occurred while trying to load data";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION:
		ಪ್ರತ್ಯಾಗಮ "Digest didn't match or signature checks failed";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_ROLLBACK_INDEX:
		ಪ್ರತ್ಯಾಗಮ "Rollback index is less than its stored value";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_ಕೀಲಿ_REJECTED:
		ಪ್ರತ್ಯಾಗಮ "Public keys are not accepted";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_METADATA:
		ಪ್ರತ್ಯಾಗಮ "Metadata is invalid or inconsistent";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_UNSUPPORTED_VERSION:
		ಪ್ರತ್ಯಾಗಮ "Metadata requires a newer version of libavb";
	ಪಕ್ಷ AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_ARGUMENT:
		ಪ್ರತ್ಯಾಗಮ "Invalid arguments are used";
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
ವರ್ಣ *avb_set_state(AvbOps *ops, ಗಣನಾ avb_boot_state boot_state)
{
	ಸಂರಚನಾ AvbOpsData *data;
	ವರ್ಣ *cmdline = ರಿಕ್ತ;

	ಯದಿ (!ops)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	data = (ಸಂರಚನಾ AvbOpsData *)ops->user_data;
	ಯದಿ (!data)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	data->boot_state = boot_state;
	ವಿಕಲ್ಪ (boot_state) {
	ಪಕ್ಷ AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		ಭಙ್ಗ;
	ಪಕ್ಷ AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		ಭಙ್ಗ;
	ಪಕ್ಷ AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		ಭಙ್ಗ;
	ಪಕ್ಷ AVB_RED:
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ cmdline;
}

ವರ್ಣ *append_cmd_ಸಾಲು(ವರ್ಣ *cmdline_orig, ವರ್ಣ *cmdline_new)
{
	ವರ್ಣ *cmd_ಸಾಲು;

	ಯದಿ (!cmdline_new)
		ಪ್ರತ್ಯಾಗಮ cmdline_orig;

	ಯದಿ (cmdline_orig)
		cmd_ಸಾಲು = cmdline_orig;
	ಅನ್ಯಥಾ
		cmd_ಸಾಲು = " ";

	cmd_ಸಾಲು = avb_strdupv(cmd_ಸಾಲು, " ", cmdline_new, ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ cmd_ಸಾಲು;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ avb_find_dm_args(ವರ್ಣ **args, ವರ್ಣ *str)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಪ್ರತಿ (i = 0; i < AVB_MAX_ARGS && args[i]; ++i) {
		ಯದಿ (strstr(args[i], str))
			ಪ್ರತ್ಯಾಗಮ i;
	}

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ವರ್ಣ *avb_set_enforce_option(ಧ್ರುವ ವರ್ಣ *cmdline, ಧ್ರುವ ವರ್ಣ *option)
{
	ವರ್ಣ *cmdarg[AVB_MAX_ARGS];
	ವರ್ಣ *newargs = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ i = 0;
	ಪೂರ್ಣಾಙ್ಕ total_args;

	memset(cmdarg, 0, ಪರಿಮಾಣ(cmdarg));
	cmdarg[i++] = strtok((ವರ್ಣ *)cmdline, " ");

	ಕುರು {
		cmdarg[i] = strtok(ರಿಕ್ತ, " ");
		ಯದಿ (!cmdarg[i])
			ಭಙ್ಗ;

		ಯದಿ (++i >= AVB_MAX_ARGS) {
			printf("%s: Can't handle more then %d args\n",
			       __func__, i);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	} ಯಾವತ್ (true);

	total_args = i;
	i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_LOGGING);
	ಯದಿ (i >= 0) {
		cmdarg[i] = (ವರ್ಣ *)option;
	} ಅನ್ಯಥಾ {
		i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_RESTART);
		ಯದಿ (i < 0) {
			printf("%s: No verity options found\n", __func__);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		cmdarg[i] = (ವರ್ಣ *)option;
	}

	ಪ್ರತಿ (i = 0; i <= total_args; i++)
		newargs = append_cmd_ಸಾಲು(newargs, cmdarg[i]);

	ಪ್ರತ್ಯಾಗಮ newargs;
}

ವರ್ಣ *avb_set_ignore_corruption(ಧ್ರುವ ವರ್ಣ *cmdline)
{
	ವರ್ಣ *newargs = ರಿಕ್ತ;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_LOGGING);
	ಯದಿ (newargs)
		newargs = append_cmd_ಸಾಲು(newargs,
					  "androidboot.veritymode=eio");

	ಪ್ರತ್ಯಾಗಮ newargs;
}

ವರ್ಣ *avb_set_enforce_verity(ಧ್ರುವ ವರ್ಣ *cmdline)
{
	ವರ್ಣ *newargs;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_RESTART);
	ಯದಿ (newargs)
		newargs = append_cmd_ಸಾಲು(newargs,
					  "androidboot.veritymode=enforcing");
	ಪ್ರತ್ಯಾಗಮ newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
ಸ್ಥಿರ unsigned long mmc_read_and_flush(ಸಂರಚನಾ mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					ಶೂನ್ಯ *buffer)
{
	unsigned long blks;
	ಶೂನ್ಯ *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	ಯದಿ (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	ಯದಿ (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		ಯದಿ (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;
	} ಅನ್ಯಥಾ {
		tmp_buf = buffer;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	ಯದಿ (unaligned)
		memcpy(buffer, tmp_buf, sectors * part->info.blksz);

	ಪ್ರತ್ಯಾಗಮ blks;
}

ಸ್ಥಿರ unsigned long mmc_write(ಸಂರಚನಾ mmc_part *part, lbaint_t start,
			       lbaint_t sectors, ಶೂನ್ಯ *buffer)
{
	ಶೂನ್ಯ *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	ಯದಿ (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	ಯದಿ (unaligned) {
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		ಯದಿ (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;

		memcpy(tmp_buf, buffer, sectors * part->info.blksz);
	} ಅನ್ಯಥಾ {
		tmp_buf = buffer;
	}

	ಪ್ರತ್ಯಾಗಮ blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

ಸ್ಥಿರ ಸಂರಚನಾ mmc_part *get_partition(AvbOps *ops, ಧ್ರುವ ವರ್ಣ *partition)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	u8 dev_ಅಂಕೆ;
	ಪೂರ್ಣಾಙ್ಕ part_ಅಂಕೆ = 0;
	ಸಂರಚನಾ mmc_part *part;
	ಸಂರಚನಾ blk_desc *mmc_blk;

	part = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ mmc_part));
	ಯದಿ (!part)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	dev_ಅಂಕೆ = get_boot_device(ops);
	part->mmc = find_mmc_device(dev_ಅಂಕೆ);
	ಯದಿ (!part->mmc) {
		printf("%s: no MMC device at slot %x\n", __func__, dev_ಅಂಕೆ);
		ಗಚ್ಛ err;
	}

	ret = mmc_init(part->mmc);
	ಯದಿ (ret) {
		printf("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		ಗಚ್ಛ err;
	}

	ಯದಿ (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_ಅಂಕೆ);
		ಯದಿ (ret) {
			printf("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			ಗಚ್ಛ err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	ಯದಿ (!mmc_blk) {
		printf("%s: failed to obtain block descriptor\n", __func__);
		ಗಚ್ಛ err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	ಯದಿ (ret < 0) {
		printf("%s: can't find partition '%s'\n", __func__, partition);
		ಗಚ್ಛ err;
	}

	part->dev_ಅಂಕೆ = dev_ಅಂಕೆ;
	part->mmc_blk = mmc_blk;

	ಪ್ರತ್ಯಾಗಮ part;
err:
	ಮುಕ್ತ(part);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ AvbIOResult mmc_byte_io(AvbOps *ops,
			       ಧ್ರುವ ವರ್ಣ *partition,
			       s64 offset,
			       size_t ಅಂಕೆ_bytes,
			       ಶೂನ್ಯ *buffer,
			       size_t *out_ಅಂಕೆ_read,
			       ಗಣನಾ mmc_io_type io_type)
{
	ulong ret;
	ಸಂರಚನಾ mmc_part *part;
	u64 start_offset, start_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_RESULT_OK;

	ಯದಿ (!partition || !buffer || io_type > IO_WRITE)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;

	part = get_partition(ops, partition);
	ಯದಿ (!part)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	ಯದಿ (!part->info.blksz) {
		io_ret = AVB_IO_RESULT_ERROR_IO;
		ಗಚ್ಛ out;
	}

	start_offset = calc_offset(part, offset);
	ಯಾವತ್ (ಅಂಕೆ_bytes) {
		start_sector = start_offset / part->info.blksz;
		sectors = ಅಂಕೆ_bytes / part->info.blksz;
		/* handle non block-aligned reads */
		ಯದಿ (start_offset % part->info.blksz ||
		    ಅಂಕೆ_bytes < part->info.blksz) {
			tmp_buf = get_sector_buf();
			ಯದಿ (start_offset % part->info.blksz) {
				residue = part->info.blksz -
					(start_offset % part->info.blksz);
				ಯದಿ (residue > ಅಂಕೆ_bytes)
					residue = ಅಂಕೆ_bytes;
			} ಅನ್ಯಥಾ {
				residue = ಅಂಕೆ_bytes;
			}

			ಯದಿ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				ಯದಿ (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					ಗಚ್ಛ out;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_offset % part->info.blksz);
				memcpy(buffer, (ಶೂನ್ಯ *)tmp_buf, residue);
			} ಅನ್ಯಥಾ {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				ಯದಿ (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					ಗಚ್ಛ out;
				}
				memcpy((ಶೂನ್ಯ *)tmp_buf +
					start_offset % part->info.blksz,
					buffer, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				ಯದಿ (ret != 1) {
					printf("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					ಗಚ್ಛ out;
				}
			}

			io_cnt += residue;
			buffer += residue;
			start_offset += residue;
			ಅಂಕೆ_bytes -= residue;
			ಅನುವರ್ತ;
		}

		ಯದಿ (sectors) {
			ಯದಿ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, buffer);
			} ಅನ್ಯಥಾ {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, buffer);
			}

			ಯದಿ (!ret) {
				printf("%s: sector read error\n", __func__);
				io_ret = AVB_IO_RESULT_ERROR_IO;
				ಗಚ್ಛ out;
			}

			io_cnt += ret * part->info.blksz;
			buffer += ret * part->info.blksz;
			start_offset += ret * part->info.blksz;
			ಅಂಕೆ_bytes -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	ಯದಿ (io_type == IO_READ && out_ಅಂಕೆ_read)
		*out_ಅಂಕೆ_read = io_cnt;

out:
	ಮುಕ್ತ(part);
	ಪ್ರತ್ಯಾಗಮ io_ret;
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
ಸ್ಥಿರ AvbIOResult read_from_partition(AvbOps *ops,
				       ಧ್ರುವ ವರ್ಣ *partition_name,
				       s64 offset_from_partition,
				       size_t ಅಂಕೆ_bytes,
				       ಶೂನ್ಯ *buffer,
				       size_t *out_ಅಂಕೆ_read)
{
	ಪ್ರತ್ಯಾಗಮ mmc_byte_io(ops, partition_name, offset_from_partition,
			   ಅಂಕೆ_bytes, buffer, out_ಅಂಕೆ_read, IO_READ);
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
ಸ್ಥಿರ AvbIOResult write_to_partition(AvbOps *ops,
				      ಧ್ರುವ ವರ್ಣ *partition_name,
				      s64 offset_from_partition,
				      size_t ಅಂಕೆ_bytes,
				      ಧ್ರುವ ಶೂನ್ಯ *buffer)
{
	ಪ್ರತ್ಯಾಗಮ mmc_byte_io(ops, partition_name, offset_from_partition,
			   ಅಂಕೆ_bytes, (ಶೂನ್ಯ *)buffer, ರಿಕ್ತ, IO_WRITE);
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
ಸ್ಥಿರ AvbIOResult validate_vbmeta_public_ಕೀಲಿ(AvbOps *ops,
					      ಧ್ರುವ u8 *public_ಕೀಲಿ_data,
					      size_t public_ಕೀಲಿ_length,
					      ಧ್ರುವ u8
					      *public_ಕೀಲಿ_metadata,
					      size_t
					      public_ಕೀಲಿ_metadata_length,
					      bool *out_ಕೀಲಿ_is_trusted)
{
	u8 ಕೀಲಿ_digest[SHA256_SUM_LEN];
	u8 trusted_digest[SHA256_SUM_LEN];
	AvbIOResult rc;

	ಯದಿ (!public_ಕೀಲಿ_length || !public_ಕೀಲಿ_data || !out_ಕೀಲಿ_is_trusted)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*out_ಕೀಲಿ_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(public_ಕೀಲಿ_data, public_ಕೀಲಿ_length, ಕೀಲಿ_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_ಬೇರು_ಕೀಲಿ_digest(ops, trusted_digest);
	ಯದಿ (rc != AVB_IO_RESULT_OK)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಯದಿ (avb_safe_memcmp(ಕೀಲಿ_digest, trusted_digest, ಪರಿಮಾಣ(ಕೀಲಿ_digest)) == 0)
		*out_ಕೀಲಿ_is_trusted = true;

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_open_session(ಸಂರಚನಾ AvbOpsData *ops_data)
{
	ಸಂರಚನಾ udevice *tee = ರಿಕ್ತ;

	ಯಾವತ್ (!ops_data->tee) {
		ಧ್ರುವ ಸಂರಚನಾ tee_optee_ta_uuid uuid = TA_AVB_UUID;
		ಸಂರಚನಾ tee_open_session_arg arg;
		ಪೂರ್ಣಾಙ್ಕ rc;

		tee = tee_find_device(tee, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ);
		ಯದಿ (!tee)
			ಪ್ರತ್ಯಾಗಮ -ENODEV;

		memset(&arg, 0, ಪರಿಮಾಣ(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, ರಿಕ್ತ);
		ಯದಿ (rc || arg.ret)
			ಅನುವರ್ತ;

		ops_data->tee = tee;
		ops_data->session = arg.session;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ AvbIOResult invoke_func(ಸಂರಚನಾ AvbOpsData *ops_data, u32 func,
			       ulong ಅಂಕೆ_param, ಸಂರಚನಾ tee_param *param)
{
	ಸಂರಚನಾ tee_invoke_arg arg;

	ಯದಿ (get_open_session(ops_data))
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;

	memset(&arg, 0, ಪರಿಮಾಣ(arg));
	arg.func = func;
	arg.session = ops_data->session;

	ಯದಿ (tee_invoke_func(ops_data->tee, &arg, ಅಂಕೆ_param, param))
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;
	ವಿಕಲ್ಪ (arg.ret) {
	ಪಕ್ಷ TEE_SUCCESS:
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
	ಪಕ್ಷ TEE_ERROR_OUT_OF_MEMORY:
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_OOM;
	ಪಕ್ಷ TEE_ERROR_STORAGE_NO_SPACE:
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	ಪಕ್ಷ TEE_ERROR_ITEM_NOT_FOUND:
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_ಮೌಲ್ಯ;
	ಪಕ್ಷ TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_data->tee, ops_data->session);
		ops_data->tee = ರಿಕ್ತ;
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;
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
ಸ್ಥಿರ AvbIOResult read_rollback_index(AvbOps *ops,
				       size_t rollback_index_slot,
				       u64 *out_rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ಯದಿ (out_rollback_index)
		*out_rollback_index = 0;

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
#ಅನ್ಯಥಾ
	AvbIOResult rc;
	ಸಂರಚನಾ tee_param param[2];

	ಯದಿ (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_ಮೌಲ್ಯ;

	memset(param, 0, ಪರಿಮಾಣ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param[0].u.ಮೌಲ್ಯ.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_OUTPUT;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_ROLLBACK_INDEX,
			 ARRAY_SIZE(param), param);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	*out_rollback_index = (u64)param[1].u.ಮೌಲ್ಯ.a << 32 |
			      (u32)param[1].u.ಮೌಲ್ಯ.b;
	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
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
ಸ್ಥಿರ AvbIOResult write_rollback_index(AvbOps *ops,
					size_t rollback_index_slot,
					u64 rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
#ಅನ್ಯಥಾ
	ಸಂರಚನಾ tee_param param[2];

	ಯದಿ (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_ಮೌಲ್ಯ;

	memset(param, 0, ಪರಿಮಾಣ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param[0].u.ಮೌಲ್ಯ.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param[1].u.ಮೌಲ್ಯ.a = (u32)(rollback_index >> 32);
	param[1].u.ಮೌಲ್ಯ.b = (u32)rollback_index;

	ಪ್ರತ್ಯಾಗಮ invoke_func(ops->user_data, TA_AVB_CMD_WRITE_ROLLBACK_INDEX,
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
ಸ್ಥಿರ AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*out_is_unlocked = true;

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
#ಅನ್ಯಥಾ
	AvbIOResult rc;
	ಸಂರಚನಾ tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_OUTPUT };

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;
	*out_is_unlocked = !param.u.ಮೌಲ್ಯ.a;
	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
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
ಸ್ಥಿರ AvbIOResult get_unique_guid_for_partition(AvbOps *ops,
						 ಧ್ರುವ ವರ್ಣ *partition,
						 ವರ್ಣ *guid_buf,
						 size_t guid_buf_size)
{
	ಸಂರಚನಾ mmc_part *part;
	size_t uuid_size;

	part = get_partition(ops, partition);
	ಯದಿ (!part)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	uuid_size = ಪರಿಮಾಣ(part->info.uuid);
	ಯದಿ (uuid_size > guid_buf_size) {
		ಮುಕ್ತ(part);
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;
	}

	memcpy(guid_buf, part->info.uuid, uuid_size);
	guid_buf[uuid_size - 1] = 0;

	ಮುಕ್ತ(part);
	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
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
ಸ್ಥಿರ AvbIOResult get_size_of_partition(AvbOps *ops,
					 ಧ್ರುವ ವರ್ಣ *partition,
					 u64 *out_size_ಅಂಕೆ_bytes)
{
	ಸಂರಚನಾ mmc_part *part;

	ಯದಿ (!out_size_ಅಂಕೆ_bytes)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	ಯದಿ (!part)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	*out_size_ಅಂಕೆ_bytes = part->info.blksz * part->info.size;
	ಮುಕ್ತ(part);

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
ಸ್ಥಿರ AvbIOResult read_persistent_ಮೌಲ್ಯ(AvbOps *ops,
					 ಧ್ರುವ ವರ್ಣ *name,
					 size_t buffer_size,
					 u8 *out_buffer,
					 size_t *out_ಅಂಕೆ_bytes_read)
{
	AvbIOResult rc;
	ಸಂರಚನಾ tee_shm *shm_name;
	ಸಂರಚನಾ tee_shm *shm_buf;
	ಸಂರಚನಾ tee_param param[2];
	ಸಂರಚನಾ udevice *tee;
	size_t name_size = strlen(name) + 1;

	ಯದಿ (get_open_session(ops->user_data))
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;

	tee = ((ಸಂರಚನಾ AvbOpsData *)ops->user_data)->tee;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, buffer_size,
			   TEE_SHM_ALLOC, &shm_buf);
	ಯದಿ (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		ಗಚ್ಛ free_name;
	}

	memcpy(shm_name->addr, name, name_size);

	memset(param, 0, ಪರಿಮಾಣ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ಒಳಹರಿವು;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buffer_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_PERSIST_ಮೌಲ್ಯ,
			 2, param);
	ಯದಿ (rc)
		ಗಚ್ಛ out;

	ಯದಿ (param[1].u.memref.size > buffer_size) {
		rc = AVB_IO_RESULT_ERROR_NO_SUCH_ಮೌಲ್ಯ;
		ಗಚ್ಛ out;
	}

	*out_ಅಂಕೆ_bytes_read = param[1].u.memref.size;

	memcpy(out_buffer, shm_buf->addr, *out_ಅಂಕೆ_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ AvbIOResult write_persistent_ಮೌಲ್ಯ(AvbOps *ops,
					  ಧ್ರುವ ವರ್ಣ *name,
					  size_t ಮೌಲ್ಯ_size,
					  ಧ್ರುವ u8 *ಮೌಲ್ಯ)
{
	AvbIOResult rc;
	ಸಂರಚನಾ tee_shm *shm_name;
	ಸಂರಚನಾ tee_shm *shm_buf;
	ಸಂರಚನಾ tee_param param[2];
	ಸಂರಚನಾ udevice *tee;
	size_t name_size = strlen(name) + 1;

	ಯದಿ (get_open_session(ops->user_data))
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_IO;

	tee = ((ಸಂರಚನಾ AvbOpsData *)ops->user_data)->tee;

	ಯದಿ (!ಮೌಲ್ಯ_size)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_ಮೌಲ್ಯ;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, ಮೌಲ್ಯ_size,
			   TEE_SHM_ALLOC, &shm_buf);
	ಯದಿ (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		ಗಚ್ಛ free_name;
	}

	memcpy(shm_name->addr, name, name_size);
	memcpy(shm_buf->addr, ಮೌಲ್ಯ, ಮೌಲ್ಯ_size);

	memset(param, 0, ಪರಿಮಾಣ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ಒಳಹರಿವು;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ಒಳಹರಿವು;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = ಮೌಲ್ಯ_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_WRITE_PERSIST_ಮೌಲ್ಯ,
			 2, param);
	ಯದಿ (rc)
		ಗಚ್ಛ out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ಪ್ರತ್ಯಾಗಮ rc;
}
#endif

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#ಯದಿ CONFIG_IS_ENABLED(AVB_ಬೇರು_ಕೀಲಿ_BUILTIN)
AvbIOResult avb_read_ಬೇರು_ಕೀಲಿ_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_ಬೇರು_pub, ಪರಿಮಾಣ(avb_ಬೇರು_pub), digest,
		       CHUNKSZ_SHA256);

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
}
#ನೋಚೇದ್ಯದಿ CONFIG_IS_ENABLED(AVB_ಬೇರು_ಕೀಲಿ_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_ಬೇರು_ಕೀಲಿ_digest(AvbOps *ops, uint8_t *digest)
{
	size_t ಅಂಕೆ_read = 0;
	AvbIOResult rc;

	rc = read_persistent_ಮೌಲ್ಯ(ops, CONFIG_AVB_ಬೇರು_ಕೀಲಿ_TEE_NAME,
				   SHA256_SUM_LEN, digest, &ಅಂಕೆ_read);
	ಯದಿ (rc != AVB_IO_RESULT_OK)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಯದಿ (ಅಂಕೆ_read != SHA256_SUM_LEN)
		ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_INVALID_ಮೌಲ್ಯ_SIZE;

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_OK;
}
#ಅನ್ಯಥಾ /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_ಬೇರು_ಕೀಲಿ_digest(AvbOps *ops, uint8_t *digest)
{
	printf("%s: board root key provider not implemented\n", __func__);

	ಪ್ರತ್ಯಾಗಮ AVB_IO_RESULT_ERROR_NO_SUCH_ಮೌಲ್ಯ;
}
#endif

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(ಪೂರ್ಣಾಙ್ಕ boot_device)
{
	ಸಂರಚನಾ AvbOpsData *ops_data;

	ops_data = avb_calloc(ಪರಿಮಾಣ(ಸಂರಚನಾ AvbOpsData));
	ಯದಿ (!ops_data)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ops_data->ops.user_data = ops_data;

	ops_data->ops.read_from_partition = read_from_partition;
	ops_data->ops.write_to_partition = write_to_partition;
	ops_data->ops.validate_vbmeta_public_ಕೀಲಿ = validate_vbmeta_public_ಕೀಲಿ;
	ops_data->ops.read_rollback_index = read_rollback_index;
	ops_data->ops.write_rollback_index = write_rollback_index;
	ops_data->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_data->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
#ifdef CONFIG_OPTEE_TA_AVB
	ops_data->ops.write_persistent_ಮೌಲ್ಯ = write_persistent_ಮೌಲ್ಯ;
	ops_data->ops.read_persistent_ಮೌಲ್ಯ = read_persistent_ಮೌಲ್ಯ;
#endif
	ops_data->ops.get_size_of_partition = get_size_of_partition;
	ops_data->mmc_dev = boot_device;

	ಪ್ರತ್ಯಾಗಮ &ops_data->ops;
}

ಶೂನ್ಯ avb_ops_free(AvbOps *ops)
{
	ಸಂರಚನಾ AvbOpsData *ops_data;

	ಯದಿ (!ops)
		ಪ್ರತ್ಯಾಗಮ;

	ops_data = ops->user_data;

	ಯದಿ (ops_data) {
#ifdef CONFIG_OPTEE_TA_AVB
		ಯದಿ (ops_data->tee)
			tee_close_session(ops_data->tee, ops_data->session);
#endif
		avb_free(ops_data);
	}
}
