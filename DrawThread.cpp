#include "DrawThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"
#include "nlohmann/json.hpp"
#include <fstream>  // To handle file input and output operations
#include <string>
#include <algorithm>
#include <thread>

// Path to the JSON file for saving and loading favourites data
const std::string FAVOURITES_FILE = "favourites.json";

// Function to save favourites data to a JSON file
void SaveFavouritesToFile(const CommonObjects& common) {
    nlohmann::json jsonFavourites;
    for (const auto& city : common.favouriteCities) {
        jsonFavourites[city.first] = city.second;
    }
    std::ofstream file(FAVOURITES_FILE);
    if (file.is_open()) {
        file << jsonFavourites.dump(4);  // Save the data with pretty formatting
        file.close();
    }
}

// Function to load favourites data from a JSON file
void LoadFavouritesFromFile(CommonObjects& common) {
    std::ifstream file(FAVOURITES_FILE);
    if (file.is_open()) {
        nlohmann::json jsonFavourites;
        file >> jsonFavourites;
        for (auto& [cityName, weatherData] : jsonFavourites.items()) {
            common.favouriteCities.push_back({ cityName, weatherData });
        }
        file.close();
    }
}

// Thread function for automatic weather updates
void AutoUpdateThread(CommonObjects* common, int updateIntervalSeconds) {
    while (!common->exit_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(updateIntervalSeconds));

        {
            std::lock_guard<std::mutex> lock(common->data_mutex);
            // Add code to update the weather data here
            if (!common->url.empty()) {
                // Update the weather data here using the URL stored in common->url
            }
        }
    }
}

