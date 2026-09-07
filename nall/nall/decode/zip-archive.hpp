#pragma once

#include <nall/decode/archive.hpp>
#include <nall/decode/zip.hpp>

namespace nall::Decode {

// Adapter that exposes the existing ZIP decoder through the format-neutral
// Archive interface while enforcing the same member-name policy as 7z.
struct ZIPArchive : Archive {
  ~ZIPArchive() override {
    close();
  }

  auto findFile(const string& filename) const -> const maybe<File> override {
    auto entry = findEntry(filename);
    if(!entry) return nothing;
    return entry->file;
  }

  auto files() const -> std::vector<File> override {
    std::vector<File> result;
    result.reserve(entries.size());
    for(auto& entry : entries) result.push_back(entry.file);
    return result;
  }

  auto open(const string& filename) -> bool override {
    close();
    errorMessage = {};
    if(!archive.open(filename)) {
      errorMessage = "The ZIP archive is corrupt or could not be opened.";
      return false;
    }

    entries.reserve(archive.file.size());
    for(auto& source : archive.file) {
      auto normalized = normalizeMemberName(source.name);
      if(!normalized) {
        errorMessage = {"Unsafe ZIP member name: ", source.name};
        close();
        return false;
      }
      for(auto& existing : entries) {
        if(existing.file.name == *normalized) {
          errorMessage = {"Duplicate normalized ZIP member name: ", *normalized};
          close();
          return false;
        }
      }

      Entry entry;
      entry.source = source;
      entry.file = convert(source, *normalized);
      entries.push_back(std::move(entry));
    }
    return true;
  }

  auto extract(const File& file) const -> std::vector<u8> override {
    auto entry = findEntry(file.name);
    if(!entry) return {};
    auto output = archive.extract(entry->source);
    if(output.size() != entry->file.size) {
      errorMessage = {"Failed to decompress ZIP member: ", entry->file.name};
      return {};
    }
    return output;
  }

  auto isDataUncompressed(const File& file) const -> bool override {
    auto entry = findEntry(file.name);
    return entry && archive.isDataUncompressed(entry->source);
  }

  auto dataViewIfUncompressed(const File& file) const -> std::span<const u8> override {
    auto entry = findEntry(file.name);
    if(!entry) return {};
    return archive.dataViewIfUncompressed(entry->source);
  }

  auto error() const -> string override {
    return errorMessage;
  }

  auto close() -> void override {
    archive.close();
    entries.clear();
  }

private:
  struct Entry {
    File file;
    ZIP::File source;
  };

  auto findEntry(const string& filename) const -> const Entry* {
    auto normalized = normalizeMemberName(filename);
    if(!normalized) {
      errorMessage = {"Unsafe archive member reference: ", filename};
      return nullptr;
    }

    for(auto& entry : entries) {
      if(entry.file.name == *normalized) {
        errorMessage = {};
        return &entry;
      }
    }

    const Entry* match = nullptr;
    for(auto& entry : entries) {
      if(!entry.file.name.iequals(*normalized)) continue;
      if(match) {
        errorMessage = {"Ambiguous case-insensitive archive member reference: ", *normalized};
        return nullptr;
      }
      match = &entry;
    }
    if(match) errorMessage = {};
    else errorMessage = {"Archive member not found: ", *normalized};
    return match;
  }

  static auto convert(const ZIP::File& source, const string& normalizedName) -> File {
    File target;
    target.name = normalizedName;
    target.data = source.data;
    target.size = source.size;
    target.csize = source.csize;
    target.cmode = source.cmode;
    target.crc32 = source.crc32;
    target.timestamp = source.timestamp;
    return target;
  }

  ZIP archive;
  std::vector<Entry> entries;
  mutable string errorMessage;
};

}
