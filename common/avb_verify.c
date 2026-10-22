// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <avb_verify.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <blk.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <part.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tee.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tee/optee_ta_avb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/sha256.h>

#ਜੇ CONFIG_IS_ENABLED(AVB_ਜੜ੍ਹ_ਕੁੰਜੀ_BUILTIN)
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ avb_ਜੜ੍ਹ_pub[1032] = {
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
#ਜੇ_ਅੰਤ /* AVB_ROOT_KEY_BUILTIN */

ਅਚਲ ਅੱਖਰ *str_avb_io_error(AvbIOResult res)
{
	ਚੋਣ (res) {
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_OK:
		ਵਾਪਸ "Requested operation was successful";
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_ERROR_IO:
		ਵਾਪਸ "Underlying hardware encountered an I/O error";
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_ERROR_OOM:
		ਵਾਪਸ "Unable to allocate memory";
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਹਿੱਸੇ_ਕਰੋ:
		ਵਾਪਸ "Requested partition does not exist";
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_ERROR_RANGE_OUTSIDE_ਹਿੱਸੇ_ਕਰੋ:
		ਵਾਪਸ "Bytes requested is outside the range of partition";
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਮੁੱਲ:
		ਵਾਪਸ "Named persistent value does not exist";
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_ERROR_INVALID_ਮੁੱਲ_ਮਾਪ:
		ਵਾਪਸ "Named persistent value size is not supported";
	ਹਾਲਤ AVB_IO_ਨਤੀਜਾ_ERROR_INSUFFICIENT_SPACE:
		ਵਾਪਸ "Buffer is too small for the requested operation";
	ਮੂਲ:
		ਵਾਪਸ "Unknown AVB error";
	}
}

ਅਚਲ ਅੱਖਰ *str_avb_ਥਾਂ_error(Avb_ਥਾਂ_Verify_ਨਤੀਜਾ res)
{
	ਚੋਣ (res) {
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_OK:
		ਵਾਪਸ "Verification passed successfully";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_OOM:
		ਵਾਪਸ "Allocation of memory failed";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_IO:
		ਵਾਪਸ "I/O error occurred while trying to load data";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_VERIFICATION:
		ਵਾਪਸ "Digest didn't match or signature checks failed";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_ROLLBACK_ਸੂਚਕਾਂਕ:
		ਵਾਪਸ "Rollback index is less than its stored value";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_ਜਨਤਕ_ਕੁੰਜੀ_REJECTED:
		ਵਾਪਸ "Public keys are not accepted";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_INVALID_METADATA:
		ਵਾਪਸ "Metadata is invalid or inconsistent";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_UNSUPPORTED_ਸੰਸਕਰਣ:
		ਵਾਪਸ "Metadata requires a newer version of libavb";
	ਹਾਲਤ AVB_ਥਾਂ_VERIFY_ਨਤੀਜਾ_ERROR_INVALID_ਦਲੀਲ:
		ਵਾਪਸ "Invalid arguments are used";
	ਮੂਲ:
		ਵਾਪਸ "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
ਅੱਖਰ *avb_ਸੈੱਟ_state(AvbOps *ops, ਗਿਣਤੀ avb_boot_state boot_state)
{
	ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *ਅੰਕੜੇ;
	ਅੱਖਰ *cmdline = ਨਿਰਾ;

	ਜੇ (!ops)
		ਵਾਪਸ ਨਿਰਾ;

	ਅੰਕੜੇ = (ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *)ops->user_ਅੰਕੜੇ;
	ਜੇ (!ਅੰਕੜੇ)
		ਵਾਪਸ ਨਿਰਾ;

	ਅੰਕੜੇ->boot_state = boot_state;
	ਚੋਣ (boot_state) {
	ਹਾਲਤ AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		ਰੋਕੋ;
	ਹਾਲਤ AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		ਰੋਕੋ;
	ਹਾਲਤ AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		ਰੋਕੋ;
	ਹਾਲਤ AVB_RED:
		ਰੋਕੋ;
	}

	ਵਾਪਸ cmdline;
}

ਅੱਖਰ *append_cmd_ਸਤਰ(ਅੱਖਰ *cmdline_orig, ਅੱਖਰ *cmdline_new)
{
	ਅੱਖਰ *cmd_ਸਤਰ;

	ਜੇ (!cmdline_new)
		ਵਾਪਸ cmdline_orig;

	ਜੇ (cmdline_orig)
		cmd_ਸਤਰ = cmdline_orig;
	ਨਹੀਂਤਾਂ
		cmd_ਸਤਰ = " ";

	cmd_ਸਤਰ = avb_strdupv(cmd_ਸਤਰ, " ", cmdline_new, ਨਿਰਾ);

	ਵਾਪਸ cmd_ਸਤਰ;
}

ਸਥਿਰ ਪੂਰਨ avb_ਲੱਭੋ_dm_ਦਲੀਲਾਂ(ਅੱਖਰ **ਦਲੀਲਾਂ, ਅੱਖਰ *str)
{
	ਪੂਰਨ i;

	ਜੇ (!str)
		ਵਾਪਸ -1;

	ਲਈ (i = 0; i < AVB_MAX_ਦਲੀਲਾਂ && ਦਲੀਲਾਂ[i]; ++i) {
		ਜੇ (ਲੜੀ_ਖੋਜ(ਦਲੀਲਾਂ[i], str))
			ਵਾਪਸ i;
	}

	ਵਾਪਸ -1;
}

ਸਥਿਰ ਅੱਖਰ *avb_ਸੈੱਟ_enforce_option(ਅਚਲ ਅੱਖਰ *cmdline, ਅਚਲ ਅੱਖਰ *option)
{
	ਅੱਖਰ *cmdarg[AVB_MAX_ਦਲੀਲਾਂ];
	ਅੱਖਰ *newargs = ਨਿਰਾ;
	ਪੂਰਨ i = 0;
	ਪੂਰਨ ਕੁੱਲ_ਦਲੀਲਾਂ;

	memset(cmdarg, 0, ਆਕਾਰ(cmdarg));
	cmdarg[i++] = strtok((ਅੱਖਰ *)cmdline, " ");

	ਕਰੋ {
		cmdarg[i] = strtok(ਨਿਰਾ, " ");
		ਜੇ (!cmdarg[i])
			ਰੋਕੋ;

		ਜੇ (++i >= AVB_MAX_ਦਲੀਲਾਂ) {
			ਲਿਖੋ("%s: Can't handle more then %d args\n",
			       __func__, i);
			ਵਾਪਸ ਨਿਰਾ;
		}
	} ਜਦੋਂ (true);

	ਕੁੱਲ_ਦਲੀਲਾਂ = i;
	i = avb_ਲੱਭੋ_dm_ਦਲੀਲਾਂ(&cmdarg[0], VERITY_ਸਾਰਣੀ_OPT_LOGGING);
	ਜੇ (i >= 0) {
		cmdarg[i] = (ਅੱਖਰ *)option;
	} ਨਹੀਂਤਾਂ {
		i = avb_ਲੱਭੋ_dm_ਦਲੀਲਾਂ(&cmdarg[0], VERITY_ਸਾਰਣੀ_OPT_RESTART);
		ਜੇ (i < 0) {
			ਲਿਖੋ("%s: No verity options found\n", __func__);
			ਵਾਪਸ ਨਿਰਾ;
		}

		cmdarg[i] = (ਅੱਖਰ *)option;
	}

	ਲਈ (i = 0; i <= ਕੁੱਲ_ਦਲੀਲਾਂ; i++)
		newargs = append_cmd_ਸਤਰ(newargs, cmdarg[i]);

	ਵਾਪਸ newargs;
}

ਅੱਖਰ *avb_ਸੈੱਟ_ignore_corruption(ਅਚਲ ਅੱਖਰ *cmdline)
{
	ਅੱਖਰ *newargs = ਨਿਰਾ;

	newargs = avb_ਸੈੱਟ_enforce_option(cmdline, VERITY_ਸਾਰਣੀ_OPT_LOGGING);
	ਜੇ (newargs)
		newargs = append_cmd_ਸਤਰ(newargs,
					  "androidboot.veritymode=eio");

	ਵਾਪਸ newargs;
}

ਅੱਖਰ *avb_ਸੈੱਟ_enforce_verity(ਅਚਲ ਅੱਖਰ *cmdline)
{
	ਅੱਖਰ *newargs;

	newargs = avb_ਸੈੱਟ_enforce_option(cmdline, VERITY_ਸਾਰਣੀ_OPT_RESTART);
	ਜੇ (newargs)
		newargs = append_cmd_ਸਤਰ(newargs,
					  "androidboot.veritymode=enforcing");
	ਵਾਪਸ newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ mmc_read_and_flush(ਬਣਤਰ mmc_part *part,
					lbaint_t ਸ਼ੁਰੂ,
					lbaint_t sectors,
					ਖਾਲੀ *ਬਫਰ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ blks;
	ਖਾਲੀ *tmp_buf;
	size_t buf_ਮਾਪ;
	bool unaligned = is_buf_unaligned(ਬਫਰ);

	ਜੇ (ਸ਼ੁਰੂ < part->info.ਸ਼ੁਰੂ) {
		ਲਿਖੋ("%s: partition start out of bounds\n", __func__);
		ਵਾਪਸ 0;
	}
	ਜੇ ((ਸ਼ੁਰੂ + sectors) > (part->info.ਸ਼ੁਰੂ + part->info.ਮਾਪ)) {
		sectors = part->info.ਸ਼ੁਰੂ + part->info.ਮਾਪ - ਸ਼ੁਰੂ;
		ਲਿਖੋ("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	ਜੇ (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, ਬਫਰ);
		tmp_buf = get_sector_buf();
		buf_ਮਾਪ = get_sector_buf_ਮਾਪ();
		ਜੇ (sectors > buf_ਮਾਪ / part->info.blksz)
			sectors = buf_ਮਾਪ / part->info.blksz;
	} ਨਹੀਂਤਾਂ {
		tmp_buf = ਬਫਰ;
	}

	blks = blk_dread(part->mmc_blk,
			 ਸ਼ੁਰੂ, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	ਜੇ (unaligned)
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ, tmp_buf, sectors * part->info.blksz);

	ਵਾਪਸ blks;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ mmc_write(ਬਣਤਰ mmc_part *part, lbaint_t ਸ਼ੁਰੂ,
			       lbaint_t sectors, ਖਾਲੀ *ਬਫਰ)
{
	ਖਾਲੀ *tmp_buf;
	size_t buf_ਮਾਪ;
	bool unaligned = is_buf_unaligned(ਬਫਰ);

	ਜੇ (ਸ਼ੁਰੂ < part->info.ਸ਼ੁਰੂ) {
		ਲਿਖੋ("%s: partition start out of bounds\n", __func__);
		ਵਾਪਸ 0;
	}
	ਜੇ ((ਸ਼ੁਰੂ + sectors) > (part->info.ਸ਼ੁਰੂ + part->info.ਮਾਪ)) {
		sectors = part->info.ਸ਼ੁਰੂ + part->info.ਮਾਪ - ਸ਼ੁਰੂ;
		ਲਿਖੋ("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	ਜੇ (unaligned) {
		tmp_buf = get_sector_buf();
		buf_ਮਾਪ = get_sector_buf_ਮਾਪ();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, ਬਫਰ);
		ਜੇ (sectors > buf_ਮਾਪ / part->info.blksz)
			sectors = buf_ਮਾਪ / part->info.blksz;

		ਸਮ੍ਰਿਤੀ_ਨਕਲ(tmp_buf, ਬਫਰ, sectors * part->info.blksz);
	} ਨਹੀਂਤਾਂ {
		tmp_buf = ਬਫਰ;
	}

	ਵਾਪਸ blk_dwrite(part->mmc_blk,
			  ਸ਼ੁਰੂ, sectors, tmp_buf);
}

ਸਥਿਰ ਬਣਤਰ mmc_part *get_ਹਿੱਸੇ_ਕਰੋ(AvbOps *ops, ਅਚਲ ਅੱਖਰ *ਹਿੱਸੇ_ਕਰੋ)
{
	ਪੂਰਨ ret;
	u8 dev_ਅੰਕ;
	ਪੂਰਨ part_ਅੰਕ = 0;
	ਬਣਤਰ mmc_part *part;
	ਬਣਤਰ blk_desc *mmc_blk;

	part = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ mmc_part));
	ਜੇ (!part)
		ਵਾਪਸ ਨਿਰਾ;

	dev_ਅੰਕ = get_boot_device(ops);
	part->mmc = ਲੱਭੋ_mmc_device(dev_ਅੰਕ);
	ਜੇ (!part->mmc) {
		ਲਿਖੋ("%s: no MMC device at slot %x\n", __func__, dev_ਅੰਕ);
		ਜਾਓ err;
	}

	ret = mmc_init(part->mmc);
	ਜੇ (ret) {
		ਲਿਖੋ("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		ਜਾਓ err;
	}

	ਜੇ (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_ਅੰਕ);
		ਜੇ (ret) {
			ਲਿਖੋ("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			ਜਾਓ err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	ਜੇ (!mmc_blk) {
		ਲਿਖੋ("%s: failed to obtain block descriptor\n", __func__);
		ਜਾਓ err;
	}

	ret = part_get_info_by_name(mmc_blk, ਹਿੱਸੇ_ਕਰੋ, &part->info);
	ਜੇ (ret < 0) {
		ਲਿਖੋ("%s: can't find partition '%s'\n", __func__, ਹਿੱਸੇ_ਕਰੋ);
		ਜਾਓ err;
	}

	part->dev_ਅੰਕ = dev_ਅੰਕ;
	part->mmc_blk = mmc_blk;

	ਵਾਪਸ part;
err:
	ਮੁਕਤ(part);
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ AvbIOResult mmc_byte_io(AvbOps *ops,
			       ਅਚਲ ਅੱਖਰ *ਹਿੱਸੇ_ਕਰੋ,
			       s64 ਵਿੱਥ,
			       size_t ਅੰਕ_ਬਾਈਟਾਂ,
			       ਖਾਲੀ *ਬਫਰ,
			       size_t *ਬਾਹਰਲਾ_ਅੰਕ_read,
			       ਗਿਣਤੀ mmc_io_type io_type)
{
	ulong ret;
	ਬਣਤਰ mmc_part *part;
	u64 ਸ਼ੁਰੂ_ਵਿੱਥ, ਸ਼ੁਰੂ_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_ਨਤੀਜਾ_OK;

	ਜੇ (!ਹਿੱਸੇ_ਕਰੋ || !ਬਫਰ || io_type > IO_WRITE)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;

	part = get_ਹਿੱਸੇ_ਕਰੋ(ops, ਹਿੱਸੇ_ਕਰੋ);
	ਜੇ (!part)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਹਿੱਸੇ_ਕਰੋ;

	ਜੇ (!part->info.blksz) {
		io_ret = AVB_IO_ਨਤੀਜਾ_ERROR_IO;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਸ਼ੁਰੂ_ਵਿੱਥ = calc_ਵਿੱਥ(part, ਵਿੱਥ);
	ਜਦੋਂ (ਅੰਕ_ਬਾਈਟਾਂ) {
		ਸ਼ੁਰੂ_sector = ਸ਼ੁਰੂ_ਵਿੱਥ / part->info.blksz;
		sectors = ਅੰਕ_ਬਾਈਟਾਂ / part->info.blksz;
		/* handle non block-aligned reads */
		ਜੇ (ਸ਼ੁਰੂ_ਵਿੱਥ % part->info.blksz ||
		    ਅੰਕ_ਬਾਈਟਾਂ < part->info.blksz) {
			tmp_buf = get_sector_buf();
			ਜੇ (ਸ਼ੁਰੂ_ਵਿੱਥ % part->info.blksz) {
				residue = part->info.blksz -
					(ਸ਼ੁਰੂ_ਵਿੱਥ % part->info.blksz);
				ਜੇ (residue > ਅੰਕ_ਬਾਈਟਾਂ)
					residue = ਅੰਕ_ਬਾਈਟਾਂ;
			} ਨਹੀਂਤਾਂ {
				residue = ਅੰਕ_ਬਾਈਟਾਂ;
			}

			ਜੇ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.ਸ਼ੁਰੂ +
							 ਸ਼ੁਰੂ_sector,
							 1, tmp_buf);

				ਜੇ (ret != 1) {
					ਲਿਖੋ("%s: read error (%ld, %lld)\n",
					       __func__, ret, ਸ਼ੁਰੂ_sector);
					io_ret = AVB_IO_ਨਤੀਜਾ_ERROR_IO;
					ਜਾਓ ਬਾਹਰਲਾ;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (ਸ਼ੁਰੂ_ਵਿੱਥ % part->info.blksz);
				ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ, (ਖਾਲੀ *)tmp_buf, residue);
			} ਨਹੀਂਤਾਂ {
				ret = mmc_read_and_flush(part,
							 part->info.ਸ਼ੁਰੂ +
							 ਸ਼ੁਰੂ_sector,
							 1, tmp_buf);

				ਜੇ (ret != 1) {
					ਲਿਖੋ("%s: read error (%ld, %lld)\n",
					       __func__, ret, ਸ਼ੁਰੂ_sector);
					io_ret = AVB_IO_ਨਤੀਜਾ_ERROR_IO;
					ਜਾਓ ਬਾਹਰਲਾ;
				}
				ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)tmp_buf +
					ਸ਼ੁਰੂ_ਵਿੱਥ % part->info.blksz,
					ਬਫਰ, residue);

				ret = mmc_write(part, part->info.ਸ਼ੁਰੂ +
						ਸ਼ੁਰੂ_sector, 1, tmp_buf);
				ਜੇ (ret != 1) {
					ਲਿਖੋ("%s: write error (%ld, %lld)\n",
					       __func__, ret, ਸ਼ੁਰੂ_sector);
					io_ret = AVB_IO_ਨਤੀਜਾ_ERROR_IO;
					ਜਾਓ ਬਾਹਰਲਾ;
				}
			}

			io_cnt += residue;
			ਬਫਰ += residue;
			ਸ਼ੁਰੂ_ਵਿੱਥ += residue;
			ਅੰਕ_ਬਾਈਟਾਂ -= residue;
			ਜਾਰੀ;
		}

		ਜੇ (sectors) {
			ਜੇ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.ਸ਼ੁਰੂ +
							 ਸ਼ੁਰੂ_sector,
							 sectors, ਬਫਰ);
			} ਨਹੀਂਤਾਂ {
				ret = mmc_write(part,
						part->info.ਸ਼ੁਰੂ +
						ਸ਼ੁਰੂ_sector,
						sectors, ਬਫਰ);
			}

			ਜੇ (!ret) {
				ਲਿਖੋ("%s: sector read error\n", __func__);
				io_ret = AVB_IO_ਨਤੀਜਾ_ERROR_IO;
				ਜਾਓ ਬਾਹਰਲਾ;
			}

			io_cnt += ret * part->info.blksz;
			ਬਫਰ += ret * part->info.blksz;
			ਸ਼ੁਰੂ_ਵਿੱਥ += ret * part->info.blksz;
			ਅੰਕ_ਬਾਈਟਾਂ -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	ਜੇ (io_type == IO_READ && ਬਾਹਰਲਾ_ਅੰਕ_read)
		*ਬਾਹਰਲਾ_ਅੰਕ_read = io_cnt;

ਬਾਹਰਲਾ:
	ਮੁਕਤ(part);
	ਵਾਪਸ io_ret;
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
ਸਥਿਰ AvbIOResult read_from_ਹਿੱਸੇ_ਕਰੋ(AvbOps *ops,
				       ਅਚਲ ਅੱਖਰ *ਹਿੱਸੇ_ਕਰੋ_name,
				       s64 ਵਿੱਥ_from_ਹਿੱਸੇ_ਕਰੋ,
				       size_t ਅੰਕ_ਬਾਈਟਾਂ,
				       ਖਾਲੀ *ਬਫਰ,
				       size_t *ਬਾਹਰਲਾ_ਅੰਕ_read)
{
	ਵਾਪਸ mmc_byte_io(ops, ਹਿੱਸੇ_ਕਰੋ_name, ਵਿੱਥ_from_ਹਿੱਸੇ_ਕਰੋ,
			   ਅੰਕ_ਬਾਈਟਾਂ, ਬਫਰ, ਬਾਹਰਲਾ_ਅੰਕ_read, IO_READ);
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
ਸਥਿਰ AvbIOResult write_ਮੰਜ਼ਿਲ_ਹਿੱਸੇ_ਕਰੋ(AvbOps *ops,
				      ਅਚਲ ਅੱਖਰ *ਹਿੱਸੇ_ਕਰੋ_name,
				      s64 ਵਿੱਥ_from_ਹਿੱਸੇ_ਕਰੋ,
				      size_t ਅੰਕ_ਬਾਈਟਾਂ,
				      ਅਚਲ ਖਾਲੀ *ਬਫਰ)
{
	ਵਾਪਸ mmc_byte_io(ops, ਹਿੱਸੇ_ਕਰੋ_name, ਵਿੱਥ_from_ਹਿੱਸੇ_ਕਰੋ,
			   ਅੰਕ_ਬਾਈਟਾਂ, (ਖਾਲੀ *)ਬਫਰ, ਨਿਰਾ, IO_WRITE);
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
ਸਥਿਰ AvbIOResult validate_vbmeta_ਜਨਤਕ_ਕੁੰਜੀ(AvbOps *ops,
					      ਅਚਲ u8 *ਜਨਤਕ_ਕੁੰਜੀ_ਅੰਕੜੇ,
					      size_t ਜਨਤਕ_ਕੁੰਜੀ_ਲੰਮਾਈ,
					      ਅਚਲ u8
					      *ਜਨਤਕ_ਕੁੰਜੀ_metadata,
					      size_t
					      ਜਨਤਕ_ਕੁੰਜੀ_metadata_ਲੰਮਾਈ,
					      bool *ਬਾਹਰਲਾ_ਕੁੰਜੀ_is_trusted)
{
	u8 ਕੁੰਜੀ_digest[SHA256_ਜੋੜ_LEN];
	u8 trusted_digest[SHA256_ਜੋੜ_LEN];
	AvbIOResult rc;

	ਜੇ (!ਜਨਤਕ_ਕੁੰਜੀ_ਲੰਮਾਈ || !ਜਨਤਕ_ਕੁੰਜੀ_ਅੰਕੜੇ || !ਬਾਹਰਲਾ_ਕੁੰਜੀ_is_trusted)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*ਬਾਹਰਲਾ_ਕੁੰਜੀ_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(ਜਨਤਕ_ਕੁੰਜੀ_ਅੰਕੜੇ, ਜਨਤਕ_ਕੁੰਜੀ_ਲੰਮਾਈ, ਕੁੰਜੀ_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_ਜੜ੍ਹ_ਕੁੰਜੀ_digest(ops, trusted_digest);
	ਜੇ (rc != AVB_IO_ਨਤੀਜਾ_OK)
		ਵਾਪਸ rc;

	ਜੇ (avb_ਸੁਰੱਖਿਅਤ_memcmp(ਕੁੰਜੀ_digest, trusted_digest, ਆਕਾਰ(ਕੁੰਜੀ_digest)) == 0)
		*ਬਾਹਰਲਾ_ਕੁੰਜੀ_is_trusted = true;

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OPTEE_TA_AVB
ਸਥਿਰ ਪੂਰਨ get_open_session(ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *ops_ਅੰਕੜੇ)
{
	ਬਣਤਰ udevice *tee = ਨਿਰਾ;

	ਜਦੋਂ (!ops_ਅੰਕੜੇ->tee) {
		ਅਚਲ ਬਣਤਰ tee_optee_ta_uuid uuid = TA_AVB_UUID;
		ਬਣਤਰ tee_open_session_arg arg;
		ਪੂਰਨ rc;

		tee = tee_ਲੱਭੋ_device(tee, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ);
		ਜੇ (!tee)
			ਵਾਪਸ -ENODEV;

		memset(&arg, 0, ਆਕਾਰ(arg));
		tee_optee_ta_uuid_ਮੰਜ਼ਿਲ_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, ਨਿਰਾ);
		ਜੇ (rc || arg.ret)
			ਜਾਰੀ;

		ops_ਅੰਕੜੇ->tee = tee;
		ops_ਅੰਕੜੇ->session = arg.session;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ AvbIOResult invoke_func(ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *ops_ਅੰਕੜੇ, u32 func,
			       ulong ਅੰਕ_param, ਬਣਤਰ tee_param *param)
{
	ਬਣਤਰ tee_invoke_arg arg;

	ਜੇ (get_open_session(ops_ਅੰਕੜੇ))
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;

	memset(&arg, 0, ਆਕਾਰ(arg));
	arg.func = func;
	arg.session = ops_ਅੰਕੜੇ->session;

	ਜੇ (tee_invoke_func(ops_ਅੰਕੜੇ->tee, &arg, ਅੰਕ_param, param))
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;
	ਚੋਣ (arg.ret) {
	ਹਾਲਤ TEE_SUCCESS:
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
	ਹਾਲਤ TEE_ERROR_ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ:
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_OOM;
	ਹਾਲਤ TEE_ERROR_STORAGE_NO_SPACE:
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_INSUFFICIENT_SPACE;
	ਹਾਲਤ TEE_ERROR_ਵਸਤੂ_NOT_ਮਿਲਿਆ:
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਮੁੱਲ;
	ਹਾਲਤ TEE_ERROR_ਨਿਸ਼ਾਨਾ_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_ਅੰਕੜੇ->tee, ops_ਅੰਕੜੇ->session);
		ops_ਅੰਕੜੇ->tee = ਨਿਰਾ;
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;
	ਮੂਲ:
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;
	}
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ AvbIOResult read_rollback_ਸੂਚਕਾਂਕ(AvbOps *ops,
				       size_t rollback_ਸੂਚਕਾਂਕ_ਥਾਂ,
				       u64 *ਬਾਹਰਲਾ_rollback_ਸੂਚਕਾਂਕ)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ਜੇ (ਬਾਹਰਲਾ_rollback_ਸੂਚਕਾਂਕ)
		*ਬਾਹਰਲਾ_rollback_ਸੂਚਕਾਂਕ = 0;

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
#ਨਹੀਂਤਾਂ
	AvbIOResult rc;
	ਬਣਤਰ tee_param param[2];

	ਜੇ (rollback_ਸੂਚਕਾਂਕ_ਥਾਂ >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਮੁੱਲ;

	memset(param, 0, ਆਕਾਰ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ਮੁੱਲ_ਨਿਵੇਸ਼;
	param[0].u.ਮੁੱਲ.a = rollback_ਸੂਚਕਾਂਕ_ਥਾਂ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ਮੁੱਲ_OUTPUT;

	rc = invoke_func(ops->user_ਅੰਕੜੇ, TA_AVB_CMD_READ_ROLLBACK_ਸੂਚਕਾਂਕ,
			 ARRAY_ਮਾਪ(param), param);
	ਜੇ (rc)
		ਵਾਪਸ rc;

	*ਬਾਹਰਲਾ_rollback_ਸੂਚਕਾਂਕ = (u64)param[1].u.ਮੁੱਲ.a << 32 |
			      (u32)param[1].u.ਮੁੱਲ.b;
	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ AvbIOResult write_rollback_ਸੂਚਕਾਂਕ(AvbOps *ops,
					size_t rollback_ਸੂਚਕਾਂਕ_ਥਾਂ,
					u64 rollback_ਸੂਚਕਾਂਕ)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
#ਨਹੀਂਤਾਂ
	ਬਣਤਰ tee_param param[2];

	ਜੇ (rollback_ਸੂਚਕਾਂਕ_ਥਾਂ >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਮੁੱਲ;

	memset(param, 0, ਆਕਾਰ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ਮੁੱਲ_ਨਿਵੇਸ਼;
	param[0].u.ਮੁੱਲ.a = rollback_ਸੂਚਕਾਂਕ_ਥਾਂ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ਮੁੱਲ_ਨਿਵੇਸ਼;
	param[1].u.ਮੁੱਲ.a = (u32)(rollback_ਸੂਚਕਾਂਕ >> 32);
	param[1].u.ਮੁੱਲ.b = (u32)rollback_ਸੂਚਕਾਂਕ;

	ਵਾਪਸ invoke_func(ops->user_ਅੰਕੜੇ, TA_AVB_CMD_WRITE_ROLLBACK_ਸੂਚਕਾਂਕ,
			   ARRAY_ਮਾਪ(param), param);
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *ਬਾਹਰਲਾ_is_unlocked)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*ਬਾਹਰਲਾ_is_unlocked = true;

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
#ਨਹੀਂਤਾਂ
	AvbIOResult rc;
	ਬਣਤਰ tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_ਮੁੱਲ_OUTPUT };

	rc = invoke_func(ops->user_ਅੰਕੜੇ, TA_AVB_CMD_READ_ਤਾਲਾ_STATE, 1, &param);
	ਜੇ (rc)
		ਵਾਪਸ rc;
	*ਬਾਹਰਲਾ_is_unlocked = !param.u.ਮੁੱਲ.a;
	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ AvbIOResult get_unique_guid_for_ਹਿੱਸੇ_ਕਰੋ(AvbOps *ops,
						 ਅਚਲ ਅੱਖਰ *ਹਿੱਸੇ_ਕਰੋ,
						 ਅੱਖਰ *guid_buf,
						 size_t guid_buf_ਮਾਪ)
{
	ਬਣਤਰ mmc_part *part;
	size_t uuid_ਮਾਪ;

	part = get_ਹਿੱਸੇ_ਕਰੋ(ops, ਹਿੱਸੇ_ਕਰੋ);
	ਜੇ (!part)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਹਿੱਸੇ_ਕਰੋ;

	uuid_ਮਾਪ = ਆਕਾਰ(part->info.uuid);
	ਜੇ (uuid_ਮਾਪ > guid_buf_ਮਾਪ) {
		ਮੁਕਤ(part);
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(guid_buf, part->info.uuid, uuid_ਮਾਪ);
	guid_buf[uuid_ਮਾਪ - 1] = 0;

	ਮੁਕਤ(part);
	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
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
ਸਥਿਰ AvbIOResult get_ਮਾਪ_ਦਾ_ਹਿੱਸੇ_ਕਰੋ(AvbOps *ops,
					 ਅਚਲ ਅੱਖਰ *ਹਿੱਸੇ_ਕਰੋ,
					 u64 *ਬਾਹਰਲਾ_ਮਾਪ_ਅੰਕ_ਬਾਈਟਾਂ)
{
	ਬਣਤਰ mmc_part *part;

	ਜੇ (!ਬਾਹਰਲਾ_ਮਾਪ_ਅੰਕ_ਬਾਈਟਾਂ)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_INSUFFICIENT_SPACE;

	part = get_ਹਿੱਸੇ_ਕਰੋ(ops, ਹਿੱਸੇ_ਕਰੋ);
	ਜੇ (!part)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਹਿੱਸੇ_ਕਰੋ;

	*ਬਾਹਰਲਾ_ਮਾਪ_ਅੰਕ_ਬਾਈਟਾਂ = part->info.blksz * part->info.ਮਾਪ;
	ਮੁਕਤ(part);

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OPTEE_TA_AVB
ਸਥਿਰ AvbIOResult read_persistent_ਮੁੱਲ(AvbOps *ops,
					 ਅਚਲ ਅੱਖਰ *name,
					 size_t ਬਫਰ_ਮਾਪ,
					 u8 *ਬਾਹਰਲਾ_ਬਫਰ,
					 size_t *ਬਾਹਰਲਾ_ਅੰਕ_ਬਾਈਟਾਂ_read)
{
	AvbIOResult rc;
	ਬਣਤਰ tee_shm *shm_name;
	ਬਣਤਰ tee_shm *shm_buf;
	ਬਣਤਰ tee_param param[2];
	ਬਣਤਰ udevice *tee;
	size_t name_ਮਾਪ = ਲੜੀ_ਲੰਬਾਈ(name) + 1;

	ਜੇ (get_open_session(ops->user_ਅੰਕੜੇ))
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;

	tee = ((ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *)ops->user_ਅੰਕੜੇ)->tee;

	rc = tee_shm_alloc(tee, name_ਮਾਪ,
			   TEE_SHM_ALLOC, &shm_name);
	ਜੇ (rc)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_OOM;

	rc = tee_shm_alloc(tee, ਬਫਰ_ਮਾਪ,
			   TEE_SHM_ALLOC, &shm_buf);
	ਜੇ (rc) {
		rc = AVB_IO_ਨਤੀਜਾ_ERROR_OOM;
		ਜਾਓ free_name;
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(shm_name->addr, name, name_ਮਾਪ);

	memset(param, 0, ਆਕਾਰ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ਨਿਵੇਸ਼;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.ਮਾਪ = name_ਮਾਪ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.ਮਾਪ = ਬਫਰ_ਮਾਪ;

	rc = invoke_func(ops->user_ਅੰਕੜੇ, TA_AVB_CMD_READ_PERSIST_ਮੁੱਲ,
			 2, param);
	ਜੇ (rc)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (param[1].u.memref.ਮਾਪ > ਬਫਰ_ਮਾਪ) {
		rc = AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਮੁੱਲ;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	*ਬਾਹਰਲਾ_ਅੰਕ_ਬਾਈਟਾਂ_read = param[1].u.memref.ਮਾਪ;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_ਬਫਰ, shm_buf->addr, *ਬਾਹਰਲਾ_ਅੰਕ_ਬਾਈਟਾਂ_read);

ਬਾਹਰਲਾ:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ਵਾਪਸ rc;
}

ਸਥਿਰ AvbIOResult write_persistent_ਮੁੱਲ(AvbOps *ops,
					  ਅਚਲ ਅੱਖਰ *name,
					  size_t ਮੁੱਲ_ਮਾਪ,
					  ਅਚਲ u8 *ਮੁੱਲ)
{
	AvbIOResult rc;
	ਬਣਤਰ tee_shm *shm_name;
	ਬਣਤਰ tee_shm *shm_buf;
	ਬਣਤਰ tee_param param[2];
	ਬਣਤਰ udevice *tee;
	size_t name_ਮਾਪ = ਲੜੀ_ਲੰਬਾਈ(name) + 1;

	ਜੇ (get_open_session(ops->user_ਅੰਕੜੇ))
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_IO;

	tee = ((ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *)ops->user_ਅੰਕੜੇ)->tee;

	ਜੇ (!ਮੁੱਲ_ਮਾਪ)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਮੁੱਲ;

	rc = tee_shm_alloc(tee, name_ਮਾਪ,
			   TEE_SHM_ALLOC, &shm_name);
	ਜੇ (rc)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_OOM;

	rc = tee_shm_alloc(tee, ਮੁੱਲ_ਮਾਪ,
			   TEE_SHM_ALLOC, &shm_buf);
	ਜੇ (rc) {
		rc = AVB_IO_ਨਤੀਜਾ_ERROR_OOM;
		ਜਾਓ free_name;
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(shm_name->addr, name, name_ਮਾਪ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(shm_buf->addr, ਮੁੱਲ, ਮੁੱਲ_ਮਾਪ);

	memset(param, 0, ਆਕਾਰ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ਨਿਵੇਸ਼;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.ਮਾਪ = name_ਮਾਪ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ਨਿਵੇਸ਼;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.ਮਾਪ = ਮੁੱਲ_ਮਾਪ;

	rc = invoke_func(ops->user_ਅੰਕੜੇ, TA_AVB_CMD_WRITE_PERSIST_ਮੁੱਲ,
			 2, param);
	ਜੇ (rc)
		ਜਾਓ ਬਾਹਰਲਾ;

ਬਾਹਰਲਾ:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#ਜੇ CONFIG_IS_ENABLED(AVB_ਜੜ੍ਹ_ਕੁੰਜੀ_BUILTIN)
AvbIOResult avb_read_ਜੜ੍ਹ_ਕੁੰਜੀ_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_ਜੜ੍ਹ_pub, ਆਕਾਰ(avb_ਜੜ੍ਹ_pub), digest,
		       CHUNKSZ_SHA256);

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
}
#ਜਾਂਜੇ CONFIG_IS_ENABLED(AVB_ਜੜ੍ਹ_ਕੁੰਜੀ_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_ਜੜ੍ਹ_ਕੁੰਜੀ_digest(AvbOps *ops, uint8_t *digest)
{
	size_t ਅੰਕ_read = 0;
	AvbIOResult rc;

	rc = read_persistent_ਮੁੱਲ(ops, CONFIG_AVB_ਜੜ੍ਹ_ਕੁੰਜੀ_TEE_NAME,
				   SHA256_ਜੋੜ_LEN, digest, &ਅੰਕ_read);
	ਜੇ (rc != AVB_IO_ਨਤੀਜਾ_OK)
		ਵਾਪਸ rc;

	ਜੇ (ਅੰਕ_read != SHA256_ਜੋੜ_LEN)
		ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_INVALID_ਮੁੱਲ_ਮਾਪ;

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_OK;
}
#ਨਹੀਂਤਾਂ /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_ਜੜ੍ਹ_ਕੁੰਜੀ_digest(AvbOps *ops, uint8_t *digest)
{
	ਲਿਖੋ("%s: board root key provider not implemented\n", __func__);

	ਵਾਪਸ AVB_IO_ਨਤੀਜਾ_ERROR_NO_SUCH_ਮੁੱਲ;
}
#ਜੇ_ਅੰਤ

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(ਪੂਰਨ boot_device)
{
	ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *ops_ਅੰਕੜੇ;

	ops_ਅੰਕੜੇ = avb_calloc(ਆਕਾਰ(ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ));
	ਜੇ (!ops_ਅੰਕੜੇ)
		ਵਾਪਸ ਨਿਰਾ;

	ops_ਅੰਕੜੇ->ops.user_ਅੰਕੜੇ = ops_ਅੰਕੜੇ;

	ops_ਅੰਕੜੇ->ops.read_from_ਹਿੱਸੇ_ਕਰੋ = read_from_ਹਿੱਸੇ_ਕਰੋ;
	ops_ਅੰਕੜੇ->ops.write_ਮੰਜ਼ਿਲ_ਹਿੱਸੇ_ਕਰੋ = write_ਮੰਜ਼ਿਲ_ਹਿੱਸੇ_ਕਰੋ;
	ops_ਅੰਕੜੇ->ops.validate_vbmeta_ਜਨਤਕ_ਕੁੰਜੀ = validate_vbmeta_ਜਨਤਕ_ਕੁੰਜੀ;
	ops_ਅੰਕੜੇ->ops.read_rollback_ਸੂਚਕਾਂਕ = read_rollback_ਸੂਚਕਾਂਕ;
	ops_ਅੰਕੜੇ->ops.write_rollback_ਸੂਚਕਾਂਕ = write_rollback_ਸੂਚਕਾਂਕ;
	ops_ਅੰਕੜੇ->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_ਅੰਕੜੇ->ops.get_unique_guid_for_ਹਿੱਸੇ_ਕਰੋ =
		get_unique_guid_for_ਹਿੱਸੇ_ਕਰੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OPTEE_TA_AVB
	ops_ਅੰਕੜੇ->ops.write_persistent_ਮੁੱਲ = write_persistent_ਮੁੱਲ;
	ops_ਅੰਕੜੇ->ops.read_persistent_ਮੁੱਲ = read_persistent_ਮੁੱਲ;
#ਜੇ_ਅੰਤ
	ops_ਅੰਕੜੇ->ops.get_ਮਾਪ_ਦਾ_ਹਿੱਸੇ_ਕਰੋ = get_ਮਾਪ_ਦਾ_ਹਿੱਸੇ_ਕਰੋ;
	ops_ਅੰਕੜੇ->mmc_dev = boot_device;

	ਵਾਪਸ &ops_ਅੰਕੜੇ->ops;
}

ਖਾਲੀ avb_ops_free(AvbOps *ops)
{
	ਬਣਤਰ Avb_Ops_ਅੰਕੜੇ *ops_ਅੰਕੜੇ;

	ਜੇ (!ops)
		ਵਾਪਸ;

	ops_ਅੰਕੜੇ = ops->user_ਅੰਕੜੇ;

	ਜੇ (ops_ਅੰਕੜੇ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OPTEE_TA_AVB
		ਜੇ (ops_ਅੰਕੜੇ->tee)
			tee_close_session(ops_ਅੰਕੜੇ->tee, ops_ਅੰਕੜੇ->session);
#ਜੇ_ਅੰਤ
		avb_free(ops_ਅੰਕੜੇ);
	}
}
