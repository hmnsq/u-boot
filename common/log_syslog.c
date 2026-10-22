// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <net.h>
#અન્તર્ભાવ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

સ્થિર શૂન્ય append(અક્ષર **buf, અક્ષર *buf_end, અચલ અક્ષર *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

સ્થિર પૂર્ણાંક log_syslog_emit(સંરચના log_device *ldev, સંરચના log_rec *rec)
{
	પૂર્ણાંક ret;
	પૂર્ણાંક fmt = gd->log_fmt;
	અક્ષર સંદેશ[BUFFER_SIZE];
	અક્ષર *સંદેશ_end = સંદેશ + BUFFER_SIZE;
	અક્ષર *ptr = સંદેશ;
	અક્ષર *iphdr;
	અક્ષર *log_સંદેશ;
	પૂર્ણાંક eth_hdr_size;
	સંરચના in_addr bcast_ip;
	unsigned પૂર્ણાંક log_level;
	અક્ષર *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	જો (ret)
		પાછા ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	જો (ret < 0) {
		eth_halt();
		જાઓ out;
	}

	memset(સંદેશ, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_સંદેશ = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	જો (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, સંદેશ_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	જો (log_hostname)
		append(&ptr, સંદેશ_end, "%s ", log_hostname);
	append(&ptr, સંદેશ_end, "uboot: ");
	જો (fmt & BIT(LOGF_LEVEL))
		append(&ptr, સંદેશ_end, "%s.",
		       log_get_level_name(rec->level));
	જો (fmt & BIT(LOGF_CAT))
		append(&ptr, સંદેશ_end, "%s,",
		       log_get_cat_name(rec->cat));
	જો (fmt & BIT(LOGF_FILE))
		append(&ptr, સંદેશ_end, "%s:", rec->file);
	જો (fmt & BIT(LOGF_લીટી))
		append(&ptr, સંદેશ_end, "%d-", rec->લીટી);
	જો (fmt & BIT(LOGF_FUNC))
		append(&ptr, સંદેશ_end, "%s()", rec->func ?: "?");
	જો (fmt & BIT(LOGF_સંદેશ))
		append(&ptr, સંદેશ_end, "%s%s",
		       fmt != BIT(LOGF_સંદેશ) ? " " : "", rec->સંદેશ);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_સંદેશ);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_સંદેશ);
	net_send_packet((uchar *)સંદેશ, ptr - સંદેશ);

out:
	પાછા ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
