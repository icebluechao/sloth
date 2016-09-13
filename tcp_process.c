
tcp_flow_t *tcp_passive_open(struct sk_buff *skb, tcp_flow_key_t key, uint64_t hooknum)
{
	if(skb == NULL)
	  return NULL;
	//todo: tcp flows max too large, not create more
	tcp_flow = tcp_flow_alloc(TCP_FLOW_LEG1, key, hook_num, skb);
	if(tcp_flow == NULL) {
		sloth_err("alloc tcp flow failed!\n");
		return NULL;
	}

	sibling = tcp_flow_alloc(TCP_FLOW_LEG2, key, hook_num, skb);
	if(sibling == NULL) {
		sloth_err("alloc sibling flow failed!\n");
		//remember to free tcp flow
		return NULL;
	}

	tcp_flow_bind(tcp_flow, sibling);
	tcp_flow_table_add(tcp_flow);
	tcp_flow_table_add(sibling);

	tcp_flow_change_state(tcp_flow, "passive open", TCP_LISTEN);
	return tcp_flow;
}
