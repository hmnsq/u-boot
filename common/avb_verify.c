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

#ಆದರೆ CONFIG_IS_ENABLED(AVB_ಬೇರು_ಕೀಲಿ_BUILTIN)
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ avb_ಬೇರು_pub[1032] = {
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
#ಆದರೆ_ಅಂತ್ಯ /* AVB_ROOT_KEY_BUILTIN */

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str_avb_io_error(AvbIOResult res)
{
	ಆಯ್ಕೆ (res) {
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_OK:
		ಹಿಂತಿರುಗಿ "Requested operation was successful";
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO:
		ಹಿಂತಿರುಗಿ "Underlying hardware encountered an I/O error";
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_OOM:
		ಹಿಂತಿರುಗಿ "Unable to allocate memory";
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ವಿಭಜನೆ:
		ಹಿಂತಿರುಗಿ "Requested partition does not exist";
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_RANGE_OUTSIDE_ವಿಭಜನೆ:
		ಹಿಂತಿರುಗಿ "Bytes requested is outside the range of partition";
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ಮೌಲ್ಯ:
		ಹಿಂತಿರುಗಿ "Named persistent value does not exist";
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_INVALID_ಮೌಲ್ಯ_ಗಾತ್ರ:
		ಹಿಂತಿರುಗಿ "Named persistent value size is not supported";
	ಸ್ಥಿತಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_INSUFFICIENT_SPACE:
		ಹಿಂತಿರುಗಿ "Buffer is too small for the requested operation";
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ "Unknown AVB error";
	}
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str_avb_ಸ್ಥಾನ_error(Avb_ಸ್ಥಾನ_Verify_ಫಲಿತಾಂಶ res)
{
	ಆಯ್ಕೆ (res) {
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_OK:
		ಹಿಂತಿರುಗಿ "Verification passed successfully";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_OOM:
		ಹಿಂತಿರುಗಿ "Allocation of memory failed";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_IO:
		ಹಿಂತಿರುಗಿ "I/O error occurred while trying to load data";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_VERIFICATION:
		ಹಿಂತಿರುಗಿ "Digest didn't match or signature checks failed";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_ROLLBACK_ಸೂಚ್ಯಂಕ:
		ಹಿಂತಿರುಗಿ "Rollback index is less than its stored value";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_ಬಹಿರಂಗ_ಕೀಲಿ_REJECTED:
		ಹಿಂತಿರುಗಿ "Public keys are not accepted";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_INVALID_METADATA:
		ಹಿಂತಿರುಗಿ "Metadata is invalid or inconsistent";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_UNSUPPORTED_ಆವೃತ್ತಿ:
		ಹಿಂತಿರುಗಿ "Metadata requires a newer version of libavb";
	ಸ್ಥಿತಿ AVB_ಸ್ಥಾನ_VERIFY_ಫಲಿತಾಂಶ_ERROR_INVALID_ವಾದ:
		ಹಿಂತಿರುಗಿ "Invalid arguments are used";
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
ಅಕ್ಷರ *avb_ಗಣ_state(AvbOps *ops, ಎಣಿಕೆ avb_boot_state boot_state)
{
	ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	ಅಕ್ಷರ *cmdline = ಖಾಲಿ;

	ಆದರೆ (!ops)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ದತ್ತಾಂಶ = (ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *)ops->user_ದತ್ತಾಂಶ;
	ಆದರೆ (!ದತ್ತಾಂಶ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ದತ್ತಾಂಶ->boot_state = boot_state;
	ಆಯ್ಕೆ (boot_state) {
	ಸ್ಥಿತಿ AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ AVB_RED:
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ cmdline;
}

ಅಕ್ಷರ *append_cmd_ಸಾಲು(ಅಕ್ಷರ *cmdline_orig, ಅಕ್ಷರ *cmdline_new)
{
	ಅಕ್ಷರ *cmd_ಸಾಲು;

	ಆದರೆ (!cmdline_new)
		ಹಿಂತಿರುಗಿ cmdline_orig;

	ಆದರೆ (cmdline_orig)
		cmd_ಸಾಲು = cmdline_orig;
	ಇಲ್ಲದಿದ್ದರೆ
		cmd_ಸಾಲು = " ";

	cmd_ಸಾಲು = avb_strdupv(cmd_ಸಾಲು, " ", cmdline_new, ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ cmd_ಸಾಲು;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ avb_ಕಂಡುಹಿಡಿ_dm_ವಾದಗಳು(ಅಕ್ಷರ **ವಾದಗಳು, ಅಕ್ಷರ *str)
{
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ -1;

	ಗಾಗಿ (i = 0; i < AVB_MAX_ವಾದಗಳು && ವಾದಗಳು[i]; ++i) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ವಾದಗಳು[i], str))
			ಹಿಂತಿರುಗಿ i;
	}

	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಅಕ್ಷರ *avb_ಗಣ_enforce_option(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmdline, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *option)
{
	ಅಕ್ಷರ *cmdarg[AVB_MAX_ವಾದಗಳು];
	ಅಕ್ಷರ *newargs = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ i = 0;
	ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_ವಾದಗಳು;

	memset(cmdarg, 0, ಗಾತ್ರ(cmdarg));
	cmdarg[i++] = strtok((ಅಕ್ಷರ *)cmdline, " ");

	ಮಾಡು {
		cmdarg[i] = strtok(ಖಾಲಿ, " ");
		ಆದರೆ (!cmdarg[i])
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (++i >= AVB_MAX_ವಾದಗಳು) {
			ಬರೆ("%s: Can't handle more then %d args\n",
			       __func__, i);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	} ಆಗ (true);

	ಒಟ್ಟು_ವಾದಗಳು = i;
	i = avb_ಕಂಡುಹಿಡಿ_dm_ವಾದಗಳು(&cmdarg[0], VERITY_ಕೋಷ್ಟಕ_OPT_LOGGING);
	ಆದರೆ (i >= 0) {
		cmdarg[i] = (ಅಕ್ಷರ *)option;
	} ಇಲ್ಲದಿದ್ದರೆ {
		i = avb_ಕಂಡುಹಿಡಿ_dm_ವಾದಗಳು(&cmdarg[0], VERITY_ಕೋಷ್ಟಕ_OPT_RESTART);
		ಆದರೆ (i < 0) {
			ಬರೆ("%s: No verity options found\n", __func__);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		cmdarg[i] = (ಅಕ್ಷರ *)option;
	}

	ಗಾಗಿ (i = 0; i <= ಒಟ್ಟು_ವಾದಗಳು; i++)
		newargs = append_cmd_ಸಾಲು(newargs, cmdarg[i]);

	ಹಿಂತಿರುಗಿ newargs;
}

ಅಕ್ಷರ *avb_ಗಣ_ignore_corruption(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmdline)
{
	ಅಕ್ಷರ *newargs = ಖಾಲಿ;

	newargs = avb_ಗಣ_enforce_option(cmdline, VERITY_ಕೋಷ್ಟಕ_OPT_LOGGING);
	ಆದರೆ (newargs)
		newargs = append_cmd_ಸಾಲು(newargs,
					  "androidboot.veritymode=eio");

	ಹಿಂತಿರುಗಿ newargs;
}

ಅಕ್ಷರ *avb_ಗಣ_enforce_verity(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmdline)
{
	ಅಕ್ಷರ *newargs;

	newargs = avb_ಗಣ_enforce_option(cmdline, VERITY_ಕೋಷ್ಟಕ_OPT_RESTART);
	ಆದರೆ (newargs)
		newargs = append_cmd_ಸಾಲು(newargs,
					  "androidboot.veritymode=enforcing");
	ಹಿಂತಿರುಗಿ newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ mmc_read_and_flush(ರಚನೆ mmc_part *part,
					lbaint_t ಆರಂಭ,
					lbaint_t sectors,
					ಶೂನ್ಯ *ಬಫರ್)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ blks;
	ಶೂನ್ಯ *tmp_buf;
	size_t buf_ಗಾತ್ರ;
	bool unaligned = is_buf_unaligned(ಬಫರ್);

	ಆದರೆ (ಆರಂಭ < part->info.ಆರಂಭ) {
		ಬರೆ("%s: partition start out of bounds\n", __func__);
		ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ ((ಆರಂಭ + sectors) > (part->info.ಆರಂಭ + part->info.ಗಾತ್ರ_2)) {
		sectors = part->info.ಆರಂಭ + part->info.ಗಾತ್ರ_2 - ಆರಂಭ;
		ಬರೆ("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	ಆದರೆ (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, ಬಫರ್);
		tmp_buf = get_sector_buf();
		buf_ಗಾತ್ರ = get_sector_buf_ಗಾತ್ರ();
		ಆದರೆ (sectors > buf_ಗಾತ್ರ / part->info.blksz)
			sectors = buf_ಗಾತ್ರ / part->info.blksz;
	} ಇಲ್ಲದಿದ್ದರೆ {
		tmp_buf = ಬಫರ್;
	}

	blks = blk_dread(part->mmc_blk,
			 ಆರಂಭ, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	ಆದರೆ (unaligned)
		ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್, tmp_buf, sectors * part->info.blksz);

	ಹಿಂತಿರುಗಿ blks;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ mmc_write(ರಚನೆ mmc_part *part, lbaint_t ಆರಂಭ,
			       lbaint_t sectors, ಶೂನ್ಯ *ಬಫರ್)
{
	ಶೂನ್ಯ *tmp_buf;
	size_t buf_ಗಾತ್ರ;
	bool unaligned = is_buf_unaligned(ಬಫರ್);

	ಆದರೆ (ಆರಂಭ < part->info.ಆರಂಭ) {
		ಬರೆ("%s: partition start out of bounds\n", __func__);
		ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ ((ಆರಂಭ + sectors) > (part->info.ಆರಂಭ + part->info.ಗಾತ್ರ_2)) {
		sectors = part->info.ಆರಂಭ + part->info.ಗಾತ್ರ_2 - ಆರಂಭ;
		ಬರೆ("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	ಆದರೆ (unaligned) {
		tmp_buf = get_sector_buf();
		buf_ಗಾತ್ರ = get_sector_buf_ಗಾತ್ರ();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, ಬಫರ್);
		ಆದರೆ (sectors > buf_ಗಾತ್ರ / part->info.blksz)
			sectors = buf_ಗಾತ್ರ / part->info.blksz;

		ಸ್ಮರಣೆ_ನಕಲು(tmp_buf, ಬಫರ್, sectors * part->info.blksz);
	} ಇಲ್ಲದಿದ್ದರೆ {
		tmp_buf = ಬಫರ್;
	}

	ಹಿಂತಿರುಗಿ blk_dwrite(part->mmc_blk,
			  ಆರಂಭ, sectors, tmp_buf);
}

ಸ್ಥಿರ ರಚನೆ mmc_part *get_ವಿಭಜನೆ(AvbOps *ops, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಭಜನೆ)
{
	ಪೂರ್ಣಾಂಕ ret;
	u8 dev_ಅಂಕೆ;
	ಪೂರ್ಣಾಂಕ part_ಅಂಕೆ = 0;
	ರಚನೆ mmc_part *part;
	ರಚನೆ blk_desc *mmc_blk;

	part = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ mmc_part));
	ಆದರೆ (!part)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	dev_ಅಂಕೆ = get_boot_device(ops);
	part->mmc = ಕಂಡುಹಿಡಿ_mmc_device(dev_ಅಂಕೆ);
	ಆದರೆ (!part->mmc) {
		ಬರೆ("%s: no MMC device at slot %x\n", __func__, dev_ಅಂಕೆ);
		ಹೋಗು err;
	}

	ret = mmc_init(part->mmc);
	ಆದರೆ (ret) {
		ಬರೆ("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		ಹೋಗು err;
	}

	ಆದರೆ (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_ಅಂಕೆ);
		ಆದರೆ (ret) {
			ಬರೆ("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			ಹೋಗು err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	ಆದರೆ (!mmc_blk) {
		ಬರೆ("%s: failed to obtain block descriptor\n", __func__);
		ಹೋಗು err;
	}

	ret = part_get_info_by_name(mmc_blk, ವಿಭಜನೆ, &part->info);
	ಆದರೆ (ret < 0) {
		ಬರೆ("%s: can't find partition '%s'\n", __func__, ವಿಭಜನೆ);
		ಹೋಗು err;
	}

	part->dev_ಅಂಕೆ = dev_ಅಂಕೆ;
	part->mmc_blk = mmc_blk;

	ಹಿಂತಿರುಗಿ part;
err:
	ಬಿಡುಗಡೆ(part);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ AvbIOResult mmc_byte_io(AvbOps *ops,
			       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಭಜನೆ,
			       s64 ಸ್ಥಾನಾಂತರ,
			       size_t ಅಂಕೆ_ಬೈಟುಗಳು,
			       ಶೂನ್ಯ *ಬಫರ್,
			       size_t *ಹೊರಹೋಗು_ಅಂಕೆ_read,
			       ಎಣಿಕೆ mmc_io_type io_type)
{
	ulong ret;
	ರಚನೆ mmc_part *part;
	u64 ಆರಂಭ_ಸ್ಥಾನಾಂತರ, ಆರಂಭ_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_ಫಲಿತಾಂಶ_OK;

	ಆದರೆ (!ವಿಭಜನೆ || !ಬಫರ್ || io_type > IO_WRITE)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;

	part = get_ವಿಭಜನೆ(ops, ವಿಭಜನೆ);
	ಆದರೆ (!part)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ವಿಭಜನೆ;

	ಆದರೆ (!part->info.blksz) {
		io_ret = AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆರಂಭ_ಸ್ಥಾನಾಂತರ = calc_ಸ್ಥಾನಾಂತರ(part, ಸ್ಥಾನಾಂತರ);
	ಆಗ (ಅಂಕೆ_ಬೈಟುಗಳು) {
		ಆರಂಭ_sector = ಆರಂಭ_ಸ್ಥಾನಾಂತರ / part->info.blksz;
		sectors = ಅಂಕೆ_ಬೈಟುಗಳು / part->info.blksz;
		/* handle non block-aligned reads */
		ಆದರೆ (ಆರಂಭ_ಸ್ಥಾನಾಂತರ % part->info.blksz ||
		    ಅಂಕೆ_ಬೈಟುಗಳು < part->info.blksz) {
			tmp_buf = get_sector_buf();
			ಆದರೆ (ಆರಂಭ_ಸ್ಥಾನಾಂತರ % part->info.blksz) {
				residue = part->info.blksz -
					(ಆರಂಭ_ಸ್ಥಾನಾಂತರ % part->info.blksz);
				ಆದರೆ (residue > ಅಂಕೆ_ಬೈಟುಗಳು)
					residue = ಅಂಕೆ_ಬೈಟುಗಳು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				residue = ಅಂಕೆ_ಬೈಟುಗಳು;
			}

			ಆದರೆ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.ಆರಂಭ +
							 ಆರಂಭ_sector,
							 1, tmp_buf);

				ಆದರೆ (ret != 1) {
					ಬರೆ("%s: read error (%ld, %lld)\n",
					       __func__, ret, ಆರಂಭ_sector);
					io_ret = AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
					ಹೋಗು ಹೊರಹೋಗು;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (ಆರಂಭ_ಸ್ಥಾನಾಂತರ % part->info.blksz);
				ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್, (ಶೂನ್ಯ *)tmp_buf, residue);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ret = mmc_read_and_flush(part,
							 part->info.ಆರಂಭ +
							 ಆರಂಭ_sector,
							 1, tmp_buf);

				ಆದರೆ (ret != 1) {
					ಬರೆ("%s: read error (%ld, %lld)\n",
					       __func__, ret, ಆರಂಭ_sector);
					io_ret = AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
					ಹೋಗು ಹೊರಹೋಗು;
				}
				ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)tmp_buf +
					ಆರಂಭ_ಸ್ಥಾನಾಂತರ % part->info.blksz,
					ಬಫರ್, residue);

				ret = mmc_write(part, part->info.ಆರಂಭ +
						ಆರಂಭ_sector, 1, tmp_buf);
				ಆದರೆ (ret != 1) {
					ಬರೆ("%s: write error (%ld, %lld)\n",
					       __func__, ret, ಆರಂಭ_sector);
					io_ret = AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
					ಹೋಗು ಹೊರಹೋಗು;
				}
			}

			io_cnt += residue;
			ಬಫರ್ += residue;
			ಆರಂಭ_ಸ್ಥಾನಾಂತರ += residue;
			ಅಂಕೆ_ಬೈಟುಗಳು -= residue;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (sectors) {
			ಆದರೆ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.ಆರಂಭ +
							 ಆರಂಭ_sector,
							 sectors, ಬಫರ್);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ret = mmc_write(part,
						part->info.ಆರಂಭ +
						ಆರಂಭ_sector,
						sectors, ಬಫರ್);
			}

			ಆದರೆ (!ret) {
				ಬರೆ("%s: sector read error\n", __func__);
				io_ret = AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
				ಹೋಗು ಹೊರಹೋಗು;
			}

			io_cnt += ret * part->info.blksz;
			ಬಫರ್ += ret * part->info.blksz;
			ಆರಂಭ_ಸ್ಥಾನಾಂತರ += ret * part->info.blksz;
			ಅಂಕೆ_ಬೈಟುಗಳು -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	ಆದರೆ (io_type == IO_READ && ಹೊರಹೋಗು_ಅಂಕೆ_read)
		*ಹೊರಹೋಗು_ಅಂಕೆ_read = io_cnt;

ಹೊರಹೋಗು:
	ಬಿಡುಗಡೆ(part);
	ಹಿಂತಿರುಗಿ io_ret;
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
ಸ್ಥಿರ AvbIOResult read_from_ವಿಭಜನೆ(AvbOps *ops,
				       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಭಜನೆ_name,
				       s64 ಸ್ಥಾನಾಂತರ_from_ವಿಭಜನೆ,
				       size_t ಅಂಕೆ_ಬೈಟುಗಳು,
				       ಶೂನ್ಯ *ಬಫರ್,
				       size_t *ಹೊರಹೋಗು_ಅಂಕೆ_read)
{
	ಹಿಂತಿರುಗಿ mmc_byte_io(ops, ವಿಭಜನೆ_name, ಸ್ಥಾನಾಂತರ_from_ವಿಭಜನೆ,
			   ಅಂಕೆ_ಬೈಟುಗಳು, ಬಫರ್, ಹೊರಹೋಗು_ಅಂಕೆ_read, IO_READ);
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
ಸ್ಥಿರ AvbIOResult write_ಗಮ್ಯ_ವಿಭಜನೆ(AvbOps *ops,
				      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಭಜನೆ_name,
				      s64 ಸ್ಥಾನಾಂತರ_from_ವಿಭಜನೆ,
				      size_t ಅಂಕೆ_ಬೈಟುಗಳು,
				      ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಬಫರ್)
{
	ಹಿಂತಿರುಗಿ mmc_byte_io(ops, ವಿಭಜನೆ_name, ಸ್ಥಾನಾಂತರ_from_ವಿಭಜನೆ,
			   ಅಂಕೆ_ಬೈಟುಗಳು, (ಶೂನ್ಯ *)ಬಫರ್, ಖಾಲಿ, IO_WRITE);
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
ಸ್ಥಿರ AvbIOResult validate_vbmeta_ಬಹಿರಂಗ_ಕೀಲಿ(AvbOps *ops,
					      ಸ್ಥಿರಾಂಕ u8 *ಬಹಿರಂಗ_ಕೀಲಿ_ದತ್ತಾಂಶ,
					      size_t ಬಹಿರಂಗ_ಕೀಲಿ_ಉದ್ದಳತೆ,
					      ಸ್ಥಿರಾಂಕ u8
					      *ಬಹಿರಂಗ_ಕೀಲಿ_metadata,
					      size_t
					      ಬಹಿರಂಗ_ಕೀಲಿ_metadata_ಉದ್ದಳತೆ,
					      bool *ಹೊರಹೋಗು_ಕೀಲಿ_is_trusted)
{
	u8 ಕೀಲಿ_digest[SHA256_ಮೊತ್ತ_LEN];
	u8 trusted_digest[SHA256_ಮೊತ್ತ_LEN];
	AvbIOResult rc;

	ಆದರೆ (!ಬಹಿರಂಗ_ಕೀಲಿ_ಉದ್ದಳತೆ || !ಬಹಿರಂಗ_ಕೀಲಿ_ದತ್ತಾಂಶ || !ಹೊರಹೋಗು_ಕೀಲಿ_is_trusted)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*ಹೊರಹೋಗು_ಕೀಲಿ_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(ಬಹಿರಂಗ_ಕೀಲಿ_ದತ್ತಾಂಶ, ಬಹಿರಂಗ_ಕೀಲಿ_ಉದ್ದಳತೆ, ಕೀಲಿ_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_ಬೇರು_ಕೀಲಿ_digest(ops, trusted_digest);
	ಆದರೆ (rc != AVB_IO_ಫಲಿತಾಂಶ_OK)
		ಹಿಂತಿರುಗಿ rc;

	ಆದರೆ (avb_ಸುರಕ್ಷಿತ_memcmp(ಕೀಲಿ_digest, trusted_digest, ಗಾತ್ರ(ಕೀಲಿ_digest)) == 0)
		*ಹೊರಹೋಗು_ಕೀಲಿ_is_trusted = true;

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OPTEE_TA_AVB
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_open_session(ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *ops_ದತ್ತಾಂಶ)
{
	ರಚನೆ udevice *tee = ಖಾಲಿ;

	ಆಗ (!ops_ದತ್ತಾಂಶ->tee) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ tee_optee_ta_uuid uuid = TA_AVB_UUID;
		ರಚನೆ tee_open_session_arg arg;
		ಪೂರ್ಣಾಂಕ rc;

		tee = tee_ಕಂಡುಹಿಡಿ_device(tee, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ);
		ಆದರೆ (!tee)
			ಹಿಂತಿರುಗಿ -ENODEV;

		memset(&arg, 0, ಗಾತ್ರ(arg));
		tee_optee_ta_uuid_ಗಮ್ಯ_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, ಖಾಲಿ);
		ಆದರೆ (rc || arg.ret)
			ಮುಂದುವರಿ;

		ops_ದತ್ತಾಂಶ->tee = tee;
		ops_ದತ್ತಾಂಶ->session = arg.session;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ AvbIOResult invoke_func(ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *ops_ದತ್ತಾಂಶ, u32 func,
			       ulong ಅಂಕೆ_param, ರಚನೆ tee_param *param)
{
	ರಚನೆ tee_invoke_arg arg;

	ಆದರೆ (get_open_session(ops_ದತ್ತಾಂಶ))
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;

	memset(&arg, 0, ಗಾತ್ರ(arg));
	arg.func = func;
	arg.session = ops_ದತ್ತಾಂಶ->session;

	ಆದರೆ (tee_invoke_func(ops_ದತ್ತಾಂಶ->tee, &arg, ಅಂಕೆ_param, param))
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
	ಆಯ್ಕೆ (arg.ret) {
	ಸ್ಥಿತಿ TEE_SUCCESS:
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
	ಸ್ಥಿತಿ TEE_ERROR_ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ:
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_OOM;
	ಸ್ಥಿತಿ TEE_ERROR_STORAGE_NO_SPACE:
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_INSUFFICIENT_SPACE;
	ಸ್ಥಿತಿ TEE_ERROR_ಅಂಶ_NOT_ಸಿಕ್ಕಿತು:
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ಮೌಲ್ಯ;
	ಸ್ಥಿತಿ TEE_ERROR_ಗುರಿ_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_ದತ್ತಾಂಶ->tee, ops_ದತ್ತಾಂಶ->session);
		ops_ದತ್ತಾಂಶ->tee = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
	}
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ AvbIOResult read_rollback_ಸೂಚ್ಯಂಕ(AvbOps *ops,
				       size_t rollback_ಸೂಚ್ಯಂಕ_ಸ್ಥಾನ,
				       u64 *ಹೊರಹೋಗು_rollback_ಸೂಚ್ಯಂಕ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ಆದರೆ (ಹೊರಹೋಗು_rollback_ಸೂಚ್ಯಂಕ)
		*ಹೊರಹೋಗು_rollback_ಸೂಚ್ಯಂಕ = 0;

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
#ಇಲ್ಲದಿದ್ದರೆ
	AvbIOResult rc;
	ರಚನೆ tee_param param[2];

	ಆದರೆ (rollback_ಸೂಚ್ಯಂಕ_ಸ್ಥಾನ >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ಮೌಲ್ಯ;

	memset(param, 0, ಗಾತ್ರ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param[0].u.ಮೌಲ್ಯ.a = rollback_ಸೂಚ್ಯಂಕ_ಸ್ಥಾನ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_OUTPUT;

	rc = invoke_func(ops->user_ದತ್ತಾಂಶ, TA_AVB_CMD_READ_ROLLBACK_ಸೂಚ್ಯಂಕ,
			 ARRAY_ಗಾತ್ರ(param), param);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	*ಹೊರಹೋಗು_rollback_ಸೂಚ್ಯಂಕ = (u64)param[1].u.ಮೌಲ್ಯ.a << 32 |
			      (u32)param[1].u.ಮೌಲ್ಯ.b;
	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ AvbIOResult write_rollback_ಸೂಚ್ಯಂಕ(AvbOps *ops,
					size_t rollback_ಸೂಚ್ಯಂಕ_ಸ್ಥಾನ,
					u64 rollback_ಸೂಚ್ಯಂಕ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
#ಇಲ್ಲದಿದ್ದರೆ
	ರಚನೆ tee_param param[2];

	ಆದರೆ (rollback_ಸೂಚ್ಯಂಕ_ಸ್ಥಾನ >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ಮೌಲ್ಯ;

	memset(param, 0, ಗಾತ್ರ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param[0].u.ಮೌಲ್ಯ.a = rollback_ಸೂಚ್ಯಂಕ_ಸ್ಥಾನ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param[1].u.ಮೌಲ್ಯ.a = (u32)(rollback_ಸೂಚ್ಯಂಕ >> 32);
	param[1].u.ಮೌಲ್ಯ.b = (u32)rollback_ಸೂಚ್ಯಂಕ;

	ಹಿಂತಿರುಗಿ invoke_func(ops->user_ದತ್ತಾಂಶ, TA_AVB_CMD_WRITE_ROLLBACK_ಸೂಚ್ಯಂಕ,
			   ARRAY_ಗಾತ್ರ(param), param);
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *ಹೊರಹೋಗು_is_unlocked)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*ಹೊರಹೋಗು_is_unlocked = true;

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
#ಇಲ್ಲದಿದ್ದರೆ
	AvbIOResult rc;
	ರಚನೆ tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_OUTPUT };

	rc = invoke_func(ops->user_ದತ್ತಾಂಶ, TA_AVB_CMD_READ_ಬೀಗ_STATE, 1, &param);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;
	*ಹೊರಹೋಗು_is_unlocked = !param.u.ಮೌಲ್ಯ.a;
	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ AvbIOResult get_unique_guid_for_ವಿಭಜನೆ(AvbOps *ops,
						 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಭಜನೆ,
						 ಅಕ್ಷರ *guid_buf,
						 size_t guid_buf_ಗಾತ್ರ)
{
	ರಚನೆ mmc_part *part;
	size_t uuid_ಗಾತ್ರ;

	part = get_ವಿಭಜನೆ(ops, ವಿಭಜನೆ);
	ಆದರೆ (!part)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ವಿಭಜನೆ;

	uuid_ಗಾತ್ರ = ಗಾತ್ರ(part->info.uuid);
	ಆದರೆ (uuid_ಗಾತ್ರ > guid_buf_ಗಾತ್ರ) {
		ಬಿಡುಗಡೆ(part);
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;
	}

	ಸ್ಮರಣೆ_ನಕಲು(guid_buf, part->info.uuid, uuid_ಗಾತ್ರ);
	guid_buf[uuid_ಗಾತ್ರ - 1] = 0;

	ಬಿಡುಗಡೆ(part);
	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
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
ಸ್ಥಿರ AvbIOResult get_ಗಾತ್ರ_ದ_ವಿಭಜನೆ(AvbOps *ops,
					 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವಿಭಜನೆ,
					 u64 *ಹೊರಹೋಗು_ಗಾತ್ರ_ಅಂಕೆ_ಬೈಟುಗಳು)
{
	ರಚನೆ mmc_part *part;

	ಆದರೆ (!ಹೊರಹೋಗು_ಗಾತ್ರ_ಅಂಕೆ_ಬೈಟುಗಳು)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_INSUFFICIENT_SPACE;

	part = get_ವಿಭಜನೆ(ops, ವಿಭಜನೆ);
	ಆದರೆ (!part)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ವಿಭಜನೆ;

	*ಹೊರಹೋಗು_ಗಾತ್ರ_ಅಂಕೆ_ಬೈಟುಗಳು = part->info.blksz * part->info.ಗಾತ್ರ_2;
	ಬಿಡುಗಡೆ(part);

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OPTEE_TA_AVB
ಸ್ಥಿರ AvbIOResult read_persistent_ಮೌಲ್ಯ(AvbOps *ops,
					 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
					 size_t ಬಫರ್_ಗಾತ್ರ,
					 u8 *ಹೊರಹೋಗು_ಬಫರ್,
					 size_t *ಹೊರಹೋಗು_ಅಂಕೆ_ಬೈಟುಗಳು_read)
{
	AvbIOResult rc;
	ರಚನೆ tee_shm *shm_name;
	ರಚನೆ tee_shm *shm_buf;
	ರಚನೆ tee_param param[2];
	ರಚನೆ udevice *tee;
	size_t name_ಗಾತ್ರ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1;

	ಆದರೆ (get_open_session(ops->user_ದತ್ತಾಂಶ))
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;

	tee = ((ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *)ops->user_ದತ್ತಾಂಶ)->tee;

	rc = tee_shm_alloc(tee, name_ಗಾತ್ರ,
			   TEE_SHM_ALLOC, &shm_name);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_OOM;

	rc = tee_shm_alloc(tee, ಬಫರ್_ಗಾತ್ರ,
			   TEE_SHM_ALLOC, &shm_buf);
	ಆದರೆ (rc) {
		rc = AVB_IO_ಫಲಿತಾಂಶ_ERROR_OOM;
		ಹೋಗು free_name;
	}

	ಸ್ಮರಣೆ_ನಕಲು(shm_name->addr, name, name_ಗಾತ್ರ);

	memset(param, 0, ಗಾತ್ರ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ಒಳಹರಿವು;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.ಗಾತ್ರ_2 = name_ಗಾತ್ರ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.ಗಾತ್ರ_2 = ಬಫರ್_ಗಾತ್ರ;

	rc = invoke_func(ops->user_ದತ್ತಾಂಶ, TA_AVB_CMD_READ_PERSIST_ಮೌಲ್ಯ,
			 2, param);
	ಆದರೆ (rc)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (param[1].u.memref.ಗಾತ್ರ_2 > ಬಫರ್_ಗಾತ್ರ) {
		rc = AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ಮೌಲ್ಯ;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	*ಹೊರಹೋಗು_ಅಂಕೆ_ಬೈಟುಗಳು_read = param[1].u.memref.ಗಾತ್ರ_2;

	ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_ಬಫರ್, shm_buf->addr, *ಹೊರಹೋಗು_ಅಂಕೆ_ಬೈಟುಗಳು_read);

ಹೊರಹೋಗು:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ AvbIOResult write_persistent_ಮೌಲ್ಯ(AvbOps *ops,
					  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
					  size_t ಮೌಲ್ಯ_ಗಾತ್ರ,
					  ಸ್ಥಿರಾಂಕ u8 *ಮೌಲ್ಯ)
{
	AvbIOResult rc;
	ರಚನೆ tee_shm *shm_name;
	ರಚನೆ tee_shm *shm_buf;
	ರಚನೆ tee_param param[2];
	ರಚನೆ udevice *tee;
	size_t name_ಗಾತ್ರ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1;

	ಆದರೆ (get_open_session(ops->user_ದತ್ತಾಂಶ))
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_IO;

	tee = ((ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *)ops->user_ದತ್ತಾಂಶ)->tee;

	ಆದರೆ (!ಮೌಲ್ಯ_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ಮೌಲ್ಯ;

	rc = tee_shm_alloc(tee, name_ಗಾತ್ರ,
			   TEE_SHM_ALLOC, &shm_name);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_OOM;

	rc = tee_shm_alloc(tee, ಮೌಲ್ಯ_ಗಾತ್ರ,
			   TEE_SHM_ALLOC, &shm_buf);
	ಆದರೆ (rc) {
		rc = AVB_IO_ಫಲಿತಾಂಶ_ERROR_OOM;
		ಹೋಗು free_name;
	}

	ಸ್ಮರಣೆ_ನಕಲು(shm_name->addr, name, name_ಗಾತ್ರ);
	ಸ್ಮರಣೆ_ನಕಲು(shm_buf->addr, ಮೌಲ್ಯ, ಮೌಲ್ಯ_ಗಾತ್ರ);

	memset(param, 0, ಗಾತ್ರ(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ಒಳಹರಿವು;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.ಗಾತ್ರ_2 = name_ಗಾತ್ರ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ಒಳಹರಿವು;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.ಗಾತ್ರ_2 = ಮೌಲ್ಯ_ಗಾತ್ರ;

	rc = invoke_func(ops->user_ದತ್ತಾಂಶ, TA_AVB_CMD_WRITE_PERSIST_ಮೌಲ್ಯ,
			 2, param);
	ಆದರೆ (rc)
		ಹೋಗು ಹೊರಹೋಗು;

ಹೊರಹೋಗು:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#ಆದರೆ CONFIG_IS_ENABLED(AVB_ಬೇರು_ಕೀಲಿ_BUILTIN)
AvbIOResult avb_read_ಬೇರು_ಕೀಲಿ_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_ಬೇರು_pub, ಗಾತ್ರ(avb_ಬೇರು_pub), digest,
		       CHUNKSZ_SHA256);

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
}
#ಅಥವಾಆದರೆ CONFIG_IS_ENABLED(AVB_ಬೇರು_ಕೀಲಿ_TEE)
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
				   SHA256_ಮೊತ್ತ_LEN, digest, &ಅಂಕೆ_read);
	ಆದರೆ (rc != AVB_IO_ಫಲಿತಾಂಶ_OK)
		ಹಿಂತಿರುಗಿ rc;

	ಆದರೆ (ಅಂಕೆ_read != SHA256_ಮೊತ್ತ_LEN)
		ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_INVALID_ಮೌಲ್ಯ_ಗಾತ್ರ;

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_OK;
}
#ಇಲ್ಲದಿದ್ದರೆ /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_ಬೇರು_ಕೀಲಿ_digest(AvbOps *ops, uint8_t *digest)
{
	ಬರೆ("%s: board root key provider not implemented\n", __func__);

	ಹಿಂತಿರುಗಿ AVB_IO_ಫಲಿತಾಂಶ_ERROR_NO_SUCH_ಮೌಲ್ಯ;
}
#ಆದರೆ_ಅಂತ್ಯ

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(ಪೂರ್ಣಾಂಕ boot_device)
{
	ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *ops_ದತ್ತಾಂಶ;

	ops_ದತ್ತಾಂಶ = avb_calloc(ಗಾತ್ರ(ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ));
	ಆದರೆ (!ops_ದತ್ತಾಂಶ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ops_ದತ್ತಾಂಶ->ops.user_ದತ್ತಾಂಶ = ops_ದತ್ತಾಂಶ;

	ops_ದತ್ತಾಂಶ->ops.read_from_ವಿಭಜನೆ = read_from_ವಿಭಜನೆ;
	ops_ದತ್ತಾಂಶ->ops.write_ಗಮ್ಯ_ವಿಭಜನೆ = write_ಗಮ್ಯ_ವಿಭಜನೆ;
	ops_ದತ್ತಾಂಶ->ops.validate_vbmeta_ಬಹಿರಂಗ_ಕೀಲಿ = validate_vbmeta_ಬಹಿರಂಗ_ಕೀಲಿ;
	ops_ದತ್ತಾಂಶ->ops.read_rollback_ಸೂಚ್ಯಂಕ = read_rollback_ಸೂಚ್ಯಂಕ;
	ops_ದತ್ತಾಂಶ->ops.write_rollback_ಸೂಚ್ಯಂಕ = write_rollback_ಸೂಚ್ಯಂಕ;
	ops_ದತ್ತಾಂಶ->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_ದತ್ತಾಂಶ->ops.get_unique_guid_for_ವಿಭಜನೆ =
		get_unique_guid_for_ವಿಭಜನೆ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OPTEE_TA_AVB
	ops_ದತ್ತಾಂಶ->ops.write_persistent_ಮೌಲ್ಯ = write_persistent_ಮೌಲ್ಯ;
	ops_ದತ್ತಾಂಶ->ops.read_persistent_ಮೌಲ್ಯ = read_persistent_ಮೌಲ್ಯ;
#ಆದರೆ_ಅಂತ್ಯ
	ops_ದತ್ತಾಂಶ->ops.get_ಗಾತ್ರ_ದ_ವಿಭಜನೆ = get_ಗಾತ್ರ_ದ_ವಿಭಜನೆ;
	ops_ದತ್ತಾಂಶ->mmc_dev = boot_device;

	ಹಿಂತಿರುಗಿ &ops_ದತ್ತಾಂಶ->ops;
}

ಶೂನ್ಯ avb_ops_free(AvbOps *ops)
{
	ರಚನೆ Avb_Ops_ದತ್ತಾಂಶ *ops_ದತ್ತಾಂಶ;

	ಆದರೆ (!ops)
		ಹಿಂತಿರುಗಿ;

	ops_ದತ್ತಾಂಶ = ops->user_ದತ್ತಾಂಶ;

	ಆದರೆ (ops_ದತ್ತಾಂಶ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OPTEE_TA_AVB
		ಆದರೆ (ops_ದತ್ತಾಂಶ->tee)
			tee_close_session(ops_ದತ್ತಾಂಶ->tee, ops_ದತ್ತಾಂಶ->session);
#ಆದರೆ_ಅಂತ್ಯ
		avb_free(ops_ದತ್ತಾಂಶ);
	}
}
