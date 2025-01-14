#ifndef _IP_SS_H_
#define _IP_SS_H_

/*----------------------------------------------*/
/* Maxumum number of concurrent calls served	*/
/*----------------------------------------------*/
#define	IPSS_NCONN	2048
/*----------------------------------------------*/
/* Inv. MAC adress of interface we so need :)	*/
/*----------------------------------------------*/

#define IPSS_MAC	{(unsigned char)~0x40,(unsigned char)~0x61,(unsigned char)~0x86,(unsigned char)~0x2b,(unsigned char)~0x85,(unsigned char)~0xd6}
#include <netinet/in.h>

#ifdef linux
#include <endian.h>
#else
#include <sys/endian.h>
#endif


#ifndef	_BYTE_ORDER
#define	_BIG_ENDIAN	BIG_ENDIAN
#define	_BYTE_ORDER	BYTE_ORDER
#endif

/*----------------------------------------------*/
/* START rtp.h -- RTP v2 header file (RFC 1889)	*/
/*						*/
/* Modified for LITTLE_ENDIAN(i386) architecture*/
/* and for GCC bit fields order in struct	*/
/* NOTE: not tested on BIG_ENDIAN architectures	*/
/*----------------------------------------------*/
#include <sys/types.h>

/*
 * The type definitions below are valid for 32-bit architectures and
 * may have to be adjusted for 16- or 64-bit architectures.
 */
typedef	unsigned char	u_int8;
typedef	unsigned short	u_int16;
typedef	unsigned int	u_int32;
typedef unsigned int    uint;
typedef	short		int16;

/*
 * Current protocol version.
 */
#define	RTP_VERSION	2

#define	RTP_SEQ_MOD (1<<16)
#define	RTP_MAX_SDES 255	/* maximum text length for SDES */


#include <net/if_dl.h>


typedef enum
{
	RTCP_SR		= 200,
	RTCP_RR		= 201,
	RTCP_SDES	= 202,
	RTCP_BYE	= 203,
	RTCP_APP	= 204
}	rtcp_type_t;

typedef	enum
{
	RTCP_SDES_END	= 0,
	RTCP_SDES_CNAME	= 1,
	RTCP_SDES_NAME	= 2,
	RTCP_SDES_EMAIL	= 3,
	RTCP_SDES_PHONE	= 4,
	RTCP_SDES_LOC	= 5,
	RTCP_SDES_TOOL	= 6,
	RTCP_SDES_NOTE	= 7,
	RTCP_SDES_PRIV	= 8
}	rtcp_sdes_type_t;

/*
 * RTP data header
 */
typedef struct
{
#if _BYTE_ORDER == _BIG_ENDIAN
	unsigned int	version	:2;	/* protocol version */
	unsigned int	p	:1;	/* padding flag */
	unsigned int	x	:1;	/* header extension flag */
	unsigned int	cc	:4;	/* CSRC count */

	unsigned int	m	:1;	/* marker bit */
	unsigned int	pt	:7;	/* payload type */
#else
	unsigned int	cc	:4;	/* CSRC count */
	unsigned int	x	:1;	/* header extension flag */
	unsigned int	p	:1;	/* padding flag */
	unsigned int	version	:2;	/* protocol version */

	unsigned int	pt	:7;	/* payload type */
	unsigned int	m	:1;	/* marker bit */
#endif
	u_int16		seq;		/* sequence number */
	u_int32		ts; 		/* timestamp */
	u_int32		ssrc;		/* synchronization source */
	u_int32		csrc[1];	/* optional CSRC list */
}	rtp_hdr_t;

/*
 * RTCP common header word
 */
