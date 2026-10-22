// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#ضمن <avb_verify.h>
#ضمن <blk.h>
#ضمن <cpu_func.h>
#ضمن <image.h>
#ضمن <malloc.h>
#ضمن <part.h>
#ضمن <tee.h>
#ضمن <tee/optee_ta_avb.h>
#ضمن <u-boot/sha256.h>

#إذا CONFIG_IS_ENABLED(AVB_ریښه_کلی_BUILTIN)
ساكن ثابت بینښه حرف avb_ریښه_pub[1032] = {
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
#د_که_پای /* AVB_ROOT_KEY_BUILTIN */

ثابت حرف *str_avb_io_error(AvbIOResult res)
{
	بدل (res) {
	حالة AVB_IO_پایله_OK:
		أرجع "Requested operation was successful";
	حالة AVB_IO_پایله_ERROR_IO:
		أرجع "Underlying hardware encountered an I/O error";
	حالة AVB_IO_پایله_ERROR_OOM:
		أرجع "Unable to allocate memory";
	حالة AVB_IO_پایله_ERROR_NO_SUCH_برخه_کول:
		أرجع "Requested partition does not exist";
	حالة AVB_IO_پایله_ERROR_RANGE_OUTSIDE_برخه_کول:
		أرجع "Bytes requested is outside the range of partition";
	حالة AVB_IO_پایله_ERROR_NO_SUCH_ارزښت:
		أرجع "Named persistent value does not exist";
	حالة AVB_IO_پایله_ERROR_INVALID_ارزښت_اندازه:
		أرجع "Named persistent value size is not supported";
	حالة AVB_IO_پایله_ERROR_INSUFFICIENT_SPACE:
		أرجع "Buffer is too small for the requested operation";
	افتراضي:
		أرجع "Unknown AVB error";
	}
}

ثابت حرف *str_avb_خونه_error(Avb_خونه_Verify_پایله res)
{
	بدل (res) {
	حالة AVB_خونه_VERIFY_پایله_OK:
		أرجع "Verification passed successfully";
	حالة AVB_خونه_VERIFY_پایله_ERROR_OOM:
		أرجع "Allocation of memory failed";
	حالة AVB_خونه_VERIFY_پایله_ERROR_IO:
		أرجع "I/O error occurred while trying to load data";
	حالة AVB_خونه_VERIFY_پایله_ERROR_VERIFICATION:
		أرجع "Digest didn't match or signature checks failed";
	حالة AVB_خونه_VERIFY_پایله_ERROR_ROLLBACK_شاخص:
		أرجع "Rollback index is less than its stored value";
	حالة AVB_خونه_VERIFY_پایله_ERROR_عامه_کلی_REJECTED:
		أرجع "Public keys are not accepted";
	حالة AVB_خونه_VERIFY_پایله_ERROR_INVALID_METADATA:
		أرجع "Metadata is invalid or inconsistent";
	حالة AVB_خونه_VERIFY_پایله_ERROR_UNSUPPORTED_نسخه:
		أرجع "Metadata requires a newer version of libavb";
	حالة AVB_خونه_VERIFY_پایله_ERROR_INVALID_دلیل:
		أرجع "Invalid arguments are used";
	افتراضي:
		أرجع "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
حرف *avb_سېټ_state(AvbOps *ops, تعداد avb_boot_state boot_state)
{
	بنية Avb_Ops_معلومات *معلومات;
	حرف *cmdline = عدم;

	إذا (!ops)
		أرجع عدم;

	معلومات = (بنية Avb_Ops_معلومات *)ops->user_معلومات;
	إذا (!معلومات)
		أرجع عدم;

	معلومات->boot_state = boot_state;
	بدل (boot_state) {
	حالة AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		اكسر;
	حالة AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		اكسر;
	حالة AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		اكسر;
	حالة AVB_RED:
		اكسر;
	}

	أرجع cmdline;
}

حرف *append_cmd_کرښه(حرف *cmdline_orig, حرف *cmdline_new)
{
	حرف *cmd_کرښه;

	إذا (!cmdline_new)
		أرجع cmdline_orig;

	إذا (cmdline_orig)
		cmd_کرښه = cmdline_orig;
	وإلا
		cmd_کرښه = " ";

	cmd_کرښه = avb_strdupv(cmd_کرښه, " ", cmdline_new, عدم);

	أرجع cmd_کرښه;
}

ساكن صحيح avb_پیدا_کړه_dm_دلیلونه(حرف **دلیلونه, حرف *str)
{
	صحيح i;

	إذا (!str)
		أرجع -1;

	لكل (i = 0; i < AVB_MAX_دلیلونه && دلیلونه[i]; ++i) {
		إذا (مزي_پلټنه(دلیلونه[i], str))
			أرجع i;
	}

	أرجع -1;
}

ساكن حرف *avb_سېټ_enforce_option(ثابت حرف *cmdline, ثابت حرف *option)
{
	حرف *cmdarg[AVB_MAX_دلیلونه];
	حرف *newargs = عدم;
	صحيح i = 0;
	صحيح ټول_دلیلونه;

	memset(cmdarg, 0, حجم(cmdarg));
	cmdarg[i++] = strtok((حرف *)cmdline, " ");

	افعل {
		cmdarg[i] = strtok(عدم, " ");
		إذا (!cmdarg[i])
			اكسر;

		إذا (++i >= AVB_MAX_دلیلونه) {
			چاپ_کړه("%s: Can't handle more then %d args\n",
			       __func__, i);
			أرجع عدم;
		}
	} طالما (true);

	ټول_دلیلونه = i;
	i = avb_پیدا_کړه_dm_دلیلونه(&cmdarg[0], VERITY_جدول_OPT_LOGGING);
	إذا (i >= 0) {
		cmdarg[i] = (حرف *)option;
	} وإلا {
		i = avb_پیدا_کړه_dm_دلیلونه(&cmdarg[0], VERITY_جدول_OPT_RESTART);
		إذا (i < 0) {
			چاپ_کړه("%s: No verity options found\n", __func__);
			أرجع عدم;
		}

		cmdarg[i] = (حرف *)option;
	}

	لكل (i = 0; i <= ټول_دلیلونه; i++)
		newargs = append_cmd_کرښه(newargs, cmdarg[i]);

	أرجع newargs;
}

حرف *avb_سېټ_ignore_corruption(ثابت حرف *cmdline)
{
	حرف *newargs = عدم;

	newargs = avb_سېټ_enforce_option(cmdline, VERITY_جدول_OPT_LOGGING);
	إذا (newargs)
		newargs = append_cmd_کرښه(newargs,
					  "androidboot.veritymode=eio");

	أرجع newargs;
}

حرف *avb_سېټ_enforce_verity(ثابت حرف *cmdline)
{
	حرف *newargs;

	newargs = avb_سېټ_enforce_option(cmdline, VERITY_جدول_OPT_RESTART);
	إذا (newargs)
		newargs = append_cmd_کرښه(newargs,
					  "androidboot.veritymode=enforcing");
	أرجع newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
ساكن بینښه اوږد mmc_read_and_flush(بنية mmc_part *part,
					lbaint_t پیل,
					lbaint_t sectors,
					فراغ *بفر)
{
	بینښه اوږد blks;
	فراغ *tmp_buf;
	size_t buf_اندازه;
	bool unaligned = is_buf_unaligned(بفر);

	إذا (پیل < part->info.پیل) {
		چاپ_کړه("%s: partition start out of bounds\n", __func__);
		أرجع 0;
	}
	إذا ((پیل + sectors) > (part->info.پیل + part->info.اندازه)) {
		sectors = part->info.پیل + part->info.اندازه - پیل;
		چاپ_کړه("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	إذا (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, بفر);
		tmp_buf = get_sector_buf();
		buf_اندازه = get_sector_buf_اندازه();
		إذا (sectors > buf_اندازه / part->info.blksz)
			sectors = buf_اندازه / part->info.blksz;
	} وإلا {
		tmp_buf = بفر;
	}

	blks = blk_dread(part->mmc_blk,
			 پیل, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	إذا (unaligned)
		حافظه_نقل(بفر, tmp_buf, sectors * part->info.blksz);

	أرجع blks;
}

ساكن بینښه اوږد mmc_write(بنية mmc_part *part, lbaint_t پیل,
			       lbaint_t sectors, فراغ *بفر)
{
	فراغ *tmp_buf;
	size_t buf_اندازه;
	bool unaligned = is_buf_unaligned(بفر);

	إذا (پیل < part->info.پیل) {
		چاپ_کړه("%s: partition start out of bounds\n", __func__);
		أرجع 0;
	}
	إذا ((پیل + sectors) > (part->info.پیل + part->info.اندازه)) {
		sectors = part->info.پیل + part->info.اندازه - پیل;
		چاپ_کړه("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	إذا (unaligned) {
		tmp_buf = get_sector_buf();
		buf_اندازه = get_sector_buf_اندازه();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, بفر);
		إذا (sectors > buf_اندازه / part->info.blksz)
			sectors = buf_اندازه / part->info.blksz;

		حافظه_نقل(tmp_buf, بفر, sectors * part->info.blksz);
	} وإلا {
		tmp_buf = بفر;
	}

	أرجع blk_dwrite(part->mmc_blk,
			  پیل, sectors, tmp_buf);
}

ساكن بنية mmc_part *get_برخه_کول(AvbOps *ops, ثابت حرف *برخه_کول)
{
	صحيح ret;
	u8 dev_عدد;
	صحيح part_عدد = 0;
	بنية mmc_part *part;
	بنية blk_desc *mmc_blk;

	part = حافظه_ورکړه(حجم(بنية mmc_part));
	إذا (!part)
		أرجع عدم;

	dev_عدد = get_boot_device(ops);
	part->mmc = پیدا_کړه_mmc_device(dev_عدد);
	إذا (!part->mmc) {
		چاپ_کړه("%s: no MMC device at slot %x\n", __func__, dev_عدد);
		اذهب err;
	}

	ret = mmc_init(part->mmc);
	إذا (ret) {
		چاپ_کړه("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		اذهب err;
	}

	إذا (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_عدد);
		إذا (ret) {
			چاپ_کړه("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			اذهب err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	إذا (!mmc_blk) {
		چاپ_کړه("%s: failed to obtain block descriptor\n", __func__);
		اذهب err;
	}

	ret = part_get_info_by_name(mmc_blk, برخه_کول, &part->info);
	إذا (ret < 0) {
		چاپ_کړه("%s: can't find partition '%s'\n", __func__, برخه_کول);
		اذهب err;
	}

	part->dev_عدد = dev_عدد;
	part->mmc_blk = mmc_blk;

	أرجع part;
err:
	حرر(part);
	أرجع عدم;
}

ساكن AvbIOResult mmc_byte_io(AvbOps *ops,
			       ثابت حرف *برخه_کول,
			       s64 دننه_واټن,
			       size_t عدد_بایټونه,
			       فراغ *بفر,
			       size_t *بهر_عدد_read,
			       تعداد mmc_io_type io_type)
{
	ulong ret;
	بنية mmc_part *part;
	u64 پیل_دننه_واټن, پیل_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_پایله_OK;

	إذا (!برخه_کول || !بفر || io_type > IO_WRITE)
		أرجع AVB_IO_پایله_ERROR_IO;

	part = get_برخه_کول(ops, برخه_کول);
	إذا (!part)
		أرجع AVB_IO_پایله_ERROR_NO_SUCH_برخه_کول;

	إذا (!part->info.blksz) {
		io_ret = AVB_IO_پایله_ERROR_IO;
		اذهب بهر;
	}

	پیل_دننه_واټن = calc_دننه_واټن(part, دننه_واټن);
	طالما (عدد_بایټونه) {
		پیل_sector = پیل_دننه_واټن / part->info.blksz;
		sectors = عدد_بایټونه / part->info.blksz;
		/* handle non block-aligned reads */
		إذا (پیل_دننه_واټن % part->info.blksz ||
		    عدد_بایټونه < part->info.blksz) {
			tmp_buf = get_sector_buf();
			إذا (پیل_دننه_واټن % part->info.blksz) {
				residue = part->info.blksz -
					(پیل_دننه_واټن % part->info.blksz);
				إذا (residue > عدد_بایټونه)
					residue = عدد_بایټونه;
			} وإلا {
				residue = عدد_بایټونه;
			}

			إذا (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.پیل +
							 پیل_sector,
							 1, tmp_buf);

				إذا (ret != 1) {
					چاپ_کړه("%s: read error (%ld, %lld)\n",
					       __func__, ret, پیل_sector);
					io_ret = AVB_IO_پایله_ERROR_IO;
					اذهب بهر;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (پیل_دننه_واټن % part->info.blksz);
				حافظه_نقل(بفر, (فراغ *)tmp_buf, residue);
			} وإلا {
				ret = mmc_read_and_flush(part,
							 part->info.پیل +
							 پیل_sector,
							 1, tmp_buf);

				إذا (ret != 1) {
					چاپ_کړه("%s: read error (%ld, %lld)\n",
					       __func__, ret, پیل_sector);
					io_ret = AVB_IO_پایله_ERROR_IO;
					اذهب بهر;
				}
				حافظه_نقل((فراغ *)tmp_buf +
					پیل_دننه_واټن % part->info.blksz,
					بفر, residue);

				ret = mmc_write(part, part->info.پیل +
						پیل_sector, 1, tmp_buf);
				إذا (ret != 1) {
					چاپ_کړه("%s: write error (%ld, %lld)\n",
					       __func__, ret, پیل_sector);
					io_ret = AVB_IO_پایله_ERROR_IO;
					اذهب بهر;
				}
			}

			io_cnt += residue;
			بفر += residue;
			پیل_دننه_واټن += residue;
			عدد_بایټونه -= residue;
			استمر;
		}

		إذا (sectors) {
			إذا (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.پیل +
							 پیل_sector,
							 sectors, بفر);
			} وإلا {
				ret = mmc_write(part,
						part->info.پیل +
						پیل_sector,
						sectors, بفر);
			}

			إذا (!ret) {
				چاپ_کړه("%s: sector read error\n", __func__);
				io_ret = AVB_IO_پایله_ERROR_IO;
				اذهب بهر;
			}

			io_cnt += ret * part->info.blksz;
			بفر += ret * part->info.blksz;
			پیل_دننه_واټن += ret * part->info.blksz;
			عدد_بایټونه -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	إذا (io_type == IO_READ && بهر_عدد_read)
		*بهر_عدد_read = io_cnt;

بهر:
	حرر(part);
	أرجع io_ret;
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
ساكن AvbIOResult read_from_برخه_کول(AvbOps *ops,
				       ثابت حرف *برخه_کول_name,
				       s64 دننه_واټن_from_برخه_کول,
				       size_t عدد_بایټونه,
				       فراغ *بفر,
				       size_t *بهر_عدد_read)
{
	أرجع mmc_byte_io(ops, برخه_کول_name, دننه_واټن_from_برخه_کول,
			   عدد_بایټونه, بفر, بهر_عدد_read, IO_READ);
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
ساكن AvbIOResult write_منزل_برخه_کول(AvbOps *ops,
				      ثابت حرف *برخه_کول_name,
				      s64 دننه_واټن_from_برخه_کول,
				      size_t عدد_بایټونه,
				      ثابت فراغ *بفر)
{
	أرجع mmc_byte_io(ops, برخه_کول_name, دننه_واټن_from_برخه_کول,
			   عدد_بایټونه, (فراغ *)بفر, عدم, IO_WRITE);
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
ساكن AvbIOResult validate_vbmeta_عامه_کلی(AvbOps *ops,
					      ثابت u8 *عامه_کلی_معلومات,
					      size_t عامه_کلی_اوږدتیا,
					      ثابت u8
					      *عامه_کلی_metadata,
					      size_t
					      عامه_کلی_metadata_اوږدتیا,
					      bool *بهر_کلی_is_trusted)
{
	u8 کلی_digest[SHA256_مجموعه_LEN];
	u8 trusted_digest[SHA256_مجموعه_LEN];
	AvbIOResult rc;

	إذا (!عامه_کلی_اوږدتیا || !عامه_کلی_معلومات || !بهر_کلی_is_trusted)
		أرجع AVB_IO_پایله_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*بهر_کلی_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(عامه_کلی_معلومات, عامه_کلی_اوږدتیا, کلی_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_ریښه_کلی_digest(ops, trusted_digest);
	إذا (rc != AVB_IO_پایله_OK)
		أرجع rc;

	إذا (avb_خوندي_memcmp(کلی_digest, trusted_digest, حجم(کلی_digest)) == 0)
		*بهر_کلی_is_trusted = true;

	أرجع AVB_IO_پایله_OK;
}

#که_تعریف_شوی CONFIG_OPTEE_TA_AVB
ساكن صحيح get_open_session(بنية Avb_Ops_معلومات *ops_معلومات)
{
	بنية udevice *tee = عدم;

	طالما (!ops_معلومات->tee) {
		ثابت بنية tee_optee_ta_uuid uuid = TA_AVB_UUID;
		بنية tee_open_session_arg arg;
		صحيح rc;

		tee = tee_پیدا_کړه_device(tee, عدم, عدم, عدم);
		إذا (!tee)
			أرجع -ENODEV;

		memset(&arg, 0, حجم(arg));
		tee_optee_ta_uuid_منزل_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, عدم);
		إذا (rc || arg.ret)
			استمر;

		ops_معلومات->tee = tee;
		ops_معلومات->session = arg.session;
	}

	أرجع 0;
}

ساكن AvbIOResult invoke_func(بنية Avb_Ops_معلومات *ops_معلومات, u32 func,
			       ulong عدد_param, بنية tee_param *param)
{
	بنية tee_invoke_arg arg;

	إذا (get_open_session(ops_معلومات))
		أرجع AVB_IO_پایله_ERROR_IO;

	memset(&arg, 0, حجم(arg));
	arg.func = func;
	arg.session = ops_معلومات->session;

	إذا (tee_invoke_func(ops_معلومات->tee, &arg, عدد_param, param))
		أرجع AVB_IO_پایله_ERROR_IO;
	بدل (arg.ret) {
	حالة TEE_SUCCESS:
		أرجع AVB_IO_پایله_OK;
	حالة TEE_ERROR_بهر_د_حافظه:
		أرجع AVB_IO_پایله_ERROR_OOM;
	حالة TEE_ERROR_STORAGE_NO_SPACE:
		أرجع AVB_IO_پایله_ERROR_INSUFFICIENT_SPACE;
	حالة TEE_ERROR_توکی_NOT_وموندل:
		أرجع AVB_IO_پایله_ERROR_NO_SUCH_ارزښت;
	حالة TEE_ERROR_موخه_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_معلومات->tee, ops_معلومات->session);
		ops_معلومات->tee = عدم;
		أرجع AVB_IO_پایله_ERROR_IO;
	افتراضي:
		أرجع AVB_IO_پایله_ERROR_IO;
	}
}
#د_که_پای

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
ساكن AvbIOResult read_rollback_شاخص(AvbOps *ops,
				       size_t rollback_شاخص_خونه,
				       u64 *بهر_rollback_شاخص)
{
#که_نه_تعریف_شوی CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	إذا (بهر_rollback_شاخص)
		*بهر_rollback_شاخص = 0;

	أرجع AVB_IO_پایله_OK;
#وإلا
	AvbIOResult rc;
	بنية tee_param param[2];

	إذا (rollback_شاخص_خونه >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		أرجع AVB_IO_پایله_ERROR_NO_SUCH_ارزښت;

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ارزښت_ننوت;
	param[0].u.ارزښت.a = rollback_شاخص_خونه;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ارزښت_OUTPUT;

	rc = invoke_func(ops->user_معلومات, TA_AVB_CMD_READ_ROLLBACK_شاخص,
			 ARRAY_اندازه(param), param);
	إذا (rc)
		أرجع rc;

	*بهر_rollback_شاخص = (u64)param[1].u.ارزښت.a << 32 |
			      (u32)param[1].u.ارزښت.b;
	أرجع AVB_IO_پایله_OK;
#د_که_پای
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
ساكن AvbIOResult write_rollback_شاخص(AvbOps *ops,
					size_t rollback_شاخص_خونه,
					u64 rollback_شاخص)
{
#که_نه_تعریف_شوی CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	أرجع AVB_IO_پایله_OK;
#وإلا
	بنية tee_param param[2];

	إذا (rollback_شاخص_خونه >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		أرجع AVB_IO_پایله_ERROR_NO_SUCH_ارزښت;

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_ارزښت_ننوت;
	param[0].u.ارزښت.a = rollback_شاخص_خونه;
	param[1].attr = TEE_PARAM_ATTR_TYPE_ارزښت_ننوت;
	param[1].u.ارزښت.a = (u32)(rollback_شاخص >> 32);
	param[1].u.ارزښت.b = (u32)rollback_شاخص;

	أرجع invoke_func(ops->user_معلومات, TA_AVB_CMD_WRITE_ROLLBACK_شاخص,
			   ARRAY_اندازه(param), param);
#د_که_پای
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
ساكن AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *بهر_is_unlocked)
{
#که_نه_تعریف_شوی CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*بهر_is_unlocked = true;

	أرجع AVB_IO_پایله_OK;
#وإلا
	AvbIOResult rc;
	بنية tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_ارزښت_OUTPUT };

	rc = invoke_func(ops->user_معلومات, TA_AVB_CMD_READ_قلف_STATE, 1, &param);
	إذا (rc)
		أرجع rc;
	*بهر_is_unlocked = !param.u.ارزښت.a;
	أرجع AVB_IO_پایله_OK;
#د_که_پای
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
ساكن AvbIOResult get_unique_guid_for_برخه_کول(AvbOps *ops,
						 ثابت حرف *برخه_کول,
						 حرف *guid_buf,
						 size_t guid_buf_اندازه)
{
	بنية mmc_part *part;
	size_t uuid_اندازه;

	part = get_برخه_کول(ops, برخه_کول);
	إذا (!part)
		أرجع AVB_IO_پایله_ERROR_NO_SUCH_برخه_کول;

	uuid_اندازه = حجم(part->info.uuid);
	إذا (uuid_اندازه > guid_buf_اندازه) {
		حرر(part);
		أرجع AVB_IO_پایله_ERROR_IO;
	}

	حافظه_نقل(guid_buf, part->info.uuid, uuid_اندازه);
	guid_buf[uuid_اندازه - 1] = 0;

	حرر(part);
	أرجع AVB_IO_پایله_OK;
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
ساكن AvbIOResult get_اندازه_د_برخه_کول(AvbOps *ops,
					 ثابت حرف *برخه_کول,
					 u64 *بهر_اندازه_عدد_بایټونه)
{
	بنية mmc_part *part;

	إذا (!بهر_اندازه_عدد_بایټونه)
		أرجع AVB_IO_پایله_ERROR_INSUFFICIENT_SPACE;

	part = get_برخه_کول(ops, برخه_کول);
	إذا (!part)
		أرجع AVB_IO_پایله_ERROR_NO_SUCH_برخه_کول;

	*بهر_اندازه_عدد_بایټونه = part->info.blksz * part->info.اندازه;
	حرر(part);

	أرجع AVB_IO_پایله_OK;
}

#که_تعریف_شوی CONFIG_OPTEE_TA_AVB
ساكن AvbIOResult read_persistent_ارزښت(AvbOps *ops,
					 ثابت حرف *name,
					 size_t بفر_اندازه,
					 u8 *بهر_بفر,
					 size_t *بهر_عدد_بایټونه_read)
{
	AvbIOResult rc;
	بنية tee_shm *shm_name;
	بنية tee_shm *shm_buf;
	بنية tee_param param[2];
	بنية udevice *tee;
	size_t name_اندازه = مزي_اوږدوالی(name) + 1;

	إذا (get_open_session(ops->user_معلومات))
		أرجع AVB_IO_پایله_ERROR_IO;

	tee = ((بنية Avb_Ops_معلومات *)ops->user_معلومات)->tee;

	rc = tee_shm_alloc(tee, name_اندازه,
			   TEE_SHM_ALLOC, &shm_name);
	إذا (rc)
		أرجع AVB_IO_پایله_ERROR_OOM;

	rc = tee_shm_alloc(tee, بفر_اندازه,
			   TEE_SHM_ALLOC, &shm_buf);
	إذا (rc) {
		rc = AVB_IO_پایله_ERROR_OOM;
		اذهب free_name;
	}

	حافظه_نقل(shm_name->addr, name, name_اندازه);

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ننوت;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.اندازه = name_اندازه;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.اندازه = بفر_اندازه;

	rc = invoke_func(ops->user_معلومات, TA_AVB_CMD_READ_PERSIST_ارزښت,
			 2, param);
	إذا (rc)
		اذهب بهر;

	إذا (param[1].u.memref.اندازه > بفر_اندازه) {
		rc = AVB_IO_پایله_ERROR_NO_SUCH_ارزښت;
		اذهب بهر;
	}

	*بهر_عدد_بایټونه_read = param[1].u.memref.اندازه;

	حافظه_نقل(بهر_بفر, shm_buf->addr, *بهر_عدد_بایټونه_read);

بهر:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	أرجع rc;
}

ساكن AvbIOResult write_persistent_ارزښت(AvbOps *ops,
					  ثابت حرف *name,
					  size_t ارزښت_اندازه,
					  ثابت u8 *ارزښت)
{
	AvbIOResult rc;
	بنية tee_shm *shm_name;
	بنية tee_shm *shm_buf;
	بنية tee_param param[2];
	بنية udevice *tee;
	size_t name_اندازه = مزي_اوږدوالی(name) + 1;

	إذا (get_open_session(ops->user_معلومات))
		أرجع AVB_IO_پایله_ERROR_IO;

	tee = ((بنية Avb_Ops_معلومات *)ops->user_معلومات)->tee;

	إذا (!ارزښت_اندازه)
		أرجع AVB_IO_پایله_ERROR_NO_SUCH_ارزښت;

	rc = tee_shm_alloc(tee, name_اندازه,
			   TEE_SHM_ALLOC, &shm_name);
	إذا (rc)
		أرجع AVB_IO_پایله_ERROR_OOM;

	rc = tee_shm_alloc(tee, ارزښت_اندازه,
			   TEE_SHM_ALLOC, &shm_buf);
	إذا (rc) {
		rc = AVB_IO_پایله_ERROR_OOM;
		اذهب free_name;
	}

	حافظه_نقل(shm_name->addr, name, name_اندازه);
	حافظه_نقل(shm_buf->addr, ارزښت, ارزښت_اندازه);

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ننوت;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.اندازه = name_اندازه;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ننوت;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.اندازه = ارزښت_اندازه;

	rc = invoke_func(ops->user_معلومات, TA_AVB_CMD_WRITE_PERSIST_ارزښت,
			 2, param);
	إذا (rc)
		اذهب بهر;

بهر:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	أرجع rc;
}
#د_که_پای

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#إذا CONFIG_IS_ENABLED(AVB_ریښه_کلی_BUILTIN)
AvbIOResult avb_read_ریښه_کلی_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_ریښه_pub, حجم(avb_ریښه_pub), digest,
		       CHUNKSZ_SHA256);

	أرجع AVB_IO_پایله_OK;
}
#وإلاإذا CONFIG_IS_ENABLED(AVB_ریښه_کلی_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_ریښه_کلی_digest(AvbOps *ops, uint8_t *digest)
{
	size_t عدد_read = 0;
	AvbIOResult rc;

	rc = read_persistent_ارزښت(ops, CONFIG_AVB_ریښه_کلی_TEE_NAME,
				   SHA256_مجموعه_LEN, digest, &عدد_read);
	إذا (rc != AVB_IO_پایله_OK)
		أرجع rc;

	إذا (عدد_read != SHA256_مجموعه_LEN)
		أرجع AVB_IO_پایله_ERROR_INVALID_ارزښت_اندازه;

	أرجع AVB_IO_پایله_OK;
}
#وإلا /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_ریښه_کلی_digest(AvbOps *ops, uint8_t *digest)
{
	چاپ_کړه("%s: board root key provider not implemented\n", __func__);

	أرجع AVB_IO_پایله_ERROR_NO_SUCH_ارزښت;
}
#د_که_پای

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(صحيح boot_device)
{
	بنية Avb_Ops_معلومات *ops_معلومات;

	ops_معلومات = avb_calloc(حجم(بنية Avb_Ops_معلومات));
	إذا (!ops_معلومات)
		أرجع عدم;

	ops_معلومات->ops.user_معلومات = ops_معلومات;

	ops_معلومات->ops.read_from_برخه_کول = read_from_برخه_کول;
	ops_معلومات->ops.write_منزل_برخه_کول = write_منزل_برخه_کول;
	ops_معلومات->ops.validate_vbmeta_عامه_کلی = validate_vbmeta_عامه_کلی;
	ops_معلومات->ops.read_rollback_شاخص = read_rollback_شاخص;
	ops_معلومات->ops.write_rollback_شاخص = write_rollback_شاخص;
	ops_معلومات->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_معلومات->ops.get_unique_guid_for_برخه_کول =
		get_unique_guid_for_برخه_کول;
#که_تعریف_شوی CONFIG_OPTEE_TA_AVB
	ops_معلومات->ops.write_persistent_ارزښت = write_persistent_ارزښت;
	ops_معلومات->ops.read_persistent_ارزښت = read_persistent_ارزښت;
#د_که_پای
	ops_معلومات->ops.get_اندازه_د_برخه_کول = get_اندازه_د_برخه_کول;
	ops_معلومات->mmc_dev = boot_device;

	أرجع &ops_معلومات->ops;
}

فراغ avb_ops_free(AvbOps *ops)
{
	بنية Avb_Ops_معلومات *ops_معلومات;

	إذا (!ops)
		أرجع;

	ops_معلومات = ops->user_معلومات;

	إذا (ops_معلومات) {
#که_تعریف_شوی CONFIG_OPTEE_TA_AVB
		إذا (ops_معلومات->tee)
			tee_close_session(ops_معلومات->tee, ops_معلومات->session);
#د_که_پای
		avb_free(ops_معلومات);
	}
}
