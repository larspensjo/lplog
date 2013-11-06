#pragma once

#include <gtk/gtk.h>

#ifdef DEBUG
#define LPLOG(...) g_debug(__VA_ARGS__)
#else
#define LPLOG(...)
#endif
