#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

int main()
{
    std::string cmd;
    bool inUse = true;
    sf::Music music;
    music.openFromFile("Music/LaNocheMasLinda.mp3");

    std::cout << "Commands for music player: \n play music(play), pause music(pause), quit(escape), loop(loop) " << std::endl;

    while (inUse) {
         std::cin >> cmd;
        if (cmd == "play") {
            music.play();
        }
        if (cmd == "pause") {
            music.pause();
        }
        if (cmd == "quit") {
            return 0;
        }
    }

}
