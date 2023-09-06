#include "lang.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "log.h"
#include "utils.h"

namespace lang {

constexpr std::string_view LANG_DIR_PATH = "./lang";

const std::unordered_map<std::string, std::string> locale_map = {
    {"en_us", "English (US)"},
    {"ja_jp", "日本語"},
};

std::unordered_map<std::string, LangKeys> lang_file_key_map = {
    {"language", LangKeys::LANGUAGE},
    {"test.text", LangKeys::TEST_TEXT},
};

std::unordered_map<LangKeys, std::string> lang_map;

bool load_lang_file(std::string locale) {
    const std::string lang_file_path = std::string(LANG_DIR_PATH) + "/" + locale + ".lang";
    if (!std::filesystem::is_regular_file(lang_file_path)) {
        log::error("Not found " + locale + ".lang");
        return false;
    }

    /*
     *  Read translation from lang file.
     */
    std::unordered_map<std::string, std::string> read_translation_map;
    {
        std::ifstream file;
        file.open(lang_file_path, std::ios::in);
        if (file.fail()) {
            log::error("Could not open " + locale + ".lang");
            return false;
        }
        std::string reading_line_buffer;
        while (std::getline(file, reading_line_buffer)) {
            const auto [key, str] = utils::split_str_on_first(reading_line_buffer, '=');
            read_translation_map.emplace(key, str);
        }
    }

    /*
     *  Push read translation to lang_map
     */
    auto iter = lang_file_key_map.begin();
    while (iter != lang_file_key_map.end()) {
        if (read_translation_map.contains(iter->first)) {
            lang_map.emplace(iter->second, read_translation_map[iter->first]);
        } else {
#ifndef NDEBUG
            if (locale == "en_us") {
                log::error("Not found lang key [" + iter->first + "] in en_us.");
            }
#endif
        }
        iter++;
    }

    return true;
}

/*
 *  en_us.lang must be loaded first
 */
bool load_translation(std::string locale) {
    const bool r1 = load_lang_file(locale);
    if (locale == "en_us") {
        return r1;
    }
    const bool r2 = load_lang_file("en_us");
    return r1 || r2;
}

std::string translate(LangKeys lang_key) {
    if (lang_map.contains(lang_key)) {
        return lang_map[lang_key];
    } else {
        return "[Failed translate]";
    }
}

}  // namespace lang