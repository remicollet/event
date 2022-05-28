// make php build system happy

#if !defined(PHP_VERSION_ID)
# error PHP_VERSION_ID is not defined, this file is stub only for php build system, use phpx/php_event.h instead
#elif PHP_VERSION_ID >= 80000
# include "php8/php_event.h"
#elif PHP_VERSION_ID >= 70000
# include "php7/php_event.h"
#elif PHP_VERSION_ID >= 50000
# include "php5/php_event.h"
#else
# error unsupported PHP version
#endif

extern zend_module_entry event_module_entry;
#ifndef phpext_event_ptr
# define phpext_event_ptr &event_module_entry
#endif
