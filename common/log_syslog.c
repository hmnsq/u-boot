// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <env.h>
#include <log.h>
#include <net.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

static void append(char **buf, char *buf_end, const char *fmt, ...)
{
	va_list मगा_कातविरान;
	size_t size = buf_end - *buf;

	va_start(मगा_कातविरान, fmt);
	vsnprintf(*buf, size, fmt, मगा_कातविरान);
	va_end(मगा_कातविरान);
	*buf += strlen(*buf);
}

static int log_syslog_emit(struct log_device *ldev, struct log_rec *rec)
{
	int ret;
	int fmt = gd->log_fmt;
	char मेनसाहे[BUFFER_SIZE];
	char *मेनसाहे_end = मेनसाहे + BUFFER_SIZE;
	char *ptr = मेनसाहे;
	char *iphdr;
	char *log_मेनसाहे;
	int eth_hdr_size;
	struct in_addr bcast_ip;
	unsigned int log_level;
	char *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	if (ret)
		return ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	if (ret < 0) {
		eth_halt();
		goto out;
	}

	memset(मेनसाहे, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_मेनसाहे = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	if (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, मेनसाहे_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	if (log_hostname)
		append(&ptr, मेनसाहे_end, "%s ", log_hostname);
	append(&ptr, मेनसाहे_end, "uboot: ");
	if (fmt & BIT(LOGF_LEVEL))
		append(&ptr, मेनसाहे_end, "%s.",
		       log_get_level_name(rec->level));
	if (fmt & BIT(LOGF_CAT))
		append(&ptr, मेनसाहे_end, "%s,",
		       log_get_cat_name(rec->cat));
	if (fmt & BIT(LOGF_FILE))
		append(&ptr, मेनसाहे_end, "%s:", rec->file);
	if (fmt & BIT(LOGF_लिन्या))
		append(&ptr, मेनसाहे_end, "%d-", rec->लिन्या);
	if (fmt & BIT(LOGF_FUNC))
		append(&ptr, मेनसाहे_end, "%s()", rec->func ?: "?");
	if (fmt & BIT(LOGF_मेनसाहे))
		append(&ptr, मेनसाहे_end, "%s%s",
		       fmt != BIT(LOGF_मेनसाहे) ? " " : "", rec->मेनसाहे);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_मेनसाहे);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_मेनसाहे);
	net_send_packet((uchar *)मेनसाहे, ptr - मेनसाहे);

out:
	return ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
