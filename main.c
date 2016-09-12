#include "sloth.h"

MODULE_AUTHOR("zhaozhanxu@163.com");
MODULE_LICENSE("BSD");

static int sloth_init(void)
{
	return 0;
}
static void sloth_exit(void)
{
}

module_init(sloth_init);     
module_exit(sloth_exit);     
