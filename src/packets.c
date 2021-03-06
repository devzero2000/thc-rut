/*
 * $Id: packets.c,v 1.7 2003/05/23 14:14:01 skyper Exp $
 */

#include "default.h"
#include <libnet.h>
#include <time.h>
#include "nmap_compat.h"
#include "fp.h"
#include "thc-rut.h"
#include "dhcp.h"

extern struct _opt opt;
extern uint8_t ip_tcp_sync[];
extern uint8_t ip_tcp_fp[];
extern uint8_t ip_udp_dcebind[];
extern uint8_t ip_udp_snmp[];
extern uint8_t ip_icmp_echo[];
extern unsigned short ip_tcp_sync_chksum;
extern unsigned short ip_tcp_fp_chksum;

/*
 * Pre-generate packets for the scanner.
 * opt.src_ip is 0.0.0.0 and filled by the kernel.
 * (We dont know the src_ip yet).
 */
void
scanner_gen_packets(void)
{
	struct tcphdr *tcp;
	struct udphdr *udp;
	struct icmp *icmp;

	tcp = (struct tcphdr *)ip_tcp_sync;
	tcp->th_sport = htons(opt.src_port);
	tcp->th_dport = htons(80);
	tcp->th_seq = htonl(0xffffffff - time(NULL));
	tcp->th_flags = TH_SYN; //syn = 1;
	tcp->th_win = 5840;
	tcp->th_sum = 0;

	/* Port wrapps around every 4096 seconds */
#if 0
	libnet_build_tcp(opt.src_port,
			80,
			0xffffffff - time(NULL),
			0x0,
			TH_SYN,
			5840,
			0,
			NULL,
			0, ip_tcp_sync);
#endif
#if 0
	libnet_build_ip(LIBNET_TCP_H,
			IPTOS_RELIABILITY,
			opt.ip_id,  /* for outgoing ip's only */
			0,
			255,
			IPPROTO_TCP,
			opt.src_ip,
			0 /*dst*/,
			NULL,
			0,
			ip_tcp_sync);
	libnet_do_checksum(ip_tcp_sync, IPPROTO_TCP, LIBNET_TCP_H);
	ip_tcp_sync_chksum = *(unsigned short *)(ip_tcp_sync + 36);
#endif

	tcp = (struct tcphdr *)ip_tcp_fp;
	tcp->th_sport = htons(opt.src_port + 1);
	tcp->th_dport = htons(80);
	tcp->th_seq = htonl(0xffffffff - time(NULL) + 1);
	tcp->th_flags = TH_SYN;
	tcp->th_win = 5840;
	tcp->th_sum = 0;
#if 0
	libnet_build_tcp(opt.src_port + 1,
			0,
			0xffffffff - time(NULL) + 1,
			0x0,
			TH_SYN,
			5840,
			0,
			NULL,
			0, ip_tcp_fp);
	tcp = (struct tcphdr *)(ip_tcp_fp);
#endif
	tcp->th_off = (20 + NMAP_FP_TONE_LEN) >> 2;
	memcpy(ip_tcp_fp + 20, NMAP_FP_TONE, NMAP_FP_TONE_LEN);
#if 0
	libnet_build_ip(LIBNET_TCP_H + NMAP_FP_TONE_LEN,
			IPTOS_RELIABILITY,
			opt.ip_id,  /* for outgoing ip's only */
			0,
			255,
			IPPROTO_TCP,
			opt.src_ip,
			0 /*dst*/,
			NULL,
			0,
			ip_tcp_fp);
#endif

	udp = (struct udphdr *)ip_udp_dcebind;
	udp->uh_sport = htons(opt.src_port + 1);
	udp->uh_ulen = htons(FP_DCEBIND_LEN);
	memcpy(ip_udp_dcebind + 8, FP_DCEBIND, FP_DCEBIND_LEN);

#if 0
	libnet_build_udp(opt.src_port + 1,
			0,
			NULL,
			FP_DCEBIND_LEN,
			ip_udp_dcebind);
#endif
#if 0
	libnet_build_ip(LIBNET_UDP_H + FP_DCEBIND_LEN,
			IPTOS_RELIABILITY,
			opt.ip_id,  /* for outgoing ip's only */
			0,
			255,
			IPPROTO_UDP,
			opt.src_ip,
			0 /*dst*/,
			NULL,
			0,
			ip_udp_dcebind);
#endif

	udp = (struct udphdr *)ip_udp_snmp;
	udp->uh_sport = htons(opt.src_port + 1);
	udp->uh_dport = htons(161);
	udp->uh_ulen = htons(FP_SNMP_LEN);
#if 0	
	libnet_build_udp(opt.src_port + 1,
			161,
			NULL,
			FP_SNMP_LEN,
			ip_udp_snmp);
#endif
	memcpy(ip_udp_snmp + LIBNET_UDP_H, FP_SNMP, FP_SNMP_LEN);
#if 0
	libnet_build_ip(LIBNET_UDP_H + FP_SNMP_LEN,
			IPTOS_RELIABILITY,
			opt.ip_id,  /* for outgoing ip's only */
			0,
			255,
			IPPROTO_UDP,
			opt.src_ip,
			0 /*dst*/,
			NULL,
			0,
			ip_udp_snmp);
#endif

	icmp = (struct icmp *)ip_icmp_echo;
	icmp->icmp_type = ICMP_ECHO;		/* 8 */
	//icmp->un.echo.id = 31338; //htons(getpid());
	icmp->icmp_hun.ih_idseq.icd_id = opt.ic_id;
#if 0
	libnet_build_icmp_echo(8,
			0,
			htons(getpid()), /* we match for this ID! */
			1,
			NULL,
			0,
			ip_icmp_echo);
#endif
#if 0
	libnet_build_ip(8,
			IPTOS_RELIABILITY,
			opt.ip_id,  /* for outgoing ip's only */
			0,
			255,
			IPPROTO_ICMP,
			opt.src_ip,
			0 /*dst*/,
			NULL,
			0,
			ip_icmp_echo);
#endif

}

