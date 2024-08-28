// ConnectedApp.cpp

#include <iostream>
#include <thread>
#include "CommonObjects.h"
#include "DrawThread.h"
#include "DownloadThread.h"

int main()
{
    CommonObjects common;
    DrawThread draw;
    auto draw_th = std::jthread([&] { draw(common); });

    DownloadThread down;
    auto down_th = std::jthread([&] { down(common); });

    std::cout << "running...\n";

    down_th.join();
    draw_th.join();

    return 0;
}
