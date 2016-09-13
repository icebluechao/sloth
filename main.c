#include "sloth.h"

static int32_t sloth_init(void)
{
	sloth_err("init success!\n");
	return 0;
}
static void sloth_exit(void)
{
	sloth_err("exit!\n");
}

module_init(sloth_init);     
module_exit(sloth_exit);     

MODULE_AUTHOR("ligong_liu & zhaozhanxu@163.com");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("no");
