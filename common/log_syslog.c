// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#அந்தர்பாவ <env.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <net.h>
#அந்தர்பாவ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

நிலையான வெறுமை append(எழுத்து **buf, எழுத்து *buf_end, மாறிலி எழுத்து *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

நிலையான முழு log_syslog_emit(கட்டமைப்பு log_device *ldev, கட்டமைப்பு log_rec *rec)
{
	முழு ret;
	முழு fmt = gd->log_fmt;
	எழுத்து செய்தி[BUFFER_SIZE];
	எழுத்து *செய்தி_end = செய்தி + BUFFER_SIZE;
	எழுத்து *ptr = செய்தி;
	எழுத்து *iphdr;
	எழுத்து *log_செய்தி;
	முழு eth_hdr_size;
	கட்டமைப்பு in_addr bcast_ip;
	unsigned முழு log_level;
	எழுத்து *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	எனில் (ret)
		திரும்பு ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	எனில் (ret < 0) {
		eth_halt();
		செல் out;
	}

	memset(செய்தி, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_செய்தி = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	எனில் (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, செய்தி_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	எனில் (log_hostname)
		append(&ptr, செய்தி_end, "%s ", log_hostname);
	append(&ptr, செய்தி_end, "uboot: ");
	எனில் (fmt & BIT(LOGF_LEVEL))
		append(&ptr, செய்தி_end, "%s.",
		       log_get_level_name(rec->level));
	எனில் (fmt & BIT(LOGF_CAT))
		append(&ptr, செய்தி_end, "%s,",
		       log_get_cat_name(rec->cat));
	எனில் (fmt & BIT(LOGF_FILE))
		append(&ptr, செய்தி_end, "%s:", rec->file);
	எனில் (fmt & BIT(LOGF_வரி))
		append(&ptr, செய்தி_end, "%d-", rec->வரி);
	எனில் (fmt & BIT(LOGF_FUNC))
		append(&ptr, செய்தி_end, "%s()", rec->func ?: "?");
	எனில் (fmt & BIT(LOGF_செய்தி))
		append(&ptr, செய்தி_end, "%s%s",
		       fmt != BIT(LOGF_செய்தி) ? " " : "", rec->செய்தி);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_செய்தி);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_செய்தி);
	net_send_packet((uchar *)செய்தி, ptr - செய்தி);

out:
	திரும்பு ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
