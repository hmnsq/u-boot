// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਫਰ_ਮਾਪ_2 480

ਸਥਿਰ ਖਾਲੀ append(ਅੱਖਰ **buf, ਅੱਖਰ *buf_ਸਿਰਾ, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	va_list ਦਲੀਲਾਂ;
	size_t ਮਾਪ = buf_ਸਿਰਾ - *buf;

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);
	vsnprintf(*buf, ਮਾਪ, fmt, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);
	*buf += ਲੜੀ_ਲੰਬਾਈ(*buf);
}

ਸਥਿਰ ਪੂਰਨ log_syslog_emit(ਬਣਤਰ log_device *ldev, ਬਣਤਰ log_rec *rec)
{
	ਪੂਰਨ ret;
	ਪੂਰਨ fmt = gd->log_fmt;
	ਅੱਖਰ ਸੁਨੇਹਾ[ਬਫਰ_ਮਾਪ_2];
	ਅੱਖਰ *ਸੁਨੇਹਾ_ਸਿਰਾ = ਸੁਨੇਹਾ + ਬਫਰ_ਮਾਪ_2;
	ਅੱਖਰ *ptr = ਸੁਨੇਹਾ;
	ਅੱਖਰ *iphdr;
	ਅੱਖਰ *log_ਸੁਨੇਹਾ;
	ਪੂਰਨ eth_hdr_ਮਾਪ;
	ਬਣਤਰ ਵਿੱਚ_addr bcast_ip;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ log_level;
	ਅੱਖਰ *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	ਜੇ (ret)
		ਵਾਪਸ ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_ਸੈੱਟ_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	ਜੇ (ret < 0) {
		eth_halt();
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	memset(ਸੁਨੇਹਾ, 0, ਬਫਰ_ਮਾਪ_2);

	/* Set ethernet header */
	eth_hdr_ਮਾਪ = net_ਸੈੱਟ_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_ਮਾਪ;
	iphdr = ptr;
	ptr += IP_UDP_HDR_ਮਾਪ;
	log_ਸੁਨੇਹਾ = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	ਜੇ (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	ਜੇ (log_hostname)
		append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "%s ", log_hostname);
	append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "uboot: ");
	ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_LEVEL))
		append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "%s.",
		       log_get_level_name(rec->level));
	ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_CAT))
		append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "%s,",
		       log_get_cat_name(rec->cat));
	ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_FILE))
		append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "%s:", rec->file);
	ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_ਸਤਰ))
		append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "%d-", rec->ਸਤਰ);
	ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_FUNC))
		append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "%s()", rec->func ?: "?");
	ਜੇ (fmt & ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_ਸੁਨੇਹਾ))
		append(&ptr, ਸੁਨੇਹਾ_ਸਿਰਾ, "%s%s",
		       fmt != ਦੋਆਧਾਰੀ_ਅੰਕ(LOGF_ਸੁਨੇਹਾ) ? " " : "", rec->ਸੁਨੇਹਾ);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_ਸੁਨੇਹਾ);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_ਸੈੱਟ_udp_ਸਿਰਲੇਖ((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_ਸੁਨੇਹਾ);
	net_send_packet((uchar *)ਸੁਨੇਹਾ, ptr - ਸੁਨੇਹਾ);

ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