typedef struct
{
#if _BYTE_ORDER == _BIG_ENDIAN
	unsigned int	version	:2;	/* protocol version */
	unsigned int	p	:1;	/* padding flag */
	unsigned int	count	:5;	/* varies by packet type */
	unsigned int	pt	:8;	/* RTCP packet type */
#else
	unsigned int	count	:5;	/* varies by packet type */
	unsigned int	p	:1;	/* padding flag */
	unsigned int	version	:2;	/* protocol version */
	unsigned int	pt	:8;	/* RTCP packet type */
#endif
	u_int16		length;		/* pkt len in words, w/o this word */
}	rtcp_common_t;

/*
 * Big/Little-endian mask for version, padding bit and packet type pair
 */

#if _BYTE_ORDER == _BIG_ENDIAN
#define RTCP_VALID_MASK		(0xc000 | 0x2000 | 0xfe)
#define RTCP_VALID_VALUE	((RTP_VERSION << 14) | RTCP_SR)
#else
#define RTCP_VALID_MASK		(0xc0 | 0x20 | 0xfe00)
#define RTCP_VALID_VALUE	((RTP_VERSION << 6) | (RTCP_SR<<8))
#endif

/*
 * Reception report block
 */
typedef struct
{
	u_int32		ssrc;		/* data source being reported */
	unsigned int	fraction:8;	/* fraction lost since last SR/RR */
	int		lost	:24;	/* cumul. no. pkts lost (signed!) */
	u_int32		last_seq;	/* extended last seq. no. received */
	u_int32		jitter;		/* interarrival jitter */
	u_int32		lsr;		/* last SR packet from this source */
	u_int32		dlsr;		/* delay since last SR packet */
}	rtcp_rr_t;

/*
 * SDES item
 */
typedef struct
{
	u_int8		type;		/* type of item (rtcp_sdes_type_t) */
	u_int8		length;		/* length of item (in octets) */
	char		data[1];	/* text, not null-terminated */
}	rtcp_sdes_item_t;

/*
 * One RTCP packet
 */
typedef	struct
{
	rtcp_common_t	common;		/* common header */
	union
	{
		struct rtcp_sr
		{				/* sender report (SR) */
			u_int32	ssrc;		/* sender generating this report */
			u_int32	ntp_sec;	/* NTP timestamp */
			u_int32	ntp_frac;
			u_int32	rtp_ts;		/* RTP timestamp */
			u_int32	psent;		/* packets sent */
			u_int32	osent;		/* octets sent */
			rtcp_rr_t rr[1];	/* variable-length list */
		} sr;

		struct rtcp_rr
		{				/* reception report (RR) */
			u_int32 ssrc;		/* receiver generating this report */
			rtcp_rr_t rr[1];	/* variable-length list */
		} rr;

		struct rtcp_sdes
		{					/* source description (SDES) */
			u_int32 src;			/* first SSRC/CSRC */
			rtcp_sdes_item_t item[1];	/* list of SDES items */
		} sdes;

		struct rtcp_bye
		{				/* BYE */
			u_int32 src[1];	 /* list of sources */
			/* can't express trailing text for reason */
		} bye;
	} r;
}	rtcp_t;

typedef	struct rtcp_sdes rtcp_sdes_t;

/*
 * Per-source state information
 */
typedef	struct
{
	u_int32	ssrc;
	u_int32	cycles;			/* shifted count of seq. number cycles */
	u_int32	base_seq;		/* base seq number */
	u_int32	bad_seq;		/* last 'bad' seq number + 1 */
	u_int32	probation;		/* sequ. packets till source is valid */
	u_int32	received;		/* packets received */
	u_int32	expected_prior;		/* packet expected at last interval */
	u_int32	received_prior;		/* packet received at last interval */
	u_int32	transit;		/* relative trans time for prev pkt */
	u_int32	jitter;			/* estimated jitter */
	u_int16	max_seq;		/* highest seq. number seen */
	/* ... */
}	source;

/*----------------------------------------------*/
/* STOP  rtp.h -- RTP v2 header file (RFC 1889) */
/*----------------------------------------------*/


