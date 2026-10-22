// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <net.h>
#അന്തര്ഭാവ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

സ്ഥിതം ശൂന്യം append(അക്ഷരം **buf, അക്ഷരം *buf_end, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ log_syslog_emit(ഘടന log_device *ldev, ഘടന log_rec *rec)
{
	പൂർണ്ണസംഖ്യ ret;
	പൂർണ്ണസംഖ്യ fmt = gd->log_fmt;
	അക്ഷരം സന്ദേശം[BUFFER_SIZE];
	അക്ഷരം *സന്ദേശം_end = സന്ദേശം + BUFFER_SIZE;
	അക്ഷരം *ptr = സന്ദേശം;
	അക്ഷരം *iphdr;
	അക്ഷരം *log_സന്ദേശം;
	പൂർണ്ണസംഖ്യ eth_hdr_size;
	ഘടന in_addr bcast_ip;
	unsigned പൂർണ്ണസംഖ്യ log_level;
	അക്ഷരം *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	എങ്കിൽ (ret)
		തിരികെ ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	എങ്കിൽ (ret < 0) {
		eth_halt();
		പോകുക out;
	}

	memset(സന്ദേശം, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_സന്ദേശം = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	എങ്കിൽ (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, സന്ദേശം_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	എങ്കിൽ (log_hostname)
		append(&ptr, സന്ദേശം_end, "%s ", log_hostname);
	append(&ptr, സന്ദേശം_end, "uboot: ");
	എങ്കിൽ (fmt & BIT(LOGF_LEVEL))
		append(&ptr, സന്ദേശം_end, "%s.",
		       log_get_level_name(rec->level));
	എങ്കിൽ (fmt & BIT(LOGF_CAT))
		append(&ptr, സന്ദേശം_end, "%s,",
		       log_get_cat_name(rec->cat));
	എങ്കിൽ (fmt & BIT(LOGF_FILE))
		append(&ptr, സന്ദേശം_end, "%s:", rec->file);
	എങ്കിൽ (fmt & BIT(LOGF_വരി))
		append(&ptr, സന്ദേശം_end, "%d-", rec->വരി);
	എങ്കിൽ (fmt & BIT(LOGF_FUNC))
		append(&ptr, സന്ദേശം_end, "%s()", rec->func ?: "?");
	എങ്കിൽ (fmt & BIT(LOGF_സന്ദേശം))
		append(&ptr, സന്ദേശം_end, "%s%s",
		       fmt != BIT(LOGF_സന്ദേശം) ? " " : "", rec->സന്ദേശം);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_സന്ദേശം);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_സന്ദേശം);
	net_send_packet((uchar *)സന്ദേശം, ptr - സന്ദേശം);

out:
	തിരികെ ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};
