// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#شامل <avb_verify.h>
#شامل <blk.h>
#شامل <cpu_func.h>
#شامل <image.h>
#شامل <malloc.h>
#شامل <part.h>
#شامل <tee.h>
#شامل <tee/optee_ta_avb.h>
#شامل <u-boot/sha256.h>

#اگر CONFIG_IS_ENABLED(AVB_جڑ_کلید_BUILTIN)
ساکن ثابت غیرعلامتی حرف avb_جڑ_pub[1032] = {
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
#اگر_ختم /* AVB_ROOT_KEY_BUILTIN */

ثابت حرف *str_avb_io_error(AvbIOResult res)
{
	انتخاب (res) {
	حالت AVB_IO_نتیجہ_OK:
		واپس "Requested operation was successful";
	حالت AVB_IO_نتیجہ_ERROR_IO:
		واپس "Underlying hardware encountered an I/O error";
	حالت AVB_IO_نتیجہ_ERROR_OOM:
		واپس "Unable to allocate memory";
	حالت AVB_IO_نتیجہ_ERROR_NO_SUCH_تقسیم:
		واپس "Requested partition does not exist";
	حالت AVB_IO_نتیجہ_ERROR_RANGE_OUTSIDE_تقسیم:
		واپس "Bytes requested is outside the range of partition";
	حالت AVB_IO_نتیجہ_ERROR_NO_SUCH_قدر:
		واپس "Named persistent value does not exist";
	حالت AVB_IO_نتیجہ_ERROR_INVALID_قدر_سائز:
		واپس "Named persistent value size is not supported";
	حالت AVB_IO_نتیجہ_ERROR_INSUFFICIENT_SPACE:
		واپس "Buffer is too small for the requested operation";
	طےشدہ:
		واپس "Unknown AVB error";
	}
}

ثابت حرف *str_avb_خانہ_error(Avb_خانہ_Verify_نتیجہ res)
{
	انتخاب (res) {
	حالت AVB_خانہ_VERIFY_نتیجہ_OK:
		واپس "Verification passed successfully";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_OOM:
		واپس "Allocation of memory failed";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_IO:
		واپس "I/O error occurred while trying to load data";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_VERIFICATION:
		واپس "Digest didn't match or signature checks failed";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_ROLLBACK_اشاریہ:
		واپس "Rollback index is less than its stored value";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_عوامی_کلید_REJECTED:
		واپس "Public keys are not accepted";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_INVALID_METADATA:
		واپس "Metadata is invalid or inconsistent";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_UNSUPPORTED_نسخہ:
		واپس "Metadata requires a newer version of libavb";
	حالت AVB_خانہ_VERIFY_نتیجہ_ERROR_INVALID_دلیل:
		واپس "Invalid arguments are used";
	طےشدہ:
		واپس "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
حرف *avb_سیٹ_state(AvbOps *ops, شمار avb_boot_state boot_state)
{
	ساخت Avb_Ops_ڈیٹا *ڈیٹا;
	حرف *cmdline = تہی;

	اگر (!ops)
		واپس تہی;

	ڈیٹا = (ساخت Avb_Ops_ڈیٹا *)ops->user_ڈیٹا;
	اگر (!ڈیٹا)
		واپس تہی;

	ڈیٹا->boot_state = boot_state;
	انتخاب (boot_state) {
	حالت AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		توڑو;
	حالت AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		توڑو;
	حالت AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		توڑو;
	حالت AVB_RED:
		توڑو;
	}

	واپس cmdline;
}

حرف *append_cmd_سطر(حرف *cmdline_orig, حرف *cmdline_new)
{
	حرف *cmd_سطر;

	اگر (!cmdline_new)
		واپس cmdline_orig;

	اگر (cmdline_orig)
		cmd_سطر = cmdline_orig;
	ورنہ
		cmd_سطر = " ";

	cmd_سطر = avb_strdupv(cmd_سطر, " ", cmdline_new, تہی);

	واپس cmd_سطر;
}

ساکن صحیح avb_ڈھونڈو_dm_دلائل(حرف **دلائل, حرف *str)
{
	صحیح i;

	اگر (!str)
		واپس -1;

	برائے (i = 0; i < AVB_MAX_دلائل && دلائل[i]; ++i) {
		اگر (لڑی_تلاش(دلائل[i], str))
			واپس i;
	}

	واپس -1;
}

ساکن حرف *avb_سیٹ_enforce_option(ثابت حرف *cmdline, ثابت حرف *option)
{
	حرف *cmdarg[AVB_MAX_دلائل];
	حرف *newargs = تہی;
	صحیح i = 0;
	صحیح کل_دلائل;

	memset(cmdarg, 0, حجم(cmdarg));
	cmdarg[i++] = strtok((حرف *)cmdline, " ");

	کرو {
		cmdarg[i] = strtok(تہی, " ");
		اگر (!cmdarg[i])
			توڑو;

		اگر (++i >= AVB_MAX_دلائل) {
			لکھو("%s: Can't handle more then %d args\n",
			       __func__, i);
			واپس تہی;
		}
	} جب (true);

	کل_دلائل = i;
	i = avb_ڈھونڈو_dm_دلائل(&cmdarg[0], VERITY_جدول_OPT_LOGGING);
	اگر (i >= 0) {
		cmdarg[i] = (حرف *)option;
	} ورنہ {
		i = avb_ڈھونڈو_dm_دلائل(&cmdarg[0], VERITY_جدول_OPT_RESTART);
		اگر (i < 0) {
			لکھو("%s: No verity options found\n", __func__);
			واپس تہی;
		}

		cmdarg[i] = (حرف *)option;
	}

	برائے (i = 0; i <= کل_دلائل; i++)
		newargs = append_cmd_سطر(newargs, cmdarg[i]);

	واپس newargs;
}

حرف *avb_سیٹ_ignore_corruption(ثابت حرف *cmdline)
{
	حرف *newargs = تہی;

	newargs = avb_سیٹ_enforce_option(cmdline, VERITY_جدول_OPT_LOGGING);
	اگر (newargs)
		newargs = append_cmd_سطر(newargs,
					  "androidboot.veritymode=eio");

	واپس newargs;
}

حرف *avb_سیٹ_enforce_verity(ثابت حرف *cmdline)
{
	حرف *newargs;

	newargs = avb_سیٹ_enforce_option(cmdline, VERITY_جدول_OPT_RESTART);
	اگر (newargs)
		newargs = append_cmd_سطر(newargs,
					  "androidboot.veritymode=enforcing");
	واپس newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
ساکن غیرعلامتی طویل mmc_read_and_flush(ساخت mmc_part *part,
					lbaint_t آغاز,
					lbaint_t sectors,
					خالی *بفر)
{
	غیرعلامتی طویل blks;
	خالی *tmp_buf;
	size_t buf_سائز;
	bool unaligned = is_buf_unaligned(بفر);

	اگر (آغاز < part->info.آغاز) {
		لکھو("%s: partition start out of bounds\n", __func__);
		واپس 0;
	}
	اگر ((آغاز + sectors) > (part->info.آغاز + part->info.سائز)) {
		sectors = part->info.آغاز + part->info.سائز - آغاز;
		لکھو("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	اگر (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, بفر);
		tmp_buf = get_sector_buf();
		buf_سائز = get_sector_buf_سائز();
		اگر (sectors > buf_سائز / part->info.blksz)
			sectors = buf_سائز / part->info.blksz;
	} ورنہ {
		tmp_buf = بفر;
	}

	blks = blk_dread(part->mmc_blk,
			 آغاز, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	اگر (unaligned)
		حافظہ_نقل(بفر, tmp_buf, sectors * part->info.blksz);

	واپس blks;
}

ساکن غیرعلامتی طویل mmc_write(ساخت mmc_part *part, lbaint_t آغاز,
			       lbaint_t sectors, خالی *بفر)
{
	خالی *tmp_buf;
	size_t buf_سائز;
	bool unaligned = is_buf_unaligned(بفر);

	اگر (آغاز < part->info.آغاز) {
		لکھو("%s: partition start out of bounds\n", __func__);
		واپس 0;
	}
	اگر ((آغاز + sectors) > (part->info.آغاز + part->info.سائز)) {
		sectors = part->info.آغاز + part->info.سائز - آغاز;
		لکھو("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	اگر (unaligned) {
		tmp_buf = get_sector_buf();
		buf_سائز = get_sector_buf_سائز();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, بفر);
		اگر (sectors > buf_سائز / part->info.blksz)
			sectors = buf_سائز / part->info.blksz;

		حافظہ_نقل(tmp_buf, بفر, sectors * part->info.blksz);
	} ورنہ {
		tmp_buf = بفر;
	}

	واپس blk_dwrite(part->mmc_blk,
			  آغاز, sectors, tmp_buf);
}

ساکن ساخت mmc_part *get_تقسیم(AvbOps *ops, ثابت حرف *تقسیم)
{
	صحیح ret;
	u8 dev_عدد;
	صحیح part_عدد = 0;
	ساخت mmc_part *part;
	ساخت blk_desc *mmc_blk;

	part = حافظہ_دو(حجم(ساخت mmc_part));
	اگر (!part)
		واپس تہی;

	dev_عدد = get_boot_device(ops);
	part->mmc = ڈھونڈو_mmc_device(dev_عدد);
	اگر (!part->mmc) {
		لکھو("%s: no MMC device at slot %x\n", __func__, dev_عدد);
		جاؤ err;
	}

	ret = mmc_init(part->mmc);
	اگر (ret) {
		لکھو("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		جاؤ err;
	}

	اگر (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_عدد);
		اگر (ret) {
			لکھو("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			جاؤ err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	اگر (!mmc_blk) {
		لکھو("%s: failed to obtain block descriptor\n", __func__);
		جاؤ err;
	}

	ret = part_get_info_by_name(mmc_blk, تقسیم, &part->info);
	اگر (ret < 0) {
		لکھو("%s: can't find partition '%s'\n", __func__, تقسیم);
		جاؤ err;
	}

	part->dev_عدد = dev_عدد;
	part->mmc_blk = mmc_blk;

	واپس part;
err:
	آزاد(part);
	واپس تہی;
}

ساکن AvbIOResult mmc_byte_io(AvbOps *ops,
			       ثابت حرف *تقسیم,
			       s64 انحراف,
			       size_t عدد_بائٹیں,
			       خالی *بفر,
			       size_t *برآمد_عدد_read,
			       شمار mmc_io_type io_type)
{
	ulong ret;
	ساخت mmc_part *part;
	u64 آغاز_انحراف, آغاز_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_نتیجہ_OK;

	اگر (!تقسیم || !بفر || io_type > IO_WRITE)
		واپس AVB_IO_نتیجہ_ERROR_IO;

	part = get_تقسیم(ops, تقسیم);
	اگر (!part)
		واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_تقسیم;

	اگر (!part->info.blksz) {
		io_ret = AVB_IO_نتیجہ_ERROR_IO;
		جاؤ برآمد;
	}

	آغاز_انحراف = calc_انحراف(part, انحراف);
	جب (عدد_بائٹیں) {
		آغاز_sector = آغاز_انحراف / part->info.blksz;
		sectors = عدد_بائٹیں / part->info.blksz;
		/* handle non block-aligned reads */
		اگر (آغاز_انحراف % part->info.blksz ||
		    عدد_بائٹیں < part->info.blksz) {
			tmp_buf = get_sector_buf();
			اگر (آغاز_انحراف % part->info.blksz) {
				residue = part->info.blksz -
					(آغاز_انحراف % part->info.blksz);
				اگر (residue > عدد_بائٹیں)
					residue = عدد_بائٹیں;
			} ورنہ {
				residue = عدد_بائٹیں;
			}

			اگر (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.آغاز +
							 آغاز_sector,
							 1, tmp_buf);

				اگر (ret != 1) {
					لکھو("%s: read error (%ld, %lld)\n",
					       __func__, ret, آغاز_sector);
					io_ret = AVB_IO_نتیجہ_ERROR_IO;
					جاؤ برآمد;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (آغاز_انحراف % part->info.blksz);
				حافظہ_نقل(بفر, (خالی *)tmp_buf, residue);
			} ورنہ {
				ret = mmc_read_and_flush(part,
							 part->info.آغاز +
							 آغاز_sector,
							 1, tmp_buf);

				اگر (ret != 1) {
					لکھو("%s: read error (%ld, %lld)\n",
					       __func__, ret, آغاز_sector);
					io_ret = AVB_IO_نتیجہ_ERROR_IO;
					جاؤ برآمد;
				}
				حافظہ_نقل((خالی *)tmp_buf +
					آغاز_انحراف % part->info.blksz,
					بفر, residue);

				ret = mmc_write(part, part->info.آغاز +
						آغاز_sector, 1, tmp_buf);
				اگر (ret != 1) {
					لکھو("%s: write error (%ld, %lld)\n",
					       __func__, ret, آغاز_sector);
					io_ret = AVB_IO_نتیجہ_ERROR_IO;
					جاؤ برآمد;
				}
			}

			io_cnt += residue;
			بفر += residue;
			آغاز_انحراف += residue;
			عدد_بائٹیں -= residue;
			جاری;
		}

		اگر (sectors) {
			اگر (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.آغاز +
							 آغاز_sector,
							 sectors, بفر);
			} ورنہ {
				ret = mmc_write(part,
						part->info.آغاز +
						آغاز_sector,
						sectors, بفر);
			}

			اگر (!ret) {
				لکھو("%s: sector read error\n", __func__);
				io_ret = AVB_IO_نتیجہ_ERROR_IO;
				جاؤ برآمد;
			}

			io_cnt += ret * part->info.blksz;
			بفر += ret * part->info.blksz;
			آغاز_انحراف += ret * part->info.blksz;
			عدد_بائٹیں -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	اگر (io_type == IO_READ && برآمد_عدد_read)
		*برآمد_عدد_read = io_cnt;

برآمد:
	آزاد(part);
	واپس io_ret;
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
ساکن AvbIOResult read_from_تقسیم(AvbOps *ops,
				       ثابت حرف *تقسیم_name,
				       s64 انحراف_from_تقسیم,
				       size_t عدد_بائٹیں,
				       خالی *بفر,
				       size_t *برآمد_عدد_read)
{
	واپس mmc_byte_io(ops, تقسیم_name, انحراف_from_تقسیم,
			   عدد_بائٹیں, بفر, برآمد_عدد_read, IO_READ);
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
ساکن AvbIOResult write_مقصد_تقسیم(AvbOps *ops,
				      ثابت حرف *تقسیم_name,
				      s64 انحراف_from_تقسیم,
				      size_t عدد_بائٹیں,
				      ثابت خالی *بفر)
{
	واپس mmc_byte_io(ops, تقسیم_name, انحراف_from_تقسیم,
			   عدد_بائٹیں, (خالی *)بفر, تہی, IO_WRITE);
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
ساکن AvbIOResult validate_vbmeta_عوامی_کلید(AvbOps *ops,
					      ثابت u8 *عوامی_کلید_ڈیٹا,
					      size_t عوامی_کلید_لمبائی,
					      ثابت u8
					      *عوامی_کلید_metadata,
					      size_t
					      عوامی_کلید_metadata_لمبائی,
					      bool *برآمد_کلید_is_trusted)
{
	u8 کلید_digest[SHA256_مجموع_LEN];
	u8 trusted_digest[SHA256_مجموع_LEN];
	AvbIOResult rc;

	اگر (!عوامی_کلید_لمبائی || !عوامی_کلید_ڈیٹا || !برآمد_کلید_is_trusted)
		واپس AVB_IO_نتیجہ_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*برآمد_کلید_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(عوامی_کلید_ڈیٹا, عوامی_کلید_لمبائی, کلید_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_جڑ_کلید_digest(ops, trusted_digest);
	اگر (rc != AVB_IO_نتیجہ_OK)
		واپس rc;

	اگر (avb_محفوظ_memcmp(کلید_digest, trusted_digest, حجم(کلید_digest)) == 0)
		*برآمد_کلید_is_trusted = true;

	واپس AVB_IO_نتیجہ_OK;
}

#اگر_تعریف_شدہ CONFIG_OPTEE_TA_AVB
ساکن صحیح get_open_session(ساخت Avb_Ops_ڈیٹا *ops_ڈیٹا)
{
	ساخت udevice *tee = تہی;

	جب (!ops_ڈیٹا->tee) {
		ثابت ساخت tee_optee_ta_uuid uuid = TA_AVB_UUID;
		ساخت tee_open_session_arg arg;
		صحیح rc;

		tee = tee_ڈھونڈو_device(tee, تہی, تہی, تہی);
		اگر (!tee)
			واپس -ENODEV;

		memset(&arg, 0, حجم(arg));
		tee_optee_ta_uuid_مقصد_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, تہی);
		اگر (rc || arg.ret)
			جاری;

		ops_ڈیٹا->tee = tee;
		ops_ڈیٹا->session = arg.session;
	}

	واپس 0;
}

ساکن AvbIOResult invoke_func(ساخت Avb_Ops_ڈیٹا *ops_ڈیٹا, u32 func,
			       ulong عدد_param, ساخت tee_param *param)
{
	ساخت tee_invoke_arg arg;

	اگر (get_open_session(ops_ڈیٹا))
		واپس AVB_IO_نتیجہ_ERROR_IO;

	memset(&arg, 0, حجم(arg));
	arg.func = func;
	arg.session = ops_ڈیٹا->session;

	اگر (tee_invoke_func(ops_ڈیٹا->tee, &arg, عدد_param, param))
		واپس AVB_IO_نتیجہ_ERROR_IO;
	انتخاب (arg.ret) {
	حالت TEE_SUCCESS:
		واپس AVB_IO_نتیجہ_OK;
	حالت TEE_ERROR_برآمد_کا_حافظہ:
		واپس AVB_IO_نتیجہ_ERROR_OOM;
	حالت TEE_ERROR_STORAGE_NO_SPACE:
		واپس AVB_IO_نتیجہ_ERROR_INSUFFICIENT_SPACE;
	حالت TEE_ERROR_شے_NOT_ملا:
		واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_قدر;
	حالت TEE_ERROR_ہدف_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_ڈیٹا->tee, ops_ڈیٹا->session);
		ops_ڈیٹا->tee = تہی;
		واپس AVB_IO_نتیجہ_ERROR_IO;
	طےشدہ:
		واپس AVB_IO_نتیجہ_ERROR_IO;
	}
}
#اگر_ختم

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
ساکن AvbIOResult read_rollback_اشاریہ(AvbOps *ops,
				       size_t rollback_اشاریہ_خانہ,
				       u64 *برآمد_rollback_اشاریہ)
{
#اگر_غیر_تعریف_شدہ CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	اگر (برآمد_rollback_اشاریہ)
		*برآمد_rollback_اشاریہ = 0;

	واپس AVB_IO_نتیجہ_OK;
#ورنہ
	AvbIOResult rc;
	ساخت tee_param param[2];

	اگر (rollback_اشاریہ_خانہ >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_قدر;

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_قدر_داخلہ;
	param[0].u.قدر.a = rollback_اشاریہ_خانہ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_قدر_OUTPUT;

	rc = invoke_func(ops->user_ڈیٹا, TA_AVB_CMD_READ_ROLLBACK_اشاریہ,
			 ARRAY_سائز(param), param);
	اگر (rc)
		واپس rc;

	*برآمد_rollback_اشاریہ = (u64)param[1].u.قدر.a << 32 |
			      (u32)param[1].u.قدر.b;
	واپس AVB_IO_نتیجہ_OK;
#اگر_ختم
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
ساکن AvbIOResult write_rollback_اشاریہ(AvbOps *ops,
					size_t rollback_اشاریہ_خانہ,
					u64 rollback_اشاریہ)
{
#اگر_غیر_تعریف_شدہ CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	واپس AVB_IO_نتیجہ_OK;
#ورنہ
	ساخت tee_param param[2];

	اگر (rollback_اشاریہ_خانہ >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_قدر;

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_قدر_داخلہ;
	param[0].u.قدر.a = rollback_اشاریہ_خانہ;
	param[1].attr = TEE_PARAM_ATTR_TYPE_قدر_داخلہ;
	param[1].u.قدر.a = (u32)(rollback_اشاریہ >> 32);
	param[1].u.قدر.b = (u32)rollback_اشاریہ;

	واپس invoke_func(ops->user_ڈیٹا, TA_AVB_CMD_WRITE_ROLLBACK_اشاریہ,
			   ARRAY_سائز(param), param);
#اگر_ختم
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
ساکن AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *برآمد_is_unlocked)
{
#اگر_غیر_تعریف_شدہ CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*برآمد_is_unlocked = true;

	واپس AVB_IO_نتیجہ_OK;
#ورنہ
	AvbIOResult rc;
	ساخت tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_قدر_OUTPUT };

	rc = invoke_func(ops->user_ڈیٹا, TA_AVB_CMD_READ_تالا_STATE, 1, &param);
	اگر (rc)
		واپس rc;
	*برآمد_is_unlocked = !param.u.قدر.a;
	واپس AVB_IO_نتیجہ_OK;
#اگر_ختم
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
ساکن AvbIOResult get_unique_guid_for_تقسیم(AvbOps *ops,
						 ثابت حرف *تقسیم,
						 حرف *guid_buf,
						 size_t guid_buf_سائز)
{
	ساخت mmc_part *part;
	size_t uuid_سائز;

	part = get_تقسیم(ops, تقسیم);
	اگر (!part)
		واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_تقسیم;

	uuid_سائز = حجم(part->info.uuid);
	اگر (uuid_سائز > guid_buf_سائز) {
		آزاد(part);
		واپس AVB_IO_نتیجہ_ERROR_IO;
	}

	حافظہ_نقل(guid_buf, part->info.uuid, uuid_سائز);
	guid_buf[uuid_سائز - 1] = 0;

	آزاد(part);
	واپس AVB_IO_نتیجہ_OK;
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
ساکن AvbIOResult get_سائز_کا_تقسیم(AvbOps *ops,
					 ثابت حرف *تقسیم,
					 u64 *برآمد_سائز_عدد_بائٹیں)
{
	ساخت mmc_part *part;

	اگر (!برآمد_سائز_عدد_بائٹیں)
		واپس AVB_IO_نتیجہ_ERROR_INSUFFICIENT_SPACE;

	part = get_تقسیم(ops, تقسیم);
	اگر (!part)
		واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_تقسیم;

	*برآمد_سائز_عدد_بائٹیں = part->info.blksz * part->info.سائز;
	آزاد(part);

	واپس AVB_IO_نتیجہ_OK;
}

#اگر_تعریف_شدہ CONFIG_OPTEE_TA_AVB
ساکن AvbIOResult read_persistent_قدر(AvbOps *ops,
					 ثابت حرف *name,
					 size_t بفر_سائز,
					 u8 *برآمد_بفر,
					 size_t *برآمد_عدد_بائٹیں_read)
{
	AvbIOResult rc;
	ساخت tee_shm *shm_name;
	ساخت tee_shm *shm_buf;
	ساخت tee_param param[2];
	ساخت udevice *tee;
	size_t name_سائز = لڑی_لمبائی(name) + 1;

	اگر (get_open_session(ops->user_ڈیٹا))
		واپس AVB_IO_نتیجہ_ERROR_IO;

	tee = ((ساخت Avb_Ops_ڈیٹا *)ops->user_ڈیٹا)->tee;

	rc = tee_shm_alloc(tee, name_سائز,
			   TEE_SHM_ALLOC, &shm_name);
	اگر (rc)
		واپس AVB_IO_نتیجہ_ERROR_OOM;

	rc = tee_shm_alloc(tee, بفر_سائز,
			   TEE_SHM_ALLOC, &shm_buf);
	اگر (rc) {
		rc = AVB_IO_نتیجہ_ERROR_OOM;
		جاؤ free_name;
	}

	حافظہ_نقل(shm_name->addr, name, name_سائز);

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_داخلہ;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.سائز = name_سائز;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.سائز = بفر_سائز;

	rc = invoke_func(ops->user_ڈیٹا, TA_AVB_CMD_READ_PERSIST_قدر,
			 2, param);
	اگر (rc)
		جاؤ برآمد;

	اگر (param[1].u.memref.سائز > بفر_سائز) {
		rc = AVB_IO_نتیجہ_ERROR_NO_SUCH_قدر;
		جاؤ برآمد;
	}

	*برآمد_عدد_بائٹیں_read = param[1].u.memref.سائز;

	حافظہ_نقل(برآمد_بفر, shm_buf->addr, *برآمد_عدد_بائٹیں_read);

برآمد:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	واپس rc;
}

ساکن AvbIOResult write_persistent_قدر(AvbOps *ops,
					  ثابت حرف *name,
					  size_t قدر_سائز,
					  ثابت u8 *قدر)
{
	AvbIOResult rc;
	ساخت tee_shm *shm_name;
	ساخت tee_shm *shm_buf;
	ساخت tee_param param[2];
	ساخت udevice *tee;
	size_t name_سائز = لڑی_لمبائی(name) + 1;

	اگر (get_open_session(ops->user_ڈیٹا))
		واپس AVB_IO_نتیجہ_ERROR_IO;

	tee = ((ساخت Avb_Ops_ڈیٹا *)ops->user_ڈیٹا)->tee;

	اگر (!قدر_سائز)
		واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_قدر;

	rc = tee_shm_alloc(tee, name_سائز,
			   TEE_SHM_ALLOC, &shm_name);
	اگر (rc)
		واپس AVB_IO_نتیجہ_ERROR_OOM;

	rc = tee_shm_alloc(tee, قدر_سائز,
			   TEE_SHM_ALLOC, &shm_buf);
	اگر (rc) {
		rc = AVB_IO_نتیجہ_ERROR_OOM;
		جاؤ free_name;
	}

	حافظہ_نقل(shm_name->addr, name, name_سائز);
	حافظہ_نقل(shm_buf->addr, قدر, قدر_سائز);

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_داخلہ;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.سائز = name_سائز;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_داخلہ;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.سائز = قدر_سائز;

	rc = invoke_func(ops->user_ڈیٹا, TA_AVB_CMD_WRITE_PERSIST_قدر,
			 2, param);
	اگر (rc)
		جاؤ برآمد;

برآمد:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	واپس rc;
}
#اگر_ختم

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#اگر CONFIG_IS_ENABLED(AVB_جڑ_کلید_BUILTIN)
AvbIOResult avb_read_جڑ_کلید_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_جڑ_pub, حجم(avb_جڑ_pub), digest,
		       CHUNKSZ_SHA256);

	واپس AVB_IO_نتیجہ_OK;
}
#وگرنہ_اگر CONFIG_IS_ENABLED(AVB_جڑ_کلید_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_جڑ_کلید_digest(AvbOps *ops, uint8_t *digest)
{
	size_t عدد_read = 0;
	AvbIOResult rc;

	rc = read_persistent_قدر(ops, CONFIG_AVB_جڑ_کلید_TEE_NAME,
				   SHA256_مجموع_LEN, digest, &عدد_read);
	اگر (rc != AVB_IO_نتیجہ_OK)
		واپس rc;

	اگر (عدد_read != SHA256_مجموع_LEN)
		واپس AVB_IO_نتیجہ_ERROR_INVALID_قدر_سائز;

	واپس AVB_IO_نتیجہ_OK;
}
#ورنہ /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_جڑ_کلید_digest(AvbOps *ops, uint8_t *digest)
{
	لکھو("%s: board root key provider not implemented\n", __func__);

	واپس AVB_IO_نتیجہ_ERROR_NO_SUCH_قدر;
}
#اگر_ختم

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(صحیح boot_device)
{
	ساخت Avb_Ops_ڈیٹا *ops_ڈیٹا;

	ops_ڈیٹا = avb_calloc(حجم(ساخت Avb_Ops_ڈیٹا));
	اگر (!ops_ڈیٹا)
		واپس تہی;

	ops_ڈیٹا->ops.user_ڈیٹا = ops_ڈیٹا;

	ops_ڈیٹا->ops.read_from_تقسیم = read_from_تقسیم;
	ops_ڈیٹا->ops.write_مقصد_تقسیم = write_مقصد_تقسیم;
	ops_ڈیٹا->ops.validate_vbmeta_عوامی_کلید = validate_vbmeta_عوامی_کلید;
	ops_ڈیٹا->ops.read_rollback_اشاریہ = read_rollback_اشاریہ;
	ops_ڈیٹا->ops.write_rollback_اشاریہ = write_rollback_اشاریہ;
	ops_ڈیٹا->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_ڈیٹا->ops.get_unique_guid_for_تقسیم =
		get_unique_guid_for_تقسیم;
#اگر_تعریف_شدہ CONFIG_OPTEE_TA_AVB
	ops_ڈیٹا->ops.write_persistent_قدر = write_persistent_قدر;
	ops_ڈیٹا->ops.read_persistent_قدر = read_persistent_قدر;
#اگر_ختم
	ops_ڈیٹا->ops.get_سائز_کا_تقسیم = get_سائز_کا_تقسیم;
	ops_ڈیٹا->mmc_dev = boot_device;

	واپس &ops_ڈیٹا->ops;
}

خالی avb_ops_free(AvbOps *ops)
{
	ساخت Avb_Ops_ڈیٹا *ops_ڈیٹا;

	اگر (!ops)
		واپس;

	ops_ڈیٹا = ops->user_ڈیٹا;

	اگر (ops_ڈیٹا) {
#اگر_تعریف_شدہ CONFIG_OPTEE_TA_AVB
		اگر (ops_ڈیٹا->tee)
			tee_close_session(ops_ڈیٹا->tee, ops_ڈیٹا->session);
#اگر_ختم
		avb_free(ops_ڈیٹا);
	}
}