/*
 * Links are numbered as follows:
 *      +-----+ 00(L0) +----+ 10(L2) +-----+
 *Flow0 |     | <----> |    | <----> |     |
 *      | CL0 | 01(L1) | SS | 11(L3) | CL1 |
 *Flow1 |     | <----> |    | <----> |     |
 *      +-----+        +----+        +-----+
 * Traf should be redirected:
 *	Flow 0:
 * CL0( L0(RI,RP) ) -> SS( L0(LI,LP) )  ===>  SS( L2(LI,LP) ) -> CL1( L2(RI,RP) )
 * CL1( L2(RI,RP) ) -> SS( L2(LI,LP) )  ===>  SS( L0(LI,LP) ) -> CL0( L0(RI,RP) )
 *	Flow 1:
 * CL0( L1(RI,RP) ) -> SS( L1(LI,LP) )  ===>  SS( L3(LI,LP) ) -> CL0( L3(RI,RP) )
 * CL1( L3(RI,RP) ) -> SS( L3(LI,LP) )  ===>  SS( L1(LI,LP) ) -> CL0( L1(RI,RP) )
 */

typedef	source		ss_ssrc;
#define	IPSS_SSRC_S	sizeof(ss_ssrc)

/*
 * Codecs we can resize packets for
 */
typedef enum 
{
	CODEC_UNKNOWN = 0,
	CODEC_G711_7,
	CODEC_G711_8,
	CODEC_G729,
	CODEC_G723
} codec_t;

/*
 * Codecs frame sizes
 */
static const uint8_t G729_frame_size = 10;
static const uint8_t G723_frame_sizes[] = {24, 20, 4, 1};
#define G723_frame_size(x) G723_frame_sizes[(*x)&3]

/*
 * UDP link adress descriptor
 */
struct	ipss_link
{
	struct in_addr	rip;	/* link remote IP */
	struct in_addr	lip;	/* link local  IP */
	uint16_t	rport;	/* link remote port */
	uint16_t	lport;	/* link local  port */
	uint8_t		codec;	/* codec type for packet resizing */
	uint8_t		fcount;	/* frame count in output packet */
};
typedef	struct ipss_link ss_link;
#define	IPSS_LINK_S sizeof(ss_link)

/*
 * UDP link destination descriptor
 */
struct	ipss_dest
{
	struct in_addr ip;	/* destination IP */
	u_int16_t port;		/* destination port */
};
typedef	struct ipss_dest ss_dest;
#define	IPSS_DEST_S sizeof(ss_dest)

/*
 * Link statistics
 */
struct	ipss_stat
{
	time_t	tstamp;		/* last active time */
	uint pcnt;		/* packet count */
	uint bcnt;		/* byte count */
	uint count;
	uint fracLostSum;
	uint packLost;
	uint jitterMax;
	uint jitterSum;
	uint firstSequence;
	uint lastSequence;
};
typedef	struct ipss_stat ss_stat;
#define	IPSS_STAT_S	sizeof(ss_stat)

/*
 * Connection structure
 */
struct	ipss_conn
{
	struct ipss_link L[6];
	struct ipss_stat S[4];
	uint Flags;
};

typedef	struct ipss_conn ss_conn;
typedef struct
{

	struct ip * sip;
	uint16_t iplen;
	uint16_t iphlen;
	struct udphdr *uh;
	rtp_hdr_t *rtp;
	int rtp_cc;
	caddr_t data;
	unsigned protoheader_len;
	int data_len;
} packetHdr;


#define	IPSS_CONN_S	sizeof(ss_conn)
#define	IPSS_CF_ACT	0x0001		/* connection Active	flag	*/
#define	IPSS_CF_CS	0x0002		/* connection CheckSum	flag	*/
#define	IPSS_CF_STAT	0x0004		/* connection Stats	flag	*/
#define	IPSS_CF_CHADDR	0x0008		/* connection change orig addr flag */
#define	IPSS_CF_CKSUM	0x0010		/* connection update udp cksum flag */

