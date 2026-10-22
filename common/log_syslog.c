// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <net.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

#ವ್ಯಾಖ್ಯಾನಿಸು ಬಫರ್_ಗಾತ್ರ_2 480

ಸ್ಥಿರ ಶೂನ್ಯ append(ಅಕ್ಷರ **buf, ಅಕ್ಷರ *buf_ಕೊನೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	va_list ವಾದಗಳು;
	size_t ಗಾತ್ರ_2 = buf_ಕೊನೆ - *buf;

	va_ಆರಂಭ(ವಾದಗಳು, fmt);
	vsnprintf(*buf, ಗಾತ್ರ_2, fmt, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);
	*buf += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*buf);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ log_syslog_emit(ರಚನೆ log_device *ldev, ರಚನೆ log_rec *rec)
{
	ಪೂರ್ಣಾಂಕ ret;
	ಪೂರ್ಣಾಂಕ fmt = gd->log_fmt;
	ಅಕ್ಷರ ಸಂದೇಶ[ಬಫರ್_ಗಾತ್ರ_2];
	ಅಕ್ಷರ *ಸಂದೇಶ_ಕೊನೆ = ಸಂದೇಶ + ಬಫರ್_ಗಾತ್ರ_2;
	ಅಕ್ಷರ *ptr = ಸಂದೇಶ;
	ಅಕ್ಷರ *iphdr;
	ಅಕ್ಷರ *log_ಸಂದೇಶ;
	ಪೂರ್ಣಾಂಕ eth_hdr_ಗಾತ್ರ;
	ರಚನೆ ಒಳ_addr bcast_ip;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ log_level;
	ಅಕ್ಷರ *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_ಗಣ_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	ಆದರೆ (ret < 0) {
		eth_halt();
		ಹೋಗು ಹೊರಹೋಗು;
	}

	memset(ಸಂದೇಶ, 0, ಬಫರ್_ಗಾತ್ರ_2);

	/* Set ethernet header */
	eth_hdr_ಗಾತ್ರ = net_ಗಣ_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_ಗಾತ್ರ;
	iphdr = ptr;
	ptr += IP_UDP_HDR_ಗಾತ್ರ;
	log_ಸಂದೇಶ = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	ಆದರೆ (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	ಆದರೆ (log_hostname)
		append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "%s ", log_hostname);
	append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "uboot: ");
	ಆದರೆ (fmt & ಬಿಟ್(LOGF_LEVEL))
		append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "%s.",
		       log_get_level_name(rec->level));
	ಆದರೆ (fmt & ಬಿಟ್(LOGF_CAT))
		append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "%s,",
		       log_get_cat_name(rec->cat));
	ಆದರೆ (fmt & ಬಿಟ್(LOGF_FILE))
		append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "%s:", rec->file);
	ಆದರೆ (fmt & ಬಿಟ್(LOGF_ಸಾಲು))
		append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "%d-", rec->ಸಾಲು);
	ಆದರೆ (fmt & ಬಿಟ್(LOGF_FUNC))
		append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "%s()", rec->func ?: "?");
	ಆದರೆ (fmt & ಬಿಟ್(LOGF_ಸಂದೇಶ))
		append(&ptr, ಸಂದೇಶ_ಕೊನೆ, "%s%s",
		       fmt != ಬಿಟ್(LOGF_ಸಂದೇಶ) ? " " : "", rec->ಸಂದೇಶ);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_ಸಂದೇಶ);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_ಗಣ_udp_ಶೀರ್ಷಿಕೆ((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_ಸಂದೇಶ);
	net_send_packet((uchar *)ಸಂದೇಶ, ptr - ಸಂದೇಶ);

ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