// Function to draw the main application window
void DrawAppWindow(void* common_ptr)
{
    auto common = (CommonObjects*)common_ptr;

    // Make the window flexible and use the full screen
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("Weather App", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Apply additional padding for improved spacing
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

    // Set text colors
    ImVec4 greenColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green color
    ImVec4 blueColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f);  // Blue color
    ImVec4 redColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);   // Red color

    ImGui::TextColored(greenColor, "Enter City Name:");

    ImGui::Spacing();

    static char buff[200];
    static bool showError = false; // Variable to display an error message
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f); // Make the input field 60% of the window width
    ImGui::InputText("City", buff, sizeof(buff));
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Search", ImVec2(120, 40))) { // Make buttons slightly larger
        std::lock_guard<std::mutex> lock(common->data_mutex);
        common->url = buff;
        showError = false; // Reset error message when searching again
    }

    ImGui::SameLine();
    if (ImGui::Button("Add To Favourites", ImVec2(160, 40))) {
        std::lock_guard<std::mutex> lock(common->data_mutex);
        common->favouriteCities.push_back({ buff, common->jsonData });
        SaveFavouritesToFile(*common); // Save favourites to the file
    }

    ImGui::SameLine();  // Add the Favourites button on the same line
    if (ImGui::Button("View Favourites", ImVec2(160, 40))) {
        ImGui::OpenPopup("FavouritesPopup");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(blueColor, "Fetched Weather Data:");

    ImGui::Spacing();

    // Lock shared variables and parse the JSON data
    {
        std::lock_guard<std::mutex> lock(common->data_mutex);
        if (!common->jsonData.empty()) {
            try {
                auto json_result = nlohmann::json::parse(common->jsonData);

                if (json_result.contains("location") && json_result.contains("current")) {
                    // Extract location and current weather information
                    auto location = json_result["location"];
                    auto current = json_result["current"];

                    ImGui::TextColored(greenColor, "Location:");
                    ImGui::BulletText("City: %s", location["name"].get<std::string>().c_str());
                    ImGui::BulletText("Region: %s", location["region"].get<std::string>().c_str());
                    ImGui::BulletText("Country: %s", location["country"].get<std::string>().c_str());
                    ImGui::Separator();

                    // Display the current weather conditions
                    ImGui::TextColored(blueColor, "Current Weather:");
                    ImGui::Text("%s", current["condition"]["text"].get<std::string>().c_str());

                    // Display weather information in a table
                    if (ImGui::BeginTable("WeatherTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
                        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                        ImGui::TableHeadersRow();

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Temperature (C)");
                        float temperature = current["temp_c"].get<float>();
                        ImVec4 tempColor = temperature > 30.0f ? redColor : blueColor;
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextColored(tempColor, "%.2f", temperature);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Humidity");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d%%", current["humidity"].get<int>());

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Wind Speed (kph)");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.2f", current["wind_kph"].get<float>());

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Wind Direction");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", current["wind_dir"].get<std::string>().c_str());

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Pressure (mb)");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.2f", current["pressure_mb"].get<float>());

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Precipitation (mm)");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.2f", current["precip_mm"].get<float>());

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Cloud Coverage");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d%%", current["cloud"].get<int>());

                        ImGui::EndTable();
                    }
                }
                else {
                    showError = true; // Set error message if the data is incorrect or missing
                }

            }
            catch (const std::exception& e) {
                showError = true; // Set error message if JSON parsing fails
            }
        }
        else if (showError) {
            ImGui::TextColored(redColor, "Error: Invalid city name or data not available. Please enter a valid city.");
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Search For A Valid City Please!");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginPopup("FavouritesPopup")) {
        ImGui::TextColored(greenColor, "Favourites:");
        ImGui::Separator();

        // Sorting options
        static int sortOption = 0;
        const char* sortOptions[] = { "Alphabetic", "Temperature", "Country" };
        ImGui::Combo("Sort By", &sortOption, sortOptions, IM_ARRAYSIZE(sortOptions));

        // Apply sorting based on the selected option
        if (sortOption == 0) {
            // Sort alphabetically
            std::sort(common->favouriteCities.begin(), common->favouriteCities.end(),
                [](const auto& a, const auto& b) {
                    return a.first < b.first;
                });
        }
        else if (sortOption == 1) {
            // Sort by temperature
            std::sort(common->favouriteCities.begin(), common->favouriteCities.end(),
                [](const auto& a, const auto& b) {
                    try {
                        auto aJson = nlohmann::json::parse(a.second);
                        auto bJson = nlohmann::json::parse(b.second);
                        float aTemp = aJson["current"]["temp_c"].get<float>();
                        float bTemp = bJson["current"]["temp_c"].get<float>();
                        return aTemp < bTemp;
                    }
                    catch (...) {
                        return false;
                    }
                });
        }
        else if (sortOption == 2) {
            // Sort by country
            std::sort(common->favouriteCities.begin(), common->favouriteCities.end(),
                [](const auto& a, const auto& b) {
                    try {
                        auto aJson = nlohmann::json::parse(a.second);
                        auto bJson = nlohmann::json::parse(b.second);
                        std::string aCountry = aJson["location"]["country"].get<std::string>();
                        std::string bCountry = bJson["location"]["country"].get<std::string>();
                        return aCountry < bCountry;
                    }
                    catch (...) {
                        return false;
                    }
                });
        }

        for (auto it = common->favouriteCities.begin(); it != common->favouriteCities.end(); ) {
            ImGui::TextColored(blueColor, "%s", it->first.c_str());
            ImGui::SameLine();
            // Add button to remove the city
            if (ImGui::Button(("Remove##" + it->first).c_str())) {
                it = common->favouriteCities.erase(it); // Remove the city
                SaveFavouritesToFile(*common); // Update favourites file after removal
            }
            else {
                ImGui::Separator();

                try {
                    auto json_result = nlohmann::json::parse(it->second);

                    if (json_result.contains("location") && json_result.contains("current")) {
                        auto location = json_result["location"];
                        auto current = json_result["current"];

                        ImGui::BulletText("Temperature (C): %.2f", current["temp_c"].get<float>());
                        ImGui::BulletText("Humidity: %d%%", current["humidity"].get<int>());
                        ImGui::BulletText("Wind Speed (kph): %.2f", current["wind_kph"].get<float>());
                        ImGui::BulletText("Pressure (mb): %.2f", current["pressure_mb"].get<float>());
                        ImGui::BulletText("Cloud Coverage: %d%%", current["cloud"].get<int>());
                        ImGui::Separator();
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid data.");
                    }
                }
                catch (const std::exception& e) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to parse JSON: %s", e.what());
                }

                ++it; // Move to the next city
            }
        }
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2); // Revert padding values to default
    ImGui::End();
}

// Thread operator function for drawing the app window
void DrawThread::operator()(CommonObjects& common)
{
    // Load favourites from file on startup
    LoadFavouritesFromFile(common);

    // Create a thread for automatic updates
    std::jthread autoUpdateThread(AutoUpdateThread, &common, 60); // Update every 60 seconds
    GuiMain(DrawAppWindow, &common);
    common.exit_flag = true;
}