#define	SSCL(c,l)	((c)->L[l])		/* connection link addr struct	*/
#define	SSCS(c,l)	((c)->S[l])		/* connection link stat	struct	*/
#define	SSCF(c)		((c)->Flags)		/* connection link flags	*/

#define	CONN_IS_ACT(c)	(((c)->Flags)&IPSS_CF_ACT)	/* connection is active check */
#define	CONN_IS_CHA(c)	(((c)->Flags)&IPSS_CF_CHADDR)	/* connection change orig addr check */
#define	CONN_IS_CKS(c)	(((c)->Flags)&IPSS_CF_CKSUM)	/* connection update checksum check */

#define	SSCL_RIA(c,l)	SSCL(c,l).rip		/* link remote IP in_addr	*/
#define	SSCL_LIA(c,l)	SSCL(c,l).lip		/* link local  IP in_addr	*/
#define	SSCL_RI(c,l)	SSCL(c,l).rip.s_addr	/* link remote IP		*/
#define	SSCL_LI(c,l)	SSCL(c,l).lip.s_addr	/* link local  IP		*/
#define	SSCL_RP(c,l)	SSCL(c,l).rport		/* link remote port		*/
#define	SSCL_LP(c,l)	SSCL(c,l).lport		/* link local  port		*/
#define SSCL_CDC(c,l)	SSCL(c,l).codec		/* link codec			*/
#define SSCL_FC(c,l)	SSCL(c,l).fcount	/* link frames count		*/

#define	SSCS_T(c,l)	SSCS(c,l).tstamp	/* link stat timestamp		*/
#define	SSCS_P(c,l)	SSCS(c,l).pcnt		/* link stat packet count	*/
#define	SSCS_B(c,l)	SSCS(c,l).bcnt		/* link stat byte count		*/

#define	SSCS_c(c,l)	SSCS(c,l).count
#define	SSCS_fLS(c,l)	SSCS(c,l).fracLostSum
#define	SSCS_pL(c,l)	SSCS(c,l).packLost
#define	SSCS_jM(c,l)	SSCS(c,l).jitterMax
#define	SSCS_jS(c,l)	SSCS(c,l).jitterSum
#define	SSCS_fS(c,l)	SSCS(c,l).firstSequence
#define	SSCS_lS(c,l)	SSCS(c,l).lastSequence

#define RI0(c)		SSCL_RI(c,0)
#define RI1(c)		SSCL_RI(c,1)
#define RI2(c)		SSCL_RI(c,2)
#define RI3(c)		SSCL_RI(c,3)
#define RI4(c)		SSCL_RI(c,4)
#define RI5(c)		SSCL_RI(c,5)
#define LI0(c)		SSCL_LI(c,0)
#define LI1(c)		SSCL_LI(c,1)
#define LI2(c)		SSCL_LI(c,2)
#define LI3(c)		SSCL_LI(c,3)
#define LI4(c)		SSCL_LI(c,4)
#define LI5(c)		SSCL_LI(c,5)
#define RP0(c)		SSCL_RP(c,0)
#define RP1(c)		SSCL_RP(c,1)
#define RP2(c)		SSCL_RP(c,2)
#define RP3(c)		SSCL_RP(c,3)
#define RP4(c)		SSCL_RP(c,4)
#define RP5(c)		SSCL_RP(c,5)
#define LP0(c)		SSCL_LP(c,0)
#define LP1(c)		SSCL_LP(c,1)
#define LP2(c)		SSCL_LP(c,2)
#define LP3(c)		SSCL_LP(c,3)
#define LP4(c)		SSCL_LP(c,4)
#define LP5(c)		SSCL_LP(c,5)

