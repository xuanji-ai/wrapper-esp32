#include "nlogger.hpp"

NLogger::NLogger() : separator_(": "), suffix_("\n") {
}

NLogger::NLogger(const std::string& prefix) : prefix_(prefix), separator_(": "), suffix_("\n") {
}

NLogger::~NLogger() {
}

void NLogger::SetPrefix(const std::string& prefix) {
    prefix_ = prefix;
}

void NLogger::SetSeparator(const std::string& separator) {
    separator_ = separator;
}

void NLogger::SetSuffix(const std::string& suffix) {
    suffix_ = suffix;
}

void NLogger::SetTags(const std::string& tags) {
    tags_ = tags;
    tags_list_.clear();
    tags_list_.push_back(tags);
}

void NLogger::SetTags(const std::vector<std::string>& tags) {
    tags_list_ = tags;
    RebuildTags();
}

void NLogger::SetTags(const std::initializer_list<std::string>& tags) {
    tags_list_ = tags;
    RebuildTags();
}

void NLogger::SetTag(int index, const std::string& tag) {
    if (index >= 0 && index < static_cast<int>(tags_list_.size())) {
        tags_list_[index] = tag;
        RebuildTags();
    }
}

void NLogger::ClearTags() {
    tags_list_.clear();
    tags_.clear();
}

void NLogger::RebuildTags() {
    tags_.clear();
    for (const auto& tag : tags_list_) {
        tags_ += tag;
    }
}
