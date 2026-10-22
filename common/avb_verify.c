// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#అన్తర్భావ <avb_verify.h>
#అన్తర్భావ <blk.h>
#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <part.h>
#అన్తర్భావ <tee.h>
#అన్తర్భావ <tee/optee_ta_avb.h>
#అన్తర్భావ <u-boot/sha256.h>

#ఒకవేళ CONFIG_IS_ENABLED(AVB_మూలం_కీలకం_BUILTIN)
స్థిర స్థిరరాశి గుర్తులేని అక్షరం avb_మూలం_pub[1032] = {
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
#అయితే_ముగింపు /* AVB_ROOT_KEY_BUILTIN */

స్థిరరాశి అక్షరం *str_avb_io_error(AvbIOResult res)
{
	ఎంపిక (res) {
	స్థితి AVB_IO_ఫలితం_OK:
		తిరిగి "Requested operation was successful";
	స్థితి AVB_IO_ఫలితం_ERROR_IO:
		తిరిగి "Underlying hardware encountered an I/O error";
	స్థితి AVB_IO_ఫలితం_ERROR_OOM:
		తిరిగి "Unable to allocate memory";
	స్థితి AVB_IO_ఫలితం_ERROR_NO_SUCH_విభజన:
		తిరిగి "Requested partition does not exist";
	స్థితి AVB_IO_ఫలితం_ERROR_RANGE_OUTSIDE_విభజన:
		తిరిగి "Bytes requested is outside the range of partition";
	స్థితి AVB_IO_ఫలితం_ERROR_NO_SUCH_విలువ:
		తిరిగి "Named persistent value does not exist";
	స్థితి AVB_IO_ఫలితం_ERROR_INVALID_విలువ_కొలత:
		తిరిగి "Named persistent value size is not supported";
	స్థితి AVB_IO_ఫలితం_ERROR_INSUFFICIENT_SPACE:
		తిరిగి "Buffer is too small for the requested operation";
	అప్రమేయ:
		తిరిగి "Unknown AVB error";
	}
}

స్థిరరాశి అక్షరం *str_avb_స్థానం_error(Avb_స్థానం_Verify_ఫలితం res)
{
	ఎంపిక (res) {
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_OK:
		తిరిగి "Verification passed successfully";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_OOM:
		తిరిగి "Allocation of memory failed";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_IO:
		తిరిగి "I/O error occurred while trying to load data";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_VERIFICATION:
		తిరిగి "Digest didn't match or signature checks failed";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_ROLLBACK_సూచీ:
		తిరిగి "Rollback index is less than its stored value";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_ప్రజా_కీలకం_REJECTED:
		తిరిగి "Public keys are not accepted";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_INVALID_METADATA:
		తిరిగి "Metadata is invalid or inconsistent";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_UNSUPPORTED_సంస్కరణ:
		తిరిగి "Metadata requires a newer version of libavb";
	స్థితి AVB_స్థానం_VERIFY_ఫలితం_ERROR_INVALID_వాదం:
		తిరిగి "Invalid arguments are used";
	అప్రమేయ:
		తిరిగి "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
అక్షరం *avb_సమితి_state(AvbOps *ops, గణన avb_boot_state boot_state)
{
	నిర్మాణం Avb_Ops_దత్తాంశం *దత్తాంశం;
	అక్షరం *cmdline = ఖాళీ;

	ఒకవేళ (!ops)
		తిరిగి ఖాళీ;

	దత్తాంశం = (నిర్మాణం Avb_Ops_దత్తాంశం *)ops->user_దత్తాంశం;
	ఒకవేళ (!దత్తాంశం)
		తిరిగి ఖాళీ;

	దత్తాంశం->boot_state = boot_state;
	ఎంపిక (boot_state) {
	స్థితి AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		ఆపు;
	స్థితి AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		ఆపు;
	స్థితి AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		ఆపు;
	స్థితి AVB_RED:
		ఆపు;
	}

	తిరిగి cmdline;
}

అక్షరం *append_cmd_పంక్తి(అక్షరం *cmdline_orig, అక్షరం *cmdline_new)
{
	అక్షరం *cmd_పంక్తి;

	ఒకవేళ (!cmdline_new)
		తిరిగి cmdline_orig;

	ఒకవేళ (cmdline_orig)
		cmd_పంక్తి = cmdline_orig;
	లేకపోతే
		cmd_పంక్తి = " ";

	cmd_పంక్తి = avb_strdupv(cmd_పంక్తి, " ", cmdline_new, ఖాళీ);

	తిరిగి cmd_పంక్తి;
}

స్థిర పూర్ణాంకం avb_కనుగొను_dm_వాదనలు(అక్షరం **వాదనలు, అక్షరం *str)
{
	పూర్ణాంకం i;

	ఒకవేళ (!str)
		తిరిగి -1;

	కొరకు (i = 0; i < AVB_MAX_వాదనలు && వాదనలు[i]; ++i) {
		ఒకవేళ (అక్షరశ్రేణి_శోధన(వాదనలు[i], str))
			తిరిగి i;
	}

	తిరిగి -1;
}

స్థిర అక్షరం *avb_సమితి_enforce_option(స్థిరరాశి అక్షరం *cmdline, స్థిరరాశి అక్షరం *option)
{
	అక్షరం *cmdarg[AVB_MAX_వాదనలు];
	అక్షరం *newargs = ఖాళీ;
	పూర్ణాంకం i = 0;
	పూర్ణాంకం మొత్తం_వాదనలు;

	memset(cmdarg, 0, పరిమాణం(cmdarg));
	cmdarg[i++] = strtok((అక్షరం *)cmdline, " ");

	చేయి {
		cmdarg[i] = strtok(ఖాళీ, " ");
		ఒకవేళ (!cmdarg[i])
			ఆపు;

		ఒకవేళ (++i >= AVB_MAX_వాదనలు) {
			రాయి("%s: Can't handle more then %d args\n",
			       __func__, i);
			తిరిగి ఖాళీ;
		}
	} అయితే (true);

	మొత్తం_వాదనలు = i;
	i = avb_కనుగొను_dm_వాదనలు(&cmdarg[0], VERITY_పట్టిక_OPT_LOGGING);
	ఒకవేళ (i >= 0) {
		cmdarg[i] = (అక్షరం *)option;
	} లేకపోతే {
		i = avb_కనుగొను_dm_వాదనలు(&cmdarg[0], VERITY_పట్టిక_OPT_RESTART);
		ఒకవేళ (i < 0) {
			రాయి("%s: No verity options found\n", __func__);
			తిరిగి ఖాళీ;
		}

		cmdarg[i] = (అక్షరం *)option;
	}

	కొరకు (i = 0; i <= మొత్తం_వాదనలు; i++)
		newargs = append_cmd_పంక్తి(newargs, cmdarg[i]);

	తిరిగి newargs;
}

అక్షరం *avb_సమితి_ignore_corruption(స్థిరరాశి అక్షరం *cmdline)
{
	అక్షరం *newargs = ఖాళీ;

	newargs = avb_సమితి_enforce_option(cmdline, VERITY_పట్టిక_OPT_LOGGING);
	ఒకవేళ (newargs)
		newargs = append_cmd_పంక్తి(newargs,
					  "androidboot.veritymode=eio");

	తిరిగి newargs;
}

అక్షరం *avb_సమితి_enforce_verity(స్థిరరాశి అక్షరం *cmdline)
{
	అక్షరం *newargs;

	newargs = avb_సమితి_enforce_option(cmdline, VERITY_పట్టిక_OPT_RESTART);
	ఒకవేళ (newargs)
		newargs = append_cmd_పంక్తి(newargs,
					  "androidboot.veritymode=enforcing");
	తిరిగి newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
స్థిర గుర్తులేని దీర్ఘ mmc_read_and_flush(నిర్మాణం mmc_part *part,
					lbaint_t ప్రారంభం,
					lbaint_t sectors,
					శూన్య *బఫరు)
{
	గుర్తులేని దీర్ఘ blks;
	శూన్య *tmp_buf;
	size_t buf_కొలత;
	bool unaligned = is_buf_unaligned(బఫరు);

	ఒకవేళ (ప్రారంభం < part->info.ప్రారంభం) {
		రాయి("%s: partition start out of bounds\n", __func__);
		తిరిగి 0;
	}
	ఒకవేళ ((ప్రారంభం + sectors) > (part->info.ప్రారంభం + part->info.కొలత)) {
		sectors = part->info.ప్రారంభం + part->info.కొలత - ప్రారంభం;
		రాయి("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	ఒకవేళ (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, బఫరు);
		tmp_buf = get_sector_buf();
		buf_కొలత = get_sector_buf_కొలత();
		ఒకవేళ (sectors > buf_కొలత / part->info.blksz)
			sectors = buf_కొలత / part->info.blksz;
	} లేకపోతే {
		tmp_buf = బఫరు;
	}

	blks = blk_dread(part->mmc_blk,
			 ప్రారంభం, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	ఒకవేళ (unaligned)
		జ్ఞాపకం_నకలు(బఫరు, tmp_buf, sectors * part->info.blksz);

	తిరిగి blks;
}

స్థిర గుర్తులేని దీర్ఘ mmc_write(నిర్మాణం mmc_part *part, lbaint_t ప్రారంభం,
			       lbaint_t sectors, శూన్య *బఫరు)
{
	శూన్య *tmp_buf;
	size_t buf_కొలత;
	bool unaligned = is_buf_unaligned(బఫరు);

	ఒకవేళ (ప్రారంభం < part->info.ప్రారంభం) {
		రాయి("%s: partition start out of bounds\n", __func__);
		తిరిగి 0;
	}
	ఒకవేళ ((ప్రారంభం + sectors) > (part->info.ప్రారంభం + part->info.కొలత)) {
		sectors = part->info.ప్రారంభం + part->info.కొలత - ప్రారంభం;
		రాయి("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	ఒకవేళ (unaligned) {
		tmp_buf = get_sector_buf();
		buf_కొలత = get_sector_buf_కొలత();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, బఫరు);
		ఒకవేళ (sectors > buf_కొలత / part->info.blksz)
			sectors = buf_కొలత / part->info.blksz;

		జ్ఞాపకం_నకలు(tmp_buf, బఫరు, sectors * part->info.blksz);
	} లేకపోతే {
		tmp_buf = బఫరు;
	}

	తిరిగి blk_dwrite(part->mmc_blk,
			  ప్రారంభం, sectors, tmp_buf);
}

స్థిర నిర్మాణం mmc_part *get_విభజన(AvbOps *ops, స్థిరరాశి అక్షరం *విభజన)
{
	పూర్ణాంకం ret;
	u8 dev_అంకె;
	పూర్ణాంకం part_అంకె = 0;
	నిర్మాణం mmc_part *part;
	నిర్మాణం blk_desc *mmc_blk;

	part = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం mmc_part));
	ఒకవేళ (!part)
		తిరిగి ఖాళీ;

	dev_అంకె = get_boot_device(ops);
	part->mmc = కనుగొను_mmc_device(dev_అంకె);
	ఒకవేళ (!part->mmc) {
		రాయి("%s: no MMC device at slot %x\n", __func__, dev_అంకె);
		వెళ్ళు err;
	}

	ret = mmc_init(part->mmc);
	ఒకవేళ (ret) {
		రాయి("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		వెళ్ళు err;
	}

	ఒకవేళ (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_అంకె);
		ఒకవేళ (ret) {
			రాయి("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			వెళ్ళు err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	ఒకవేళ (!mmc_blk) {
		రాయి("%s: failed to obtain block descriptor\n", __func__);
		వెళ్ళు err;
	}

	ret = part_get_info_by_name(mmc_blk, విభజన, &part->info);
	ఒకవేళ (ret < 0) {
		రాయి("%s: can't find partition '%s'\n", __func__, విభజన);
		వెళ్ళు err;
	}

	part->dev_అంకె = dev_అంకె;
	part->mmc_blk = mmc_blk;

	తిరిగి part;
err:
	విడుదల(part);
	తిరిగి ఖాళీ;
}

స్థిర AvbIOResult mmc_byte_io(AvbOps *ops,
			       స్థిరరాశి అక్షరం *విభజన,
			       s64 స్థానభ్రంశం,
			       size_t అంకె_బైట్లు,
			       శూన్య *బఫరు,
			       size_t *బహిర్గతం_అంకె_read,
			       గణన mmc_io_type io_type)
{
	ulong ret;
	నిర్మాణం mmc_part *part;
	u64 ప్రారంభం_స్థానభ్రంశం, ప్రారంభం_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_ఫలితం_OK;

	ఒకవేళ (!విభజన || !బఫరు || io_type > IO_WRITE)
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;

	part = get_విభజన(ops, విభజన);
	ఒకవేళ (!part)
		తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విభజన;

	ఒకవేళ (!part->info.blksz) {
		io_ret = AVB_IO_ఫలితం_ERROR_IO;
		వెళ్ళు బహిర్గతం;
	}

	ప్రారంభం_స్థానభ్రంశం = calc_స్థానభ్రంశం(part, స్థానభ్రంశం);
	అయితే (అంకె_బైట్లు) {
		ప్రారంభం_sector = ప్రారంభం_స్థానభ్రంశం / part->info.blksz;
		sectors = అంకె_బైట్లు / part->info.blksz;
		/* handle non block-aligned reads */
		ఒకవేళ (ప్రారంభం_స్థానభ్రంశం % part->info.blksz ||
		    అంకె_బైట్లు < part->info.blksz) {
			tmp_buf = get_sector_buf();
			ఒకవేళ (ప్రారంభం_స్థానభ్రంశం % part->info.blksz) {
				residue = part->info.blksz -
					(ప్రారంభం_స్థానభ్రంశం % part->info.blksz);
				ఒకవేళ (residue > అంకె_బైట్లు)
					residue = అంకె_బైట్లు;
			} లేకపోతే {
				residue = అంకె_బైట్లు;
			}

			ఒకవేళ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.ప్రారంభం +
							 ప్రారంభం_sector,
							 1, tmp_buf);

				ఒకవేళ (ret != 1) {
					రాయి("%s: read error (%ld, %lld)\n",
					       __func__, ret, ప్రారంభం_sector);
					io_ret = AVB_IO_ఫలితం_ERROR_IO;
					వెళ్ళు బహిర్గతం;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (ప్రారంభం_స్థానభ్రంశం % part->info.blksz);
				జ్ఞాపకం_నకలు(బఫరు, (శూన్య *)tmp_buf, residue);
			} లేకపోతే {
				ret = mmc_read_and_flush(part,
							 part->info.ప్రారంభం +
							 ప్రారంభం_sector,
							 1, tmp_buf);

				ఒకవేళ (ret != 1) {
					రాయి("%s: read error (%ld, %lld)\n",
					       __func__, ret, ప్రారంభం_sector);
					io_ret = AVB_IO_ఫలితం_ERROR_IO;
					వెళ్ళు బహిర్గతం;
				}
				జ్ఞాపకం_నకలు((శూన్య *)tmp_buf +
					ప్రారంభం_స్థానభ్రంశం % part->info.blksz,
					బఫరు, residue);

				ret = mmc_write(part, part->info.ప్రారంభం +
						ప్రారంభం_sector, 1, tmp_buf);
				ఒకవేళ (ret != 1) {
					రాయి("%s: write error (%ld, %lld)\n",
					       __func__, ret, ప్రారంభం_sector);
					io_ret = AVB_IO_ఫలితం_ERROR_IO;
					వెళ్ళు బహిర్గతం;
				}
			}

			io_cnt += residue;
			బఫరు += residue;
			ప్రారంభం_స్థానభ్రంశం += residue;
			అంకె_బైట్లు -= residue;
			కొనసాగు;
		}

		ఒకవేళ (sectors) {
			ఒకవేళ (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.ప్రారంభం +
							 ప్రారంభం_sector,
							 sectors, బఫరు);
			} లేకపోతే {
				ret = mmc_write(part,
						part->info.ప్రారంభం +
						ప్రారంభం_sector,
						sectors, బఫరు);
			}

			ఒకవేళ (!ret) {
				రాయి("%s: sector read error\n", __func__);
				io_ret = AVB_IO_ఫలితం_ERROR_IO;
				వెళ్ళు బహిర్గతం;
			}

			io_cnt += ret * part->info.blksz;
			బఫరు += ret * part->info.blksz;
			ప్రారంభం_స్థానభ్రంశం += ret * part->info.blksz;
			అంకె_బైట్లు -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	ఒకవేళ (io_type == IO_READ && బహిర్గతం_అంకె_read)
		*బహిర్గతం_అంకె_read = io_cnt;

బహిర్గతం:
	విడుదల(part);
	తిరిగి io_ret;
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
స్థిర AvbIOResult read_from_విభజన(AvbOps *ops,
				       స్థిరరాశి అక్షరం *విభజన_name,
				       s64 స్థానభ్రంశం_from_విభజన,
				       size_t అంకె_బైట్లు,
				       శూన్య *బఫరు,
				       size_t *బహిర్గతం_అంకె_read)
{
	తిరిగి mmc_byte_io(ops, విభజన_name, స్థానభ్రంశం_from_విభజన,
			   అంకె_బైట్లు, బఫరు, బహిర్గతం_అంకె_read, IO_READ);
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
స్థిర AvbIOResult write_గమ్యం_విభజన(AvbOps *ops,
				      స్థిరరాశి అక్షరం *విభజన_name,
				      s64 స్థానభ్రంశం_from_విభజన,
				      size_t అంకె_బైట్లు,
				      స్థిరరాశి శూన్య *బఫరు)
{
	తిరిగి mmc_byte_io(ops, విభజన_name, స్థానభ్రంశం_from_విభజన,
			   అంకె_బైట్లు, (శూన్య *)బఫరు, ఖాళీ, IO_WRITE);
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
స్థిర AvbIOResult validate_vbmeta_ప్రజా_కీలకం(AvbOps *ops,
					      స్థిరరాశి u8 *ప్రజా_కీలకం_దత్తాంశం,
					      size_t ప్రజా_కీలకం_నిడివి,
					      స్థిరరాశి u8
					      *ప్రజా_కీలకం_metadata,
					      size_t
					      ప్రజా_కీలకం_metadata_నిడివి,
					      bool *బహిర్గతం_కీలకం_is_trusted)
{
	u8 కీలకం_digest[SHA256_కూడిక_LEN];
	u8 trusted_digest[SHA256_కూడిక_LEN];
	AvbIOResult rc;

	ఒకవేళ (!ప్రజా_కీలకం_నిడివి || !ప్రజా_కీలకం_దత్తాంశం || !బహిర్గతం_కీలకం_is_trusted)
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*బహిర్గతం_కీలకం_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(ప్రజా_కీలకం_దత్తాంశం, ప్రజా_కీలకం_నిడివి, కీలకం_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_మూలం_కీలకం_digest(ops, trusted_digest);
	ఒకవేళ (rc != AVB_IO_ఫలితం_OK)
		తిరిగి rc;

	ఒకవేళ (avb_సురక్షిత_memcmp(కీలకం_digest, trusted_digest, పరిమాణం(కీలకం_digest)) == 0)
		*బహిర్గతం_కీలకం_is_trusted = true;

	తిరిగి AVB_IO_ఫలితం_OK;
}

#నిర్వచితమైతే CONFIG_OPTEE_TA_AVB
స్థిర పూర్ణాంకం get_open_session(నిర్మాణం Avb_Ops_దత్తాంశం *ops_దత్తాంశం)
{
	నిర్మాణం udevice *tee = ఖాళీ;

	అయితే (!ops_దత్తాంశం->tee) {
		స్థిరరాశి నిర్మాణం tee_optee_ta_uuid uuid = TA_AVB_UUID;
		నిర్మాణం tee_open_session_arg arg;
		పూర్ణాంకం rc;

		tee = tee_కనుగొను_device(tee, ఖాళీ, ఖాళీ, ఖాళీ);
		ఒకవేళ (!tee)
			తిరిగి -ENODEV;

		memset(&arg, 0, పరిమాణం(arg));
		tee_optee_ta_uuid_గమ్యం_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, ఖాళీ);
		ఒకవేళ (rc || arg.ret)
			కొనసాగు;

		ops_దత్తాంశం->tee = tee;
		ops_దత్తాంశం->session = arg.session;
	}

	తిరిగి 0;
}

స్థిర AvbIOResult invoke_func(నిర్మాణం Avb_Ops_దత్తాంశం *ops_దత్తాంశం, u32 func,
			       ulong అంకె_param, నిర్మాణం tee_param *param)
{
	నిర్మాణం tee_invoke_arg arg;

	ఒకవేళ (get_open_session(ops_దత్తాంశం))
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;

	memset(&arg, 0, పరిమాణం(arg));
	arg.func = func;
	arg.session = ops_దత్తాంశం->session;

	ఒకవేళ (tee_invoke_func(ops_దత్తాంశం->tee, &arg, అంకె_param, param))
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;
	ఎంపిక (arg.ret) {
	స్థితి TEE_SUCCESS:
		తిరిగి AVB_IO_ఫలితం_OK;
	స్థితి TEE_ERROR_బహిర్గతం_యొక్క_జ్ఞాపకం:
		తిరిగి AVB_IO_ఫలితం_ERROR_OOM;
	స్థితి TEE_ERROR_STORAGE_NO_SPACE:
		తిరిగి AVB_IO_ఫలితం_ERROR_INSUFFICIENT_SPACE;
	స్థితి TEE_ERROR_అంశం_NOT_దొరికింది:
		తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విలువ;
	స్థితి TEE_ERROR_లక్ష్యం_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_దత్తాంశం->tee, ops_దత్తాంశం->session);
		ops_దత్తాంశం->tee = ఖాళీ;
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;
	అప్రమేయ:
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;
	}
}
#అయితే_ముగింపు

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
స్థిర AvbIOResult read_rollback_సూచీ(AvbOps *ops,
				       size_t rollback_సూచీ_స్థానం,
				       u64 *బహిర్గతం_rollback_సూచీ)
{
#నిర్వచితం_కాకపోతే CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	ఒకవేళ (బహిర్గతం_rollback_సూచీ)
		*బహిర్గతం_rollback_సూచీ = 0;

	తిరిగి AVB_IO_ఫలితం_OK;
#లేకపోతే
	AvbIOResult rc;
	నిర్మాణం tee_param param[2];

	ఒకవేళ (rollback_సూచీ_స్థానం >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విలువ;

	memset(param, 0, పరిమాణం(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_విలువ_ఆదానం;
	param[0].u.విలువ.a = rollback_సూచీ_స్థానం;
	param[1].attr = TEE_PARAM_ATTR_TYPE_విలువ_OUTPUT;

	rc = invoke_func(ops->user_దత్తాంశం, TA_AVB_CMD_READ_ROLLBACK_సూచీ,
			 ARRAY_కొలత(param), param);
	ఒకవేళ (rc)
		తిరిగి rc;

	*బహిర్గతం_rollback_సూచీ = (u64)param[1].u.విలువ.a << 32 |
			      (u32)param[1].u.విలువ.b;
	తిరిగి AVB_IO_ఫలితం_OK;
#అయితే_ముగింపు
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
స్థిర AvbIOResult write_rollback_సూచీ(AvbOps *ops,
					size_t rollback_సూచీ_స్థానం,
					u64 rollback_సూచీ)
{
#నిర్వచితం_కాకపోతే CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	తిరిగి AVB_IO_ఫలితం_OK;
#లేకపోతే
	నిర్మాణం tee_param param[2];

	ఒకవేళ (rollback_సూచీ_స్థానం >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విలువ;

	memset(param, 0, పరిమాణం(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_విలువ_ఆదానం;
	param[0].u.విలువ.a = rollback_సూచీ_స్థానం;
	param[1].attr = TEE_PARAM_ATTR_TYPE_విలువ_ఆదానం;
	param[1].u.విలువ.a = (u32)(rollback_సూచీ >> 32);
	param[1].u.విలువ.b = (u32)rollback_సూచీ;

	తిరిగి invoke_func(ops->user_దత్తాంశం, TA_AVB_CMD_WRITE_ROLLBACK_సూచీ,
			   ARRAY_కొలత(param), param);
#అయితే_ముగింపు
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
స్థిర AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *బహిర్గతం_is_unlocked)
{
#నిర్వచితం_కాకపోతే CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*బహిర్గతం_is_unlocked = true;

	తిరిగి AVB_IO_ఫలితం_OK;
#లేకపోతే
	AvbIOResult rc;
	నిర్మాణం tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_విలువ_OUTPUT };

	rc = invoke_func(ops->user_దత్తాంశం, TA_AVB_CMD_READ_తాళం_STATE, 1, &param);
	ఒకవేళ (rc)
		తిరిగి rc;
	*బహిర్గతం_is_unlocked = !param.u.విలువ.a;
	తిరిగి AVB_IO_ఫలితం_OK;
#అయితే_ముగింపు
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
స్థిర AvbIOResult get_unique_guid_for_విభజన(AvbOps *ops,
						 స్థిరరాశి అక్షరం *విభజన,
						 అక్షరం *guid_buf,
						 size_t guid_buf_కొలత)
{
	నిర్మాణం mmc_part *part;
	size_t uuid_కొలత;

	part = get_విభజన(ops, విభజన);
	ఒకవేళ (!part)
		తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విభజన;

	uuid_కొలత = పరిమాణం(part->info.uuid);
	ఒకవేళ (uuid_కొలత > guid_buf_కొలత) {
		విడుదల(part);
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;
	}

	జ్ఞాపకం_నకలు(guid_buf, part->info.uuid, uuid_కొలత);
	guid_buf[uuid_కొలత - 1] = 0;

	విడుదల(part);
	తిరిగి AVB_IO_ఫలితం_OK;
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
స్థిర AvbIOResult get_కొలత_యొక్క_విభజన(AvbOps *ops,
					 స్థిరరాశి అక్షరం *విభజన,
					 u64 *బహిర్గతం_కొలత_అంకె_బైట్లు)
{
	నిర్మాణం mmc_part *part;

	ఒకవేళ (!బహిర్గతం_కొలత_అంకె_బైట్లు)
		తిరిగి AVB_IO_ఫలితం_ERROR_INSUFFICIENT_SPACE;

	part = get_విభజన(ops, విభజన);
	ఒకవేళ (!part)
		తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విభజన;

	*బహిర్గతం_కొలత_అంకె_బైట్లు = part->info.blksz * part->info.కొలత;
	విడుదల(part);

	తిరిగి AVB_IO_ఫలితం_OK;
}

#నిర్వచితమైతే CONFIG_OPTEE_TA_AVB
స్థిర AvbIOResult read_persistent_విలువ(AvbOps *ops,
					 స్థిరరాశి అక్షరం *name,
					 size_t బఫరు_కొలత,
					 u8 *బహిర్గతం_బఫరు,
					 size_t *బహిర్గతం_అంకె_బైట్లు_read)
{
	AvbIOResult rc;
	నిర్మాణం tee_shm *shm_name;
	నిర్మాణం tee_shm *shm_buf;
	నిర్మాణం tee_param param[2];
	నిర్మాణం udevice *tee;
	size_t name_కొలత = అక్షరశ్రేణి_పొడవు(name) + 1;

	ఒకవేళ (get_open_session(ops->user_దత్తాంశం))
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;

	tee = ((నిర్మాణం Avb_Ops_దత్తాంశం *)ops->user_దత్తాంశం)->tee;

	rc = tee_shm_alloc(tee, name_కొలత,
			   TEE_SHM_ALLOC, &shm_name);
	ఒకవేళ (rc)
		తిరిగి AVB_IO_ఫలితం_ERROR_OOM;

	rc = tee_shm_alloc(tee, బఫరు_కొలత,
			   TEE_SHM_ALLOC, &shm_buf);
	ఒకవేళ (rc) {
		rc = AVB_IO_ఫలితం_ERROR_OOM;
		వెళ్ళు free_name;
	}

	జ్ఞాపకం_నకలు(shm_name->addr, name, name_కొలత);

	memset(param, 0, పరిమాణం(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ఆదానం;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.కొలత = name_కొలత;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.కొలత = బఫరు_కొలత;

	rc = invoke_func(ops->user_దత్తాంశం, TA_AVB_CMD_READ_PERSIST_విలువ,
			 2, param);
	ఒకవేళ (rc)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (param[1].u.memref.కొలత > బఫరు_కొలత) {
		rc = AVB_IO_ఫలితం_ERROR_NO_SUCH_విలువ;
		వెళ్ళు బహిర్గతం;
	}

	*బహిర్గతం_అంకె_బైట్లు_read = param[1].u.memref.కొలత;

	జ్ఞాపకం_నకలు(బహిర్గతం_బఫరు, shm_buf->addr, *బహిర్గతం_అంకె_బైట్లు_read);

బహిర్గతం:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	తిరిగి rc;
}

స్థిర AvbIOResult write_persistent_విలువ(AvbOps *ops,
					  స్థిరరాశి అక్షరం *name,
					  size_t విలువ_కొలత,
					  స్థిరరాశి u8 *విలువ)
{
	AvbIOResult rc;
	నిర్మాణం tee_shm *shm_name;
	నిర్మాణం tee_shm *shm_buf;
	నిర్మాణం tee_param param[2];
	నిర్మాణం udevice *tee;
	size_t name_కొలత = అక్షరశ్రేణి_పొడవు(name) + 1;

	ఒకవేళ (get_open_session(ops->user_దత్తాంశం))
		తిరిగి AVB_IO_ఫలితం_ERROR_IO;

	tee = ((నిర్మాణం Avb_Ops_దత్తాంశం *)ops->user_దత్తాంశం)->tee;

	ఒకవేళ (!విలువ_కొలత)
		తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విలువ;

	rc = tee_shm_alloc(tee, name_కొలత,
			   TEE_SHM_ALLOC, &shm_name);
	ఒకవేళ (rc)
		తిరిగి AVB_IO_ఫలితం_ERROR_OOM;

	rc = tee_shm_alloc(tee, విలువ_కొలత,
			   TEE_SHM_ALLOC, &shm_buf);
	ఒకవేళ (rc) {
		rc = AVB_IO_ఫలితం_ERROR_OOM;
		వెళ్ళు free_name;
	}

	జ్ఞాపకం_నకలు(shm_name->addr, name, name_కొలత);
	జ్ఞాపకం_నకలు(shm_buf->addr, విలువ, విలువ_కొలత);

	memset(param, 0, పరిమాణం(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ఆదానం;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.కొలత = name_కొలత;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_ఆదానం;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.కొలత = విలువ_కొలత;

	rc = invoke_func(ops->user_దత్తాంశం, TA_AVB_CMD_WRITE_PERSIST_విలువ,
			 2, param);
	ఒకవేళ (rc)
		వెళ్ళు బహిర్గతం;

బహిర్గతం:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	తిరిగి rc;
}
#అయితే_ముగింపు

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#ఒకవేళ CONFIG_IS_ENABLED(AVB_మూలం_కీలకం_BUILTIN)
AvbIOResult avb_read_మూలం_కీలకం_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_మూలం_pub, పరిమాణం(avb_మూలం_pub), digest,
		       CHUNKSZ_SHA256);

	తిరిగి AVB_IO_ఫలితం_OK;
}
#లేదావేళ CONFIG_IS_ENABLED(AVB_మూలం_కీలకం_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_మూలం_కీలకం_digest(AvbOps *ops, uint8_t *digest)
{
	size_t అంకె_read = 0;
	AvbIOResult rc;

	rc = read_persistent_విలువ(ops, CONFIG_AVB_మూలం_కీలకం_TEE_NAME,
				   SHA256_కూడిక_LEN, digest, &అంకె_read);
	ఒకవేళ (rc != AVB_IO_ఫలితం_OK)
		తిరిగి rc;

	ఒకవేళ (అంకె_read != SHA256_కూడిక_LEN)
		తిరిగి AVB_IO_ఫలితం_ERROR_INVALID_విలువ_కొలత;

	తిరిగి AVB_IO_ఫలితం_OK;
}
#లేకపోతే /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_మూలం_కీలకం_digest(AvbOps *ops, uint8_t *digest)
{
	రాయి("%s: board root key provider not implemented\n", __func__);

	తిరిగి AVB_IO_ఫలితం_ERROR_NO_SUCH_విలువ;
}
#అయితే_ముగింపు

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(పూర్ణాంకం boot_device)
{
	నిర్మాణం Avb_Ops_దత్తాంశం *ops_దత్తాంశం;

	ops_దత్తాంశం = avb_calloc(పరిమాణం(నిర్మాణం Avb_Ops_దత్తాంశం));
	ఒకవేళ (!ops_దత్తాంశం)
		తిరిగి ఖాళీ;

	ops_దత్తాంశం->ops.user_దత్తాంశం = ops_దత్తాంశం;

	ops_దత్తాంశం->ops.read_from_విభజన = read_from_విభజన;
	ops_దత్తాంశం->ops.write_గమ్యం_విభజన = write_గమ్యం_విభజన;
	ops_దత్తాంశం->ops.validate_vbmeta_ప్రజా_కీలకం = validate_vbmeta_ప్రజా_కీలకం;
	ops_దత్తాంశం->ops.read_rollback_సూచీ = read_rollback_సూచీ;
	ops_దత్తాంశం->ops.write_rollback_సూచీ = write_rollback_సూచీ;
	ops_దత్తాంశం->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_దత్తాంశం->ops.get_unique_guid_for_విభజన =
		get_unique_guid_for_విభజన;
#నిర్వచితమైతే CONFIG_OPTEE_TA_AVB
	ops_దత్తాంశం->ops.write_persistent_విలువ = write_persistent_విలువ;
	ops_దత్తాంశం->ops.read_persistent_విలువ = read_persistent_విలువ;
#అయితే_ముగింపు
	ops_దత్తాంశం->ops.get_కొలత_యొక్క_విభజన = get_కొలత_యొక్క_విభజన;
	ops_దత్తాంశం->mmc_dev = boot_device;

	తిరిగి &ops_దత్తాంశం->ops;
}

శూన్య avb_ops_free(AvbOps *ops)
{
	నిర్మాణం Avb_Ops_దత్తాంశం *ops_దత్తాంశం;

	ఒకవేళ (!ops)
		తిరిగి;

	ops_దత్తాంశం = ops->user_దత్తాంశం;

	ఒకవేళ (ops_దత్తాంశం) {
#నిర్వచితమైతే CONFIG_OPTEE_TA_AVB
		ఒకవేళ (ops_దత్తాంశం->tee)
			tee_close_session(ops_దత్తాంశం->tee, ops_దత్తాంశం->session);
#అయితే_ముగింపు
		avb_free(ops_దత్తాంశం);
	}
}
