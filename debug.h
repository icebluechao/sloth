#pragma once

#define sloth_printk(level, format, arg...) \
	printk(level "[sloth] %s: " format, __FUNCTION__, ##arg)

#define sloth_err(format, arg...) { \
	sloth_printk(KERN_ERR, format, ##arg); \
}

#define sloth_err(format, arg...) { \
	sloth_printk(KERN_ERR, format, ##arg); \
}

#define sloth_warn(format, arg...) { \
	sloth_printk(KERN_WARNING, format, ##arg); \
}

#define sloth_debug(format, arg...) { \
	sloth_printk(KERN_DEBUG, format, ##arg); \
}

#define sloth_info(format, arg...) { \
	sloth_printk(KERN_INFO, format, ##arg); \
}
