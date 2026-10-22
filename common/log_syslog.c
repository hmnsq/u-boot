// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#अन्तर्भाव <env.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <net.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

स्थिर शून्य append(वर्ण **buf, वर्ण *buf_end, ध्रुव वर्ण *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

स्थिर पूर्णाङ्क log_syslog_emit(संरचना log_device *ldev, संरचना log_rec *rec)
{
	पूर्णाङ्क ret;
	पूर्णाङ्क fmt = gd->log_fmt;
	वर्ण संदेश[BUFFER_SIZE];
	वर्ण *संदेश_end = संदेश + BUFFER_SIZE;
	वर्ण *ptr = संदेश;
	वर्ण *iphdr;
	वर्ण *log_संदेश;
	पूर्णाङ्क eth_hdr_size;
	संरचना in_addr bcast_ip;
	unsigned पूर्णाङ्क log_level;
	वर्ण *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	यदि (ret)
		प्रत्यागम ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	यदि (ret < 0) {
		eth_halt();
		गच्छ out;
	}

	memset(संदेश, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_संदेश = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	यदि (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, संदेश_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	यदि (log_hostname)
		append(&ptr, संदेश_end, "%s ", log_hostname);
	append(&ptr, संदेश_end, "uboot: ");
	यदि (fmt & BIT(LOGF_LEVEL))
		append(&ptr, संदेश_end, "%s.",
		       log_get_level_name(rec->level));
	यदि (fmt & BIT(LOGF_CAT))
		append(&ptr, संदेश_end, "%s,",
		       log_get_cat_name(rec->cat));
	यदि (fmt & BIT(LOGF_FILE))
		append(&ptr, संदेश_end, "%s:", rec->file);
	यदि (fmt & BIT(LOGF_पंक्ति))
		append(&ptr, संदेश_end, "%d-", rec->पंक्ति);
	यदि (fmt & BIT(LOGF_FUNC))
		append(&ptr, संदेश_end, "%s()", rec->func ?: "?");
	यदि (fmt & BIT(LOGF_संदेश))
		append(&ptr, संदेश_end, "%s%s",
		       fmt != BIT(LOGF_संदेश) ? " " : "", rec->संदेश);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_संदेश);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_संदेश);
	net_send_packet((uchar *)संदेश, ptr - संदेश);

out:
	प्रत्यागम ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