void
dhcp_gen_packets(uint8_t *packet, uint32_t srcip, uint8_t *dsbuf, struct _dhcpset *ds)
{
#if 0
	int len;

	libnet_build_udp(68,
			67,
			NULL,
			datalen - 8,  /* length */
			packet + LIBNET_ETH_H + LIBNET_IP_H);

	libnet_build_ip(datalen,  /* length of ip DATA */
			0,
			7350,
			0,
			128,
			IPPROTO_UDP,
			srcip,  /* source IP      */
			-1,     /* Destination IP */
			NULL,
			0,
			packet + LIBNET_ETH_H);

	libnet_build_ethernet(ETHBCAST,
			"\x00\x00\x00\x00\x00\x00",
			ETHERTYPE_IP,
			NULL,
			0,
			packet);
#endif

	build_bootp(packet);
	dhcp_add_option(ds, DHCP_END, 0, NULL);
	memcpy(packet + sizeof(struct _bootp), dsbuf, ds->lsize);
}

void
arp_gen_packets(unsigned int srcip)
{
#if 0
	opt.ln_arp = libnet_build_arp(ARPHRD_ETHER,
			ETHERTYPE_IP,
			6,
			4,
			ARPOP_REQUEST,
			ETHZCAST,
			(unsigned char *)&srcip,
			ETHBCAST,
			"\x00\x00\x00\x00", /* IP */
			NULL,
			0,
			opt.ln_ctx, opt.ln_arp);

	opt.ln_eth = libnet_build_ethernet(ETHBCAST,
			ETHZCAST,
			ETHERTYPE_ARP,
			NULL,
			0,
			opt.ln_ctx, opt.ln_eth);
#endif
}

