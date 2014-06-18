#ifndef HEADER_PROTOCOLS
#define HEADER_PROTOCOLS

struct ethernet_header {
  uint8_t dest[6];
  uint8_t source[6];
  uint8_t ether_type[2];
  uint8_t fields_8021Q[2];
  uint8_t ether_type_8021Q[2];
};
typedef struct ethernet_header ETHERNET_HEADER;


struct ip_header {
  uint8_t vers_ihl;
  uint8_t service;
  uint8_t total_lengh[2];
  uint8_t id[2];
  uint8_t flags[2];
  uint8_t fragment_position;
  uint8_t protocol;
  uint8_t checksum[2];
  uint8_t ip_source[4];
  uint8_t ip_dest[4];
};
typedef struct ip_header IP_HEADER;


struct icmp_header {
  uint8_t type;
  uint8_t code;
  uint8_t checksum[2];
  uint8_t id[2];
  uint8_t seq_num[2];
};
typedef struct icmp_header ICMP_HEADER;

#endif
