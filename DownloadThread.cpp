#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>

void DownloadThread::operator()(CommonObjects& common)
{
    while (!common.exit_flag) {
        if (!common.url.empty()) {
            httplib::Client cli("http://api.weatherapi.com");

            std::string endpoint = "/v1/current.json?key=a738c520a8ed4d34bda181523242608&q=" + common.url;
            auto res = cli.Get(endpoint.c_str());
            if (res && res->status == 200) {
                std::lock_guard<std::mutex> lock(common.data_mutex);
                common.jsonData = res->body;
            }
            else {
                std::lock_guard<std::mutex> lock(common.data_mutex);
                common.jsonData = "Failed to fetch data.";
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Polling interval
    }
}

void DownloadThread::SetUrl(std::string_view new_url)
{
    _download_url = new_url;
}
