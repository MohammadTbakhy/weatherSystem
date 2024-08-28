#ifndef COMMON_OBJECTS_H
#define COMMON_OBJECTS_H

#include <string>
#include <atomic>
#include <mutex>
#include <vector>

struct CommonObjects {
    std::string url;                       // URL for the download
    std::string jsonData;                  // Holds JSON data fetched
    std::vector<std::pair<std::string, std::string>> favouriteCities; // Vector of pairs to store city name and its JSON data
    std::atomic<bool> exit_flag{ false };  // Flag to signal threads to exit
    std::mutex data_mutex;                 // Mutex to protect shared data
};

#endif // COMMON_OBJECTS_H
