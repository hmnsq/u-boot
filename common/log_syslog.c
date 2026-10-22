// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ضمن <env.h>
#ضمن <log.h>
#ضمن <net.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

ساكن فراغ append(حرف **buf, حرف *buf_end, ثابت حرف *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

ساكن صحيح log_syslog_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح ret;
	صحيح fmt = gd->log_fmt;
	حرف ڤسنن[BUFFER_SIZE];
	حرف *ڤسنن_end = ڤسنن + BUFFER_SIZE;
	حرف *ptr = ڤسنن;
	حرف *iphdr;
	حرف *log_ڤسنن;
	صحيح eth_hdr_size;
	بنية in_addr bcast_ip;
	unsigned صحيح log_level;
	حرف *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	إذا (ret)
		أرجع ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	إذا (ret < 0) {
		eth_halt();
		اذهب out;
	}

	memset(ڤسنن, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_ڤسنن = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	إذا (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, ڤسنن_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	إذا (log_hostname)
		append(&ptr, ڤسنن_end, "%s ", log_hostname);
	append(&ptr, ڤسنن_end, "uboot: ");
	إذا (fmt & BIT(LOGF_LEVEL))
		append(&ptr, ڤسنن_end, "%s.",
		       log_get_level_name(rec->level));
	إذا (fmt & BIT(LOGF_CAT))
		append(&ptr, ڤسنن_end, "%s,",
		       log_get_cat_name(rec->cat));
	إذا (fmt & BIT(LOGF_FILE))
		append(&ptr, ڤسنن_end, "%s:", rec->file);
	إذا (fmt & BIT(LOGF_باريس))
		append(&ptr, ڤسنن_end, "%d-", rec->باريس);
	إذا (fmt & BIT(LOGF_FUNC))
		append(&ptr, ڤسنن_end, "%s()", rec->func ?: "?");
	إذا (fmt & BIT(LOGF_ڤسنن))
		append(&ptr, ڤسنن_end, "%s%s",
		       fmt != BIT(LOGF_ڤسنن) ? " " : "", rec->ڤسنن);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_ڤسنن);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_ڤسنن);
	net_send_packet((uchar *)ڤسنن, ptr - ڤسنن);

out:
	أرجع ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
