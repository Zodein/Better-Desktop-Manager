#ifndef WINDOW_SWITCHER_H
#define WINDOW_SWITCHER_H
#include "../thumbnail/thumbnail_manager.h"

class WindowSwitcher {
   public:
    static ThumbnailManager* thumbnail_manager;
    static void create_gui();
};
#endif
