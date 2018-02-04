
#define VRRP_ADVT_FRAME_SZ 264
struct vrrp_advt_info
{
  /* Version & Type. */
  uint8_t ver_type;

  /* VrId. */
  uint8_t vrid;

  /* Priority. */
  uint8_t priority;

  /* Number of IP addresses to backup. */
  uint8_t num_ip_addrs;
  union
  {
    struct {
        /* Auth type. */
        u_int8_t auth_type;
        /* Advertisement interval. */
        u_int8_t advt_int; /* rsvd + Adver Int */
    };
    /* Advertisement interval. */
    uint16_t max_advt_int;
  };

  /* VRRP Checksum. */
  u_int16_t cksum;
  union
  {
	  struct in_addr vip_v4[0];
	  struct in6_addr vip_v6[0];
  };
};

typedef struct vrrp_advt_buf {
    int                  sock;
    union {
        struct in_addr  addr;
        struct sockaddr_in sin;
        struct in6_addr  addr_ipv6;
        struct sockaddr_in6 sin_ipv6;
    } u;
    union {
        struct ip iph;
        struct ip6_hdr ip6h;
    };
    uint32_t             ifindex;
    int                  size;
    uint8_t              is_vrrpe;
    uint8_t              af_type ;
    char                 data[VRRP_ADVT_FRAME_SZ];
    int                  vrf_id;
} vrrp_advt_buf_t;

