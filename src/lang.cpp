#include "lang.h"

#include <filesystem>
#include <string>
#include <unordered_map>

#include "log.h"

namespace lang {

constexpr std::string_view LANG_DIR_PATH = "./lang";

const std::unordered_map<std::string, std::string> locale_map = {
    {"en_us", "English (US)"},
    {"ja_jp", "日本語"},
};

enum class LangKeys {
    TEST_TEXT,
};

std::unordered_map<std::string, LangKeys> lang_file_key_map = {
    {"test_text", LangKeys::TEST_TEXT},
};

std::unordered_map<LangKeys, std::string> lang_map;

bool load_lang_file(std::string locale) {
    const std::string lang_file = std::string(LANG_DIR_PATH) + "/" + locale + ".lang";
    if (!std::filesystem::is_regular_file(lang_file)) {
        log::error("Not found " + locale + ".lang");
        return false;
    }
}

bool load_translation(std::string locale) {
    const bool r1 = load_lang_file("en_us");
    if (locale == "en_us") {
        return r1;
    }

    const bool r2 = load_lang_file(locale);
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