#define T0(c)		SSCS_T(c,0)
#define T1(c)		SSCS_T(c,1)
#define T2(c)		SSCS_T(c,2)
#define T3(c)		SSCS_T(c,3)
#define P0(c)		SSCS_P(c,0)
#define P1(c)		SSCS_P(c,1)
#define P2(c)		SSCS_P(c,2)
#define P3(c)		SSCS_P(c,3)
#define B0(c)		SSCS_B(c,0)
#define B1(c)		SSCS_B(c,1)
#define B2(c)		SSCS_B(c,2)
#define B3(c)		SSCS_B(c,3)

/* Init command structure  */
struct	ipss_icmd
{			
	uint16_t port_base;	/* port base */
	uint16_t port_end;	/* port end */
};
typedef struct ipss_icmd ss_icmd;

/* Get command structure */
struct	ipss_gcmd
{			
	uint cmd;					/* command code */
	uint c;						/* connection # */
	union gcmddata { ss_conn conn; } gcmddata;	/* for IPSS_CSET_ADDR */
};
typedef	struct ipss_gcmd	ss_gcmd;

#define	IPSS_GCMD_S	sizeof(ss_gcmd)
#define	IPSS_CMDH_S	(sizeof(ss_gcmd)-sizeof(union gcmddata))
#define	CGET_conn	gcmddata.conn
		
/* Set log levels command structure */
struct ipss_lcmd
{
    int start_level;
    int last_level;
};
typedef struct ipss_lcmd	ss_lcmd;
#define	IPSS_LCMD_S	sizeof(ss_lcmd)

/*
 * Set command structure
 */
struct	ipss_scmd
{
	uint cmd;				/* command code	*/
	uint c;					/* connection #	*/
	union scmddata
	{
		struct ciniconn
		{
			uint f;			/* connection flags */
			u_int16_t cinilp[6];	/* IPSS_CINI_CONN local ports */
		} ciniconn;

		struct csetlink
		{		
			uint f;			/* additions for connection flags (OR) */
			uint l;			/* connection link # */
			ss_dest	dest;		/* link dest address */
			codec_t	codec;		/* codec type for packet resizing */
			uint8_t fcount;		/* frame count in output packet */
		} csetlink;
		
		struct csetplt
		{		
			unsigned payloadType; /* payload type of telephone event codec*/
		} csetplt;
		
	} scmddata;
};
typedef	struct ipss_scmd	ss_scmd;
#define	IPSS_SCMD_S	sizeof(ss_scmd)

#define	CINI_f		scmddata.ciniconn.f
#define	CINI_lp		scmddata.ciniconn.cinilp
#define	CSET_f		scmddata.csetlink.f
#define	CSET_l		scmddata.csetlink.l
#define	CSET_dest	scmddata.csetlink.dest
#define	CSET_codec	scmddata.csetlink.codec
#define CSET_payloadType scmddata.csetplt.payloadType
#define	CSET_fcount	scmddata.csetlink.fcount
#define	CSET_ipa	CSET_dest.ip
#define	CSET_ip		CSET_dest.ip.s_addr
#define	CSET_port	CSET_dest.port

/* IPSS control codes */
#ifndef IP_SS_INI
#define	IP_SS_INI 90   /* init IPSS module */
#define	IP_SS_GET 91   /* smth get */
#define	IP_SS_SET 92   /* smth set */
#define	IP_SS_DEL 93   /* release IPSS module */
#define IP_SS_LOG 94   /* Set log levels                       */
#endif

/* ipss_cmd Command Codes */
#define	IPSS_CINI_CONN	0	/* Init connection (conn #)		*/
#define	IPSS_CACT_CONN	1	/* Activate connection (conn #)		*/
#define	IPSS_CSTP_CONN	2	/* Stop connection (conn #)		*/
#define	IPSS_CSET_RLINK	3	/* Set connection link rem.addrs	*/
				/* (conn #,link #,ss_dest)		*/
#define	IPSS_CSET_LLINK	4	/* Set connection link loc.addrs	*/
				/* (conn #,link #,ss_dest)		*/
#define	IPSS_CGET_CONN	5	/* Get connection info (conn #)		*/
#define IPSS_CSET_LINKC	6	/* Set link buffer codec and frames cnt	*/

