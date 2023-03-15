#ifndef THUMBNAIL_MANAGER_H
#define THUMBNAIL_MANAGER_H
#include <vector>

#include "thumbnail.h"

class ThumbnailManager {
   public:
    std::vector<Thumbnail*> thumbnails;
    std::vector<Thumbnail*> thumbnails_comparing;
    int window_width;
    int window_height;
    int margin;
    int thumbnail_height;
    int max_width;
    std::vector<int> widths;

    ThumbnailManager(int margin, int thumbnail_height);
    bool check_if_new_thumbnails_added();
    void calculate_all_thumbnails_positions();
    void update_all_windows_positions();
    void collect_all_thumbnails();
    void register_all_thumbnails();
    void destroy_all_thumbnails();
    void destroy_all_comparing_thumbnails();
    void update_thumbnails_if_needed();
    static BOOL CALLBACK collector_callback(HWND hwnd, LPARAM lParam);
};

#endif