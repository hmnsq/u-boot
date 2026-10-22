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

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

ಸ್ಥಿರ ಶೂನ್ಯ append(ವರ್ಣ **buf, ವರ್ಣ *buf_end, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ log_syslog_emit(ಸಂರಚನಾ log_device *ldev, ಸಂರಚನಾ log_rec *rec)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	ಪೂರ್ಣಾಙ್ಕ fmt = gd->log_fmt;
	ವರ್ಣ ಸಂದೇಶ[BUFFER_SIZE];
	ವರ್ಣ *ಸಂದೇಶ_end = ಸಂದೇಶ + BUFFER_SIZE;
	ವರ್ಣ *ptr = ಸಂದೇಶ;
	ವರ್ಣ *iphdr;
	ವರ್ಣ *log_ಸಂದೇಶ;
	ಪೂರ್ಣಾಙ್ಕ eth_hdr_size;
	ಸಂರಚನಾ in_addr bcast_ip;
	unsigned ಪೂರ್ಣಾಙ್ಕ log_level;
	ವರ್ಣ *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	ಯದಿ (ret < 0) {
		eth_halt();
		ಗಚ್ಛ out;
	}

	memset(ಸಂದೇಶ, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_ಸಂದೇಶ = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	ಯದಿ (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, ಸಂದೇಶ_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	ಯದಿ (log_hostname)
		append(&ptr, ಸಂದೇಶ_end, "%s ", log_hostname);
	append(&ptr, ಸಂದೇಶ_end, "uboot: ");
	ಯದಿ (fmt & BIT(LOGF_LEVEL))
		append(&ptr, ಸಂದೇಶ_end, "%s.",
		       log_get_level_name(rec->level));
	ಯದಿ (fmt & BIT(LOGF_CAT))
		append(&ptr, ಸಂದೇಶ_end, "%s,",
		       log_get_cat_name(rec->cat));
	ಯದಿ (fmt & BIT(LOGF_FILE))
		append(&ptr, ಸಂದೇಶ_end, "%s:", rec->file);
	ಯದಿ (fmt & BIT(LOGF_ಸಾಲು))
		append(&ptr, ಸಂದೇಶ_end, "%d-", rec->ಸಾಲು);
	ಯದಿ (fmt & BIT(LOGF_FUNC))
		append(&ptr, ಸಂದೇಶ_end, "%s()", rec->func ?: "?");
	ಯದಿ (fmt & BIT(LOGF_ಸಂದೇಶ))
		append(&ptr, ಸಂದೇಶ_end, "%s%s",
		       fmt != BIT(LOGF_ಸಂದೇಶ) ? " " : "", rec->ಸಂದೇಶ);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_ಸಂದೇಶ);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_ಸಂದೇಶ);
	net_send_packet((uchar *)ಸಂದೇಶ, ptr - ಸಂದೇಶ);

out:
	ಪ್ರತ್ಯಾಗಮ ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
