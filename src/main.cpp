#include <iostream>
#include <filesystem>
#include <list>
#include <random>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

struct Song {
    std::string name;
    std::filesystem::path path;
};

struct Playlist {
    std::string name;
    std::vector<Song> songs;
};

class MusicManager {
private:
    sf::Music music;
    std::vector<Song> queue;
    int queueIdx = -1;
    bool shuffleEnabled = false;

    void loadCurrent() {
        if (queueIdx<0 || queueIdx >= (int)queue.size())return;
        if (!music.openFromFile(queue[queueIdx].path.string())) {
            std::cerr << "failed to load music from file" << std::endl;
            return;
        }
        music.play();
    }

public:
    void playPlaylist(const Playlist& pl, int startingidx, bool shuffle) {
        queue = pl.songs;
        shuffleEnabled=shuffle;

        if (shuffleEnabled) {
            std::swap(queue[0], queue[startingidx]);
            std::shuffle(queue.begin()+1,queue.end(), std::mt19937{std::random_device{}()});
        }
        else {
            std::rotate(queue.begin(), queue.begin() + startingidx, queue.end());
            queueIdx=0;
        }
        loadCurrent();
    }

    void togglePause() {
        if (music.getStatus()==sf::Music::Status::Paused) {
            music.play();
        }
        else {
            music.pause();
        }
    }
    void next() {
        if (queue.empty()) return;
        queueIdx = (queueIdx + 1) % queue.size();
        loadCurrent();
    }

    void prev() {
        if (queue.empty()) return;
        queueIdx = (queueIdx-1) % queue.size();
        loadCurrent();
    }

    void update() {
        if (music.getStatus() == sf::Music::Status::Stopped && queueIdx>=0) {
            next();
        }
    }

    void setShuffle() {
        if (shuffleEnabled==true) {
            shuffleEnabled=false;
        }
        else {
            shuffleEnabled=true;
        }
    }

    void setVolume(float newVol) {
        music.setVolume(newVol);
    }

    bool getShuffleMode() {
        return shuffleEnabled;
    }

    bool isPlaying() {
        if (music.getStatus() == sf::Music::Status::Playing) {
            return true;
        }
        return false;
    }

};

std::vector<Playlist> loadPlaylists(const std::filesystem::path& musicRoot) {
    std::vector<Playlist> playlists;
    for (auto& dirEntry : std::filesystem::directory_iterator(musicRoot)) {
        if (!dirEntry.is_directory()) {
            continue;
        }
        else {
            Playlist pl;
            pl.name = dirEntry.path().filename().string();
            for (auto& fileEntry : std::filesystem::directory_iterator(dirEntry.path())) {
                auto ext = fileEntry.path().extension().string();
                if (ext == ".mp3" || ext == ".ogg" || ext == ".wav" || ext == ".flac") {
                    Song s;
                    s.path = fileEntry.path();
                    s.name = fileEntry.path().stem().string();
                    pl.songs.push_back(s);
                }
            }
            if (!pl.songs.empty()) {
                playlists.push_back(pl);
            }
        }
    }
    return playlists;
}

std::filesystem::path loadMusicRoot() {
    return std::filesystem::path(__FILE__).parent_path() / "Music/";
}

int PlaylistMenu(std::vector<Playlist> playlists, MusicManager& mm, bool shuffleMode, int windowSizeX, int windowSizeY) {
    static int selectedIdx = 0;
    ImGui::Begin("Playlists", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginCombo("Playlist",playlists[selectedIdx].name.c_str())) {
        for (int i = 0; i < int(playlists.size()); i++) {
            bool isSelected = selectedIdx==i;
            if (ImGui::Selectable(playlists[i].name.c_str(), isSelected)) {
                selectedIdx=i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    Playlist& pl = playlists[selectedIdx];
    if (ImGui::Button("Play Playlist", ImVec2(100, 25))) {
        mm.playPlaylist(playlists[selectedIdx], 0, false);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Shuffle", &shuffleMode);

    ImGui::SetNextWindowSize(ImVec2(300,100));
    ImGui::SetNextWindowPos(ImVec2(0,windowSizeY/2-125));
    ImGui::Begin("Songs", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    for (int j =0; j < (int)pl.songs.size(); j++) {
        if (ImGui::Selectable(pl.songs[j].name.c_str())) {
            mm.playPlaylist(pl, j, shuffleMode);
        }
    }
    ImGui::End();

    ImGui::End();

    return selectedIdx;
}

int main() {

    //music setup
    auto playlists = loadPlaylists(loadMusicRoot());
    MusicManager music_manager = MusicManager();

    //main window setup
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    int desktopSizeX = desktop.size.x;
    int desktopSizeY = desktop.size.y;
    unsigned windowSizeX = 400;
    unsigned windowSizeY = 400;

    sf::RenderWindow window(sf::VideoMode({windowSizeX, windowSizeY}), "Adrian Music Player");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        std::cerr << "Failed to initialize SFML." << std::endl;
        return 1;
    }

    sf::Clock deltaClock;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        windowSizeX = window.getSize().x;
        windowSizeY = window.getSize().y;

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::SetNextWindowPos(ImVec2(0,0));

        int selectedIdx = PlaylistMenu(playlists, music_manager, music_manager.getShuffleMode(),windowSizeX,windowSizeY);

        //Play and Pause
        ImGui::SetNextWindowPos(ImVec2(windowSizeX/2 - 75, windowSizeY-100));

        ImGui::Begin("Hello, world!",nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::Button("<<", ImVec2(25, 25))) {
            music_manager.prev();
        }

        ImGui::SameLine();
        if (ImGui::Button("< / ||", ImVec2(100, 50))) {
            music_manager.togglePause();
        }

        ImGui::SameLine();
        if (ImGui::Button(">>", ImVec2(25, 25))) {
            music_manager.next();
        }
        ImGui::End();

        //Exit Button
        ImGui::SetNextWindowPos(ImVec2(windowSizeX-75, 0));
        ImGui::Begin("Exit Button",nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("Exit", ImVec2(50, 50))) {
            window.close();
        };
        ImGui::End();

        //Volume Button
        ImGui::SetNextWindowPos(ImVec2(20, windowSizeY-200));
        ImGui::Begin("VolumeBar",nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        static float volume = 50.0f;
        if (ImGui::VSliderFloat("##VolumeSlider", ImVec2(20,150), &volume, 0.0f, 100.0f, "%.2f")) {
            music_manager.setVolume(volume);
        }
        ImGui::End();

        window.clear();
        music_manager.update();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}