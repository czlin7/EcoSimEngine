#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

class Assets {
    std::unordered_map<std::string, sf::Texture> m_textureMap;
    std::unordered_map<std::string, sf::Font> m_fontMap;
    std::unordered_map<std::string, std::string> m_fontPathMap;
    std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;
    std::unordered_map<std::string, std::unique_ptr<sf::Sound>> m_soundMap;

    void addTexture(const std::string& name, const std::string& path);
    void addFont(const std::string& name, const std::string& path);
    void addSound(const std::string& name, const std::string& path);

public:
    Assets();
    ~Assets();

    void loadFromFile(const std::string& path);

    [[nodiscard]] const sf::Texture& getTexture(const std::string& name) const;
    [[nodiscard]] const sf::Font& getFont(const std::string& name) const;
    [[nodiscard]] const std::string& getFontPath(const std::string& name) const;
    [[nodiscard]] sf::Sound& getSound(const std::string& name);

    [[nodiscard]] const std::unordered_map<std::string, sf::Texture>& getTextureMap() const;
    [[nodiscard]] std::unordered_map<std::string, std::unique_ptr<sf::Sound>>& getSoundMap();
};
