#pragma once

#define TCP_FLOW_HASH_TABLE_SIZE 65536 /* 2**16 */

typedef struct skb_queue {
	struct list_head	list;
	uint32_t			seq;
	uint32_t			nxt_seq;
	uint32_t			pkts_num;
}skb_queue_t;

typedef struct tcp_flow {
	struct hlist_node	list;
	tcp_flow_key_t		key;
	uint32_t		state;
	uint32_t		hook_num;
	spinlock_t		lock;
//	tcp_kref_t		ref_cnt;
	skb_queue_t		recv_queue;

	uint32_t		snd_wnd;
	uint32_t		snd_nxt;
	uint32_t		snd_una;
	uint32_t		snd_cwnd;
	uint32_t		snd_recover;
	uint32_t		rcv_wnd;
	uint32_t		rcv_nxt;
}tcp_flow_t;

typedef struct tcp_flow_bucket {
	struct hlist_head head;
	spinlock_t lock;
}tcp_flow_bucket_t;

typedef struct tcp_flow_table {
	int32_t	hash_size;
	int32_t	hash_mask;
	tcp_flow_bucket_t *bucket[0];
}tcp_flow_table_t;

static inline int32_t tcp_flow_key_hash(tcp_flow_key_t key)
{
	return jhash_3words(key.saddr, key.daddr, key.sport, key.dport);
}

static inline int32_t tcp_flow_key_equal(const tcp_flow_key_t k1,
			const tcp_flow_key_t k2)
{
	return ((k1.saddr == k2.saddr) && (k1.daddr == k2.daddr) &&
				(k1.sport == k2.sport) &&(k1.dport == k2.dport));
}

int32_t tcp_flow_table_init(void);
void tcp_flow_table_add(tcp_flow_t *tcp_flow);
void tcp_flow_table_remove(tcp_flow_t *tcp_flow);
void tcp_flow_table_purge(void);
tcp_flow_t *tcp_flow_table_find(tcp_flow_key_t key);
