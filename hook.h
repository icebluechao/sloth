#pragma once

typedef struct tcp_flow_key {
	uint32_t saddr;
	uint32_t daddr;
	uint16_t sport;
	uint16_t dport;
}tcp_flow_key_t;

#define HEALTH_CHECK() {																\
	if(!skb || !skb->dev || (skb->dev->flags & IFF_LOOPBACK) || skb_linearize(skb)) {	\
		return ret;																		\
	}																					\
	iph = ip_hdr(skb);																	\
	if(!virt_addr_valid(iph) || (iph->protocol != IPPROTO_TCP) ||						\
				(skb->len < ntohs(iph->tot_len)) ||										\
				((ntohs(iph->frag_off) & IP_OFFSET) > 0) ||								\
				!pskb_may_pull(skb, sizeof(struct tcphdr))) {							\
		return ret;																		\
	}																					\
	tcph = (struct tcphdr *)((char *)iph + (iph->ihl << 2));							\
	if(((tcph->doff << 2) < sizeof(struct tcphdr)) ||									\
				!pskb_may_pull(skb, tcph->doff << 2) ||									\
				ipv4_is_zeronet(iph->saddr) || ipv4_is_lbcast(iph->saddr) ||			\
				ipv4_is_multicast(iph->saddr) || ipv4_is_zeronet(iph->daddr) ||			\
				ipv4_is_lbcast(iph->daddr) || ipv4_is_multicast(iph->daddr)) {			\
		return ret;																		\
	}																					\
	skb_set_transport_header(skb, (char *)iph - (char *)skb->data + (iph->ihl << 2));	\
}

static inline void tcp_flow_extra_key(tcp_flow_key_t *key, struct iphdr *iph,
			struct tcphdr *tcph)
{
	key->saddr = iph->saddr;
	key->daddr = iph->daddr;
	key->sport = ntohs(tcph->source);
	key->dport = ntohs(tcph->dest);
}

int32_t hook_init(void);
void hook_exit(void);