#define IPSS_CSET_PLT	7	/* Set payload type of telephone event	*/


#define	CMD_CGET(_s,_len,_cmd,_conn) \
{	(*_cmd).c	=_conn; \
	(*_cmd).cmd	=IPSS_CGET_CONN; \
	if ( getsockopt(_s, IPPROTO_IP, IP_SS_GET, _cmd, _len) < 0 ) \
		err(EX_UNAVAILABLE, "getsockopt(CGET)"); \
}
#define	CMD_CINI(_s,_cmd,_conn,_lp0,_lp1,_lp2,_lp3,_lp4,_lp5) \
{	(*_cmd).CINI_lp[0]=_lp0; (*_cmd).CINI_lp[1]=_lp1; \
	(*_cmd).CINI_lp[2]=_lp2; (*_cmd).CINI_lp[3]=_lp3; \
        (*_cmd).CINI_lp[4]=_lp4; (*_cmd).CINI_lp[5]=_lp5; \
	(*_cmd).c	=_conn; \
	(*_cmd).cmd	=IPSS_CINI_CONN; \
	if ( setsockopt(_s, IPPROTO_IP, IP_SS_SET, _cmd, IPSS_SCMD_S) < 0 ) \
		err(EX_UNAVAILABLE, "setsockopt(CINI)"); \
}
#define	CMD_CACT(_s,_cmd,_conn) \
{	(*_cmd).c	=_conn; \
	(*_cmd).cmd	=IPSS_CACT_CONN; \
	if ( setsockopt(_s, IPPROTO_IP, IP_SS_SET, _cmd, IPSS_SCMD_S) < 0 ) \
		err(EX_UNAVAILABLE, "setsockopt(CACT)"); \
}
#define	CMD_CSTP(_s,_cmd,_conn) \
{	(*_cmd).c	=_conn; \
	(*_cmd).cmd	=IPSS_CSTP_CONN; \
	if ( setsockopt(_s, IPPROTO_IP, IP_SS_SET, _cmd, IPSS_SCMD_S) < 0 ) \
		err(EX_UNAVAILABLE, "setsockopt(CSTP)"); \
}
#define	CMD_CSETR(_s,_cmd,_conn,_rip,_rport) \
{	(*_cmd).CSET_ip.s_addr=_rip;	(*_cmd).CSET_port=_rport; \
	(*_cmd).c	=_conn; \
	(*_cmd).cmd	=IPSS_CSET_RLINK; \
	if ( setsockopt(_s, IPPROTO_IP, IP_SS_SET, _cmd, IPSS_SCMD_S) < 0 ) \
		err(EX_UNAVAILABLE, "setsockopt(CSETR)"); \
}
#define	CMD_CSETL(_s,_cmd,_conn,_lip,_lport) \
{	(*_cmd).CSET_ip.s_addr=_lip;	(*_cmd).CSET_port=_lport; \
	(*_cmd).c	=_conn; \
	(*_cmd).cmd	=IPSS_CSET_LLINK; \
	if ( setsockopt(_s, IPPROTO_IP, IP_SS_SET, _cmd, IPSS_SCMD_S) < 0 ) \
		err(EX_UNAVAILABLE, "setsockopt(CSETL)"); \
}
#define	CMD_CSET(_s,_cmd,_conn,_rip,_rport)	CMD_CSETR(_s,_cmd,_conn,_rip,_rport)

/* Swap bytes in short & inverse - for checksum */
/* #define CS(_s) (~((((_s)&0x00FF)<<8)|(((_s)&0xFF00)>>8)) & 0xFFFF) */

/* Swap bytes in short - like htons/ntohs */
#ifdef	__htons
#define	H2N(_s)	__htons(_s)
#else
#define	H2N(_s)	(((_s&0x00FF)<<8)+((_s&0xFF00)>>8))
#endif
#define	N2H	H2N

