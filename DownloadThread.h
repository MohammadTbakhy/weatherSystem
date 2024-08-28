// DownloadThread.h
#ifndef DOWNLOAD_THREAD_H
#define DOWNLOAD_THREAD_H

#include "CommonObjects.h"
#include <string_view>

class DownloadThread {
public:
    void operator()(CommonObjects& common);
    void SetUrl(std::string_view new_url);
private:
    std::string _download_url;
};

#endif // DOWNLOAD_THREAD_H
