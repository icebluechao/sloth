#include "sloth.h"

static uint32_t nf_inet_pre_routing_hook(uint32_t hook_num, struct sk_buff *skb,
			const struct net_device *indev, const struct net_device *outdev,
			int32_t (*okfn)(struct sk_buff *))
{
	uint32_t ret = NF_ACCEPT;
	tcp_flow_key_t key;
	tcp_flow_t *tcp_flow = NULL;
	struct iphdr *iph;
	struct tcphdr *tcph;
	
	HEALTH_CHECK();

	tcp_flow_extra_key(&key, iph, tcph);
	tcp_flow = tcp_flow_table_find(key);
	if(unlikely(tcp_flow = NULL)) {
		if(!tcph->syn || tcph-> fin || tcph->rst || tcph->psh || tcph->ack)
		  goto __exit__;
		if(skb->sp) /* ipsec */
		  goto __exit__;
	}

	return NF_STOLEN;

__exit__:
	return ret;
}

static unsigned int nf_inet_post_routing_hook(unsigned int hooknum, struct sk_buff *skb,
			const struct net_device *in, const struct net_device *out,
			int (*okfn)(struct sk_buff *))
{
	return NF_STOLEN;
}

static bool nf_hooks_registered = false;
static struct nf_hook_ops sloth_nf_hooks[] __read_mostly = {
	{
		.hook		= nf_inet_pre_routing_hook,
		.owner		= THIS_MODULE,
		.pf			= PF_INET,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_RAW+2,
	},
	{
		.hook		= nf_inet_post_routing_hook,
		.owner		= THIS_MODULE,
		.pf			= PF_INET,
		.hooknum	= NF_INET_POST_ROUTING,
		.priority	= NF_IP_PRI_LAST,
	},
};

int32_t hook_init(void)
{
	int32_t ret = nf_register_hooks(sloth_nf_hooks, ARRAY_SIZE(nf_hooks));
	if(unlikely(ret < 0)) {
		sloth_err("Can not register nf_hooks!\n");
		return ret;
	}
	nf_hooks_registered = true;
	return 0;
}

void hook_exit(void)
{
	if(unlikely(nf_hooks_registered == false)) {
		sloth_warn("nf_hooks not registered, so do not unregister!\n");
		return;
	}

	nf_unregister_hooks(sloth_nf_hooks, ARRAY_SIZE(nf_hooks));
	nf_hooks_registered = false;
}