#define	H3N(_s)	(((_s&0x0000FF)<<16)+(_s&0x00FF00)+((_s&0xFF0000)>>16))
#define	N3H	H3N

#ifdef	__htonl
#define	H4N(_s)	__htonl(_s)
#else
#define	H4N(_s)	(((_s&0x000000FF)<<24)+((_s&0x0000FF00)<<8)+((_s&0x00FF0000)>>8)+((_s&0xFF000000)>>24))
#endif
#define	N4H	H4N

/* Convert ASCIIZ IP to network format (inet_aton) */
#define	I2N(ip,p)	(inet_aton(ip,p)? 0 : err(EX_DATAERR, "invalid IP: %s",ip))

#define	XCMD_CSET(_s,_cmd,_conn,_link,_rip,_rport) \
{	I2N(_rip,&(*_cmd).CSET_ipa);	(*_cmd).CSET_port=H2N(_rport); \
	(*_cmd).CSET_l	=_link; \
	(*_cmd).c	=_conn; \
	(*_cmd).cmd	=IPSS_CSET_LINK; \
	if ( setsockopt(s, IPPROTO_IP, IP_SS_SET, _cmd, IPSS_SCMD_S) < 0 ) \
		err(EX_UNAVAILABLE, "setsockopt(CSET)"); \
}

#ifdef _KERNEL

/* Packet processing result */
#define IPSS_PACKET_IGNORED	0
#define IPSS_PACKET_PROCESSED	1
#define IPSS_PACKET_BUFFERED	2

/*
 * Packet buffer.
 * Holds pointers on two codec-dependent functions. Both pointers should
 * be initialized before using buffer for stroing packets.
 *
 * frames_count(struct mbuf*)
 * simply returns a frames count in the given packet
 *
 * frames_length(struct mbuf*, uint16_t)
 * returns a length of desired frames count from given packet
 *
 */
struct ipss_buffer
{
	caddr_t buf;		/* internal buffer */
	int16_t len;		/* internal data length */
	int16_t curr;		/* current frames count */
	int16_t threshold;	/* threshold value */
	int16_t seq; 		/* sequence number */
	u_int32 ts;		/* timestamp */
	uint16_t dur;		/* frame duration*/ 
	
	uint16_t (*frames_count)(caddr_t, uint16_t);
	uint16_t (*frames_length)(caddr_t, uint16_t);
	
};
typedef struct ipss_buffer ss_buffer;

/*
 * Incoming packet handling structure. 
 */
#define	CMAP_MAGIC	0x43043B
struct ipss_cmap
{
	unsigned	magic;
	struct ip	*ip;
	struct udphdr	*uh;
	ss_conn		*cn;
	u_int16_t	is; /* source index for RTP_Get */
	ss_buffer	*buf;
	
	unsigned payloadType_;
	
	int (*cf)(struct ipss_cmap*);
};
typedef struct ipss_cmap ss_cmap;
#define	IPSS_CMAP_S	sizeof(ss_cmap)

#define RTF_DELCLONE	0x80            /*  unused, was RTF_DELCLONE */
#define RTF_CLONING	0x100           /*  unused, was RTF_CLONING */

/*
 * Function definitions.
 */
typedef	int ip_ss_chk_t	(register struct mbuf *);
typedef	int ip_ss_ctl_t	(struct sockopt *);
extern	ip_ss_chk_t	*ip_ss_chk_ptr;
extern	ip_ss_ctl_t	*ip_ss_ctl_ptr;

/*
 * Macros for checking IPSS loading
 */
#define	IPSS_LOADED	(ip_ss_ctl_ptr!=NULL)

#define PULLUP(mbuf, len) \
if (mbuf->m_len < len)  \
    mbuf = m_pullup(mbuf,len); 

										
										


/*
 * Kernel malloc type declaration
 */
//MALLOC_DECLARE(M_IPSS);

#endif /* _KERNEL */

#endif /* _IPRTP_H_ */
