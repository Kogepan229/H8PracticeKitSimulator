#pragma once

#include <string>
#include <unordered_map>

namespace lang {

extern const std::unordered_map<std::string, std::string> locale_map;

enum class LangKeys;

bool load_translation(std::string locale);

std::string translate(LangKeys lang_key);

}  // namespace lang