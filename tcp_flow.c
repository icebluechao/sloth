#include "sloth.h"

static struct kmem_cache *tcp_flow_cache = NULL;

int32_t tcp_flow_cache_init(void)
{
	tcp_flow_cache = kmem_cache_create("tcp_flow_cache",
				sizeof(tcp_flow_t), 0, SLAB_HWCACHE_ALIGN, NULL);
	if(tcp_flow_cache == NULL) {
		sloth_err("kmem_cache_create tcp_flow_cache failed!\n");
		return -ENOMEM;
	} else {
		sloth_debug("kmem_cache_create tcp_flow_cache %p\n", tcp_flow_cache);
		return 0;
	}
}

void tcp_flow_cache_clean(void)
{
	if(tcp_flow_cache = NULL)
	  return;
	sloth_debug("kmem_cache_destroy tcp_flow_cache %p\n", tcp_flow_cache);
	kmem_cache_destroy(tcp_flow_cache);
	tcp_flow_cache = NULL;
}

tcp_flow_t *tcp_flow_alloc(tcp_flow_type_t type, tcp_flow_key_t key,
			uint32_t hook_num, struct sk_buff *skb)
{
	tcp_flow = (tcp_flow_t *)kmem_cache_alloc(tcp_flow_cache, GFP_ATOMIC);
	if(tcp_flow == NULL) {
		sloth_err("alloc tcp flow failed!\n");
		return NULL;
	}
	memset(tcp_flow, 0, sizeof(tcp_flow_t));
	kref_init(&tcp_flow->ref_cnt);

	tcp_flow->type = type;
	tcp_flow->state = TCP_CLOSED;
}

tcp_flow_table_t *tcp_flow_table = NULL;

int32_t tcp_flow_table_init(void)
{
	int32_t i, alloc_size = 0;
	int32_t tcp_flow_hash_table_size = TCP_FLOW_HASH_TABLE_SIZE;

	while(tcp_flow_table == NULL) {
		alloc_size = sizeof(tcp_flow_table_t) +
			sizeof(tcp_flow_bucket_t *) * tcp_flow_hash_table_size;
		tcp_flow_table = kmalloc(alloc_size, GFP_ATOMIC);
		if(tcp_flow_table == NULL) {
			if(tcp_flow_hash_table_size < 1024) {
				sloth_err("alloc tcp_flow_table failes, hash_size %d, alloc_size %d\n",
							tcp_flow_hash_table_size, alloc_size);
				return -ENOMEM;
			} else {
				sloth_err("alloc tcp_flow_table failes, hash_size %d, alloc_size %d\n",
							tcp_flow_hash_table_size, alloc_size);
				tcp_flow_hash_table_size = tcp_flow_hash_table_size / 2;
			}
		}
	}
	memset(tcp_flow_table, 0, alloc_size);
	tcp_flow_table->hash_size = tcp_flow_hash_table_size;
	tcp_flow_table->hash_mask = tcp_flow_hash_table_size - 1;

	for(i = 0; i < tcp_flow_hash_table_size; i++) {
		tcp_flow_table->bucket[i] = kmalloc(sizeof(tcp_flow_bucket_t), GFP_ATOMIC);
		if(tcp_flow_table->bucket[i] == NULL) {
			sloth_err("alloc tcp_flow_bucket failes, index %d\n", i);
			while(--i >= 0) {
				kfree(tcp_flow_table->bucket[i]);
			}
			kfree(tcp_flow_table);
			return -ENOMEM;
		}
		INIT_HLIST_HEAD(&tcp_flow_table->bucket[i]->head);
		spin_lock_init(&tcp_flow_table->bucket[i]->lock);
	}
	return 0;
}

static inline tcp_flow_bucket_t *tcp_flow_hash_bucket(tcp_flow_key_t key)
{
	int i = tcp_flow_key_hash(key) & tcp_flow_table->hash_mask;
	return tcp_flow_table->bucket[i];
}

void tcp_flow_table_add(tcp_flow_t *tcp_flow)
{
	tcp_flow_bucket_t *bucket;

	if(!tcp_flow_table || !tcp_flow) {
		sloth_err("tcp_flow_table %p and tcp_flow %p must not be NULL!\n",
					tcp_flow_table, tcp_flow);
		return;
	}

	bucket = tcp_flow_hash_bucket(tcp_flow->key);
	sloth_debug("add tcp_flow %p to tcp_flow_table %p\n", tcp_flow, tcp_flow_table);
	spin_lock_bh(&bucket->lock);
	hlist_add_head(&tcp_flow->list, &bucket->head);
	spin_unlock_bh(&bucket->lock);
	//todo: start flow timer
}

static inline void _tcp_flow_table_remove(tcp_flow_t *tcp_flow)
{
	if(!tcp_flow || hlist_unhashed(&tcp_flow->list)) {
		sloth_err("tcp_flow %p null or unlinked tcp_flow!\n", tcp_flow);
		return;
	}
	hlist_del_init(&tcp_flow->list);
}

void tcp_flow_table_remove(tcp_flow_t *tcp_flow)
{
	tcp_flow_bucket_t *bucket;

	if(!tcp_flow_table || !tcp_flow) {
		sloth_err("tcp_flow_table %p and tcp_flow %p must not be NULL!\n",
					tcp_flow_table, tcp_flow);
		return;
	}

	bucket = tcp_flow_hash_bucket(tcp_flow->key);
	spin_lock_bh(&bucket->lock);
	_tcp_flow_table_remove(tcp_flow);
	spin_unlock_bh(&bucket->lock);
}

void tcp_flow_table_purge(void)
{
	uint32_t i;
	struct hlist_node *node;
	tcp_flow_t *tcp_flow;
	tcp_flow_bucket_t *bucket;

	if(tcp_flow_table == NULL) {
		sloth_err("tcp_flow_table %p must not be NULL!\n", tcp_flow_table);
		return;
	}
	for(i = 0; i < tcp_flow_table->hash_size; i++) {
		bucket = tcp_flow_table->bucket[i];
		spin_lock_bh(&bucket->lock);
		hlist_for_each_entry_safe(tcp_flow, node, &bucket->head, list) {
			sloth_debug("remove tcp_flow %p from tcp_flow_table %p\n",
						tcp_flow, tcp_flow_table);
			_tcp_flow_table_remove(tcp_flow);
		}
		spin_unlock_bh(&bucket->lock);
		kfree(bucket);
	}
	kfree(tcp_flow_table);
	tcp_flow_table = NULL;
}

tcp_flow_t *tcp_flow_table_find(tcp_flow_key_t key)
{
	tcp_flow_bucket_t *bucket;
	struct hlist_node *node;
	tcp_flow_t *tcp_flow;
	
	bucket = tcp_flow_hash_bucket(key);
	spin_lock_bh(&bucket->lock);
	hlist_for_each_entry_safe(tcp_flow, node, &bucket->head, list) {
		if(tcp_flow_key_equal(tcp_flow->key, key) &&
					between(tcp_flow->state, TCP_LISTEN, TCP_LAST_ACK)) {
			spin_unlock_bh(&bucket->lock);
			return tcp_flow;
		}
	}
	spin_unlock_bh(&bucket->lock);
	return NULL;
}
