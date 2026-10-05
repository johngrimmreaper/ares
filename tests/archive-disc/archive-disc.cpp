#include <nall/nall.hpp>
#include <nall/vfs.hpp>
using namespace nall;

#include <nall/main.hpp>
#include <nall/decode/disc-archive.hpp>
#include <mia/mia.hpp>

#include <cstdlib>
#include <string_view>

namespace {

u32 failures = 0;

auto fixture(const string& root, const string& name) -> string {
  return {root, root.endsWith("/") ? "" : "/", name};
}

auto contains(const string& value, std::string_view expected) -> bool {
  return std::string_view(value.data(), value.size()).find(expected) != std::string_view::npos;
}

auto check(bool condition, const string& name, const string& detail = {}) -> void {
  if(condition) {
    print("PASS  ", name, "\n");
    return;
  }
  failures++;
  print("FAIL  ", name);
  if(detail) print(": ", detail);
  print("\n");
}

auto expectDiscArchive(const string& root, const string& archiveName, const string& testName) -> void {
  Decode::DiscArchive archive;
  auto opened = archive.open(fixture(root, archiveName));
  check(opened, {testName, " opens"}, archive.error());
  if(!opened) return;

  auto first = archive.readDataSector(0);
  check(first.size() == 2048, {testName, " reads data sector"}, archive.error());
  check(first.size() >= 4 && !memory::compare(first.data(), "SEGA", 4), {testName, " preserves Sega header"});

  auto third = archive.readDataSector(2);
  auto repeat = archive.readDataSector(0);
  check(third.size() == 2048 && third[0x100] == 2, {testName, " random sector read"}, archive.error());
  check(repeat == first, {testName, " repeated sector read is stable"}, archive.error());
}

auto expectFailure(const string& root, const string& archiveName, std::string_view expected, const string& testName) -> void {
  Decode::DiscArchive archive;
  if(!archive.open(fixture(root, archiveName))) {
    check(contains(archive.error(), expected), testName, archive.error());
    return;
  }

  auto sector = archive.readDataSector(0);
  check(sector.empty() && contains(archive.error(), expected), testName, archive.error());
}

auto testMountedDisc(const string& root) -> void {
  string error;
  auto disc = vfs::cdrom::open(fixture(root, "multi-track.7z"), &error);
  check((bool)disc, "CUE with data and audio tracks mounts", error);
  if(!disc) return;
  vfs::file& mounted = *disc;

  auto dataOffset = 2448ull * (CD::LeadInSectors + CD::LBAtoABA(2)) + 16;
  mounted.seek(dataOffset);
  std::vector<u8> data(2048);
  mounted.read(data);
  check(data[0x100] == 2, "mounted Sega CD random data-sector bytes");

  auto audioOffset = 2448ull * (CD::LeadInSectors + CD::LBAtoABA(4));
  mounted.seek(audioOffset);
  std::vector<u8> audio(2352);
  mounted.read(audio);
  check(audio.front() == 0x40 && audio.back() == 0x40, "mounted Sega CD audio-track mapping");

  mounted.seek(dataOffset);
  std::vector<u8> repeated(2048);
  mounted.read(repeated);
  check(repeated == data, "mounted disc repeated random access is stable");
}

auto testMediaDetection(const string& root) -> void {
  auto path = fixture(root, "one-cue-one-bin.7z");
  auto matches = mia::identify(path);
  bool found = false;
  for(auto& match : matches) if(match == "Mega CD") found = true;
  check(found, "MIA recognizes the SevenZip fixture as Mega CD");

  auto medium = mia::Medium::create("Mega CD");
  auto result = medium->load(path);
  check(result == successful, "Mega CD load reaches mounted cd.rom", result.info);
  check(medium->pak && medium->pak->read("cd.rom"), "Mega CD pak exposes cd.rom");
}

}  // namespace

auto nall::main(Arguments arguments) -> void {
  if(arguments.size() != 1) {
    print("usage: archive-disc-test <generated-fixture-directory>\n");
    std::exit(EXIT_FAILURE);
  }
  auto root = arguments[0];

  expectDiscArchive(root, "one-cue-one-bin.7z", "one CUE plus one BIN");
  expectDiscArchive(root, "multi-track.7z", "one CUE plus multiple BIN tracks");
  expectDiscArchive(root, "nested.7z", "nested archive directory");
  expectDiscArchive(root, "case-mismatch.7z", "unique case-insensitive CUE reference");
  expectDiscArchive(root, "unicode.7z", "Unicode member names");
  expectDiscArchive(root, "solid.7z", "solid SevenZip archive");
  expectDiscArchive(root, "non-solid.7z", "non-solid SevenZip archive");
  expectDiscArchive(root, "direct-iso.7z", "single direct ISO image");

  expectFailure(root, "ambiguous-cues.7z", "multiple CUE", "multiple unrelated CUE files are rejected");
  expectFailure(root, "missing-bin.7z", "not found", "missing CUE member reports its name");
  expectFailure(root, "corrupt.7z", "corrupt", "corrupt SevenZip archive fails cleanly");
  expectFailure(root, "password.7z", "password-protected", "password-protected SevenZip archive is explicit");
  expectFailure(root, "no-disc.7z", "no supported", "archive without supported media is rejected");
  expectFailure(root, "traversal.7z", "Unsafe CUE", "traversal-style CUE reference is rejected");
  expectFailure(root, "case-collision.7z", "Ambiguous case-insensitive", "case-colliding members are not guessed");

  testMountedDisc(root);
  testMediaDetection(root);

  print("\n", failures ? "FAILED" : "PASSED", ": ", failures, " failed checks\n");
  std::exit(failures ? EXIT_FAILURE : EXIT_SUCCESS);
}
