//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		:
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <poll.h>

//
//	From FreeBSD's ping.c
//
static uint16_t MakeCheckSum( uint16_t *addr, int len )
{
	register int nleft = len;
	register uint16_t *w = addr;
	register int sum = 0;
	uint16_t answer = 0;

	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return(answer);
}

//
//	Initilaize ICMP Packet
//
static int32_t InitIcmpPacket(char *buf, int32_t querytype)
{
	struct in_addr fromaddr;
	struct ip *ip = (struct ip *)buf;
	struct icmp *icmp = (struct icmp *)(ip + 1);
	int32_t icmplen = 0;

	memset( &fromaddr, 0, sizeof(fromaddr) );

	ip->ip_src = fromaddr;	/* kernel fills in */
	ip->ip_v = 4;
	ip->ip_hl = sizeof *ip >> 2;
	ip->ip_tos = 0;
	ip->ip_id = htons(4321);
	ip->ip_ttl = 255;
	ip->ip_p = 1;
	ip->ip_sum = 0;			/* kernel fills in */

	memset( icmp, 0xAA, ICMP_MINLEN );
	icmp->icmp_seq = 1;
	icmp->icmp_cksum = 0;
	icmp->icmp_type = querytype;
	icmp->icmp_code = 0;

	switch(querytype) {
	case ICMP_TSTAMP:
		gettimeofday( (struct timeval *)(icmp+8), NULL);
		bzero( icmp+12, 8 );
		icmplen = 20;
		break;
	case ICMP_MASKREQ:
		*((char *)(icmp+8)) = 255;
		icmplen = 12;
		break;
	case ICMP_ECHO:
		icmplen = ICMP_MINLEN;
		break;
	default:
		return -1;
	}
	ip->ip_len = sizeof(struct ip) + icmplen;
	return icmplen;
}


static int32_t SendPing( int sock, int querytype, const char *hostname )
{
	char buf[1500];
	struct ip *ip = (struct ip *)buf;
	struct icmp *icmp = (struct icmp *)(ip + 1);
	struct sockaddr_in dst;
	int icmplen;
	struct in_addr tmpaddr;
	struct hostent *hp;

	if( (hp = gethostbyname(hostname)) == NULL )
	{
		tmpaddr.s_addr = inet_addr(hostname);
	}
	else
	{
		bcopy(hp->h_addr_list[0], &tmpaddr.s_addr, hp->h_length);
	}

	bzero(buf, 1500);
	icmplen = InitIcmpPacket(buf, querytype );

	if( icmplen < 0 )
		return -1;

	dst.sin_family = AF_INET;
	dst.sin_addr = tmpaddr;

	ip->ip_dst.s_addr = tmpaddr.s_addr;

	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = MakeCheckSum((uint16_t *)icmp, icmplen);
	if (sendto(sock, buf, ip->ip_len, 0, (struct sockaddr *)&dst, sizeof(dst)) < 0)
	{
		return -1;
	}
	return 0;
}



//
//	Receive Ping Reply
//
static int32_t ReceivePingReply( int32_t sock, const char * /*target*/ )
{
	char buf[1500];
	struct ip *ip = (struct ip *)buf;
	struct icmp *icmp;
	int err = 0;
	long int fromlen = 0;
	int hlen;
	struct timeval tv;
	struct tm *tmtime;
	int recvd = 0;
	// char *hostto;
	char /*hostbuf[128],*/ timebuf[128];
	unsigned long int icmptime, icmpmask;
	struct pollfd	pollEvent;
	int32_t hPoll;

	gettimeofday(&tv, NULL);

	//	Wait Event form UART
	pollEvent.fd		= sock;
	pollEvent.events	= POLLIN | POLLERR;
	pollEvent.revents	= 0;
	hPoll = poll( (struct pollfd*)&pollEvent, 1, 3000 );

	if( hPoll < 0 ) {
		printf("Poll Error!!!\n");
		return -1;
	}
	else if( hPoll == 0 ){
		printf("Timeout!!!\n");
		return 1;
	}

	if ((err = recvfrom(sock, buf, sizeof(buf), 0, NULL, (socklen_t*)&fromlen)) < 0)
	{
		printf("icmpquery:  recvfrom");
		return -1;
	}

	hlen = ip->ip_hl << 2;
	icmp = (struct icmp *)(buf + hlen);

	switch(icmp->icmp_type) {
		case ICMP_ECHOREPLY:
			printf("%-40.40s: icmp echo reply\n", inet_ntoa(ip->ip_src));
			break;
		case ICMP_TSTAMPREPLY:
			icmptime = ntohl(icmp->icmp_ttime);
			 /* ms since midnight. yuch. */
			tv.tv_sec -= tv.tv_sec%(24*60*60);
			tv.tv_sec += (icmptime/1000);
			tv.tv_usec = (icmptime%1000);
			tmtime = localtime(&tv.tv_sec);
			strftime(timebuf, 128, "%H:%M:%S", tmtime);
			printf("%-40.40s:  %s\n", inet_ntoa(ip->ip_src), timebuf);
			break;

		case ICMP_MASKREPLY:
			icmpmask = ntohl(icmp->icmp_dun.id_mask);
			printf("%-40.40s:  0x%lX\n", inet_ntoa(ip->ip_src), icmpmask);
			break;

		default:
			printf("Unknown ICMP message received (type %d)\n",
					icmp->icmp_type);
			break;
	}
	recvd ++;
	return 0;
}


int32_t ping( const char *target )
{
	int32_t sock;
	// char *hostfrom = NULL;
	int32_t on = 1;
	int32_t querytype = ICMP_ECHO;	//	ICMP_MASKREQ
	// int32_t querytype = ICMP_TSTAMP;	//	ICMP_MASKREQ
	// int32_t querytype = ICMP_MASKREQ;	//	ICMP_MASKREQ

	if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
		return -1;
	}

	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		return -1;
	}

	if( 0 != SendPing(sock, querytype, target ) )
	{
		printf("Error : sendpings!!\n");
		return -1;
	}

	if( 0 != ReceivePingReply(sock, target ) )
	{
		printf("Error : recvpings!!\n");
		return -1;
	}

	close( sock );

	return 0;
}
