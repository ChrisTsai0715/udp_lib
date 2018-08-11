#ifndef _LOG_PRINT_
#define _LOG_PRINT_

#include <stdlib.h>
#include <stdarg.h>

enum
{
	LOG_PRINT_ERROR = 0,
	LOG_PRINT_WARNING,
	LOG_PRINT_DEBUG,
	LOG_PRINT_INFO,
};

static void log_print(char level, const char *fmt, ...)
{
    //raw print  --cairui
#ifdef PLUG_DEBUG
	return;
#else
    switch(level)
    {
	case LOG_PRINT_ERROR:
        printf("**ERROR**    ");
        break;
	case LOG_PRINT_WARNING:
        printf("**WARING**   ");
        break;
    default:
	case LOG_PRINT_INFO:
        printf("- Info -     ");
        break; 
	case LOG_PRINT_DEBUG:
        printf("- Debug -    ");
        break; 
    }

	char buf[128] = {0};
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
    buf[sizeof(buf)] = '\0';
    printf(buf);
    printf("\n");
    va_end(args);
#endif
}

#endif
