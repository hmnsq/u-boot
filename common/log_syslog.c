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
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

static số_nguyên log_syslog_emit(struct log_device *ldev, struct log_rec *rec)
{
	số_nguyên ret;
	số_nguyên fmt = gd->log_fmt;
	char msg[BUFFER_SIZE];
	char *msg_end = msg + BUFFER_SIZE;
	char *ptr = msg;
	char *iphdr;
	char *log_msg;
	số_nguyên eth_hdr_size;
	struct in_addr bcast_ip;
	unsigned số_nguyên log_level;
	char *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	nếu (ret)
		trả_về ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	nếu (ret < 0) {
		eth_halt();
		goto out;
	}

	memset(msg, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_msg = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	nếu (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, msg_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	nếu (log_hostname)
		append(&ptr, msg_end, "%s ", log_hostname);
	append(&ptr, msg_end, "uboot: ");
	nếu (fmt & BIT(LOGF_LEVEL))
		append(&ptr, msg_end, "%s.",
		       log_get_level_name(rec->level));
	nếu (fmt & BIT(LOGF_CAT))
		append(&ptr, msg_end, "%s,",
		       log_get_cat_name(rec->cat));
	nếu (fmt & BIT(LOGF_FILE))
		append(&ptr, msg_end, "%s:", rec->file);
	nếu (fmt & BIT(LOGF_LINE))
		append(&ptr, msg_end, "%d-", rec->line);
	nếu (fmt & BIT(LOGF_FUNC))
		append(&ptr, msg_end, "%s()", rec->func ?: "?");
	nếu (fmt & BIT(LOGF_MSG))
		append(&ptr, msg_end, "%s%s",
		       fmt != BIT(LOGF_MSG) ? " " : "", rec->msg);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_msg);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_msg);
	net_send_packet((uchar *)msg, ptr - msg);

out:
	trả_về ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
