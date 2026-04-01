#pragma once
#include <filesystem>
#include <optional>


class GSCreator {
public:
    static std::optional<std::filesystem::path> findCS2CFG(); // метод для пошуку папки з грою
    static void createConfig(const std::filesystem::path& path); // метод для створення конфігураційного файлу
};