#!/usr/bin/env bash
set -eu

if [ "$#" -ne 1 ]; then
  echo "usage: $0 <output-directory>" >&2
  exit 2
fi

fixture_root=$1
work_root="$fixture_root/source"
mkdir -p "$fixture_root" "$work_root"

python3 - "$work_root" <<'PY'
from pathlib import Path
import sys

root = Path(sys.argv[1])

def sega_user_sector(number: int) -> bytes:
    data = bytearray(2048)
    data[0:4] = b"SEGA"
    data[4:16] = b"DISCSYSTEM  "
    data[0x100] = number
    data[0x180:0x18e] = b"SYNTHETIC-0001"
    return bytes(data)

def mode1_raw_sector(number: int) -> bytes:
    data = bytearray(2352)
    data[0:12] = bytes([0x00] + [0xff] * 10 + [0x00])
    data[15] = 1
    data[16:16 + 2048] = sega_user_sector(number)
    return bytes(data)

raw = b"".join(mode1_raw_sector(n) for n in range(4))
iso = b"".join(sega_user_sector(n) for n in range(4))
audio = b"".join(bytes([0x40 + n]) * 2352 for n in range(3))

def put(case: str, name: str, content: bytes | str) -> None:
    path = root / case / name
    path.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(content, str):
        path.write_text(content, encoding="utf-8", newline="\n")
    else:
        path.write_bytes(content)

cue = 'FILE "disc.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n'
for case in ("one", "solid", "non-solid"):
    put(case, "game.cue", cue)
    put(case, "disc.bin", raw)

put("multi", "game.cue", 'FILE "data.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\nFILE "audio.bin" BINARY\n  TRACK 02 AUDIO\n    INDEX 01 00:00:00\n')
put("multi", "data.bin", raw)
put("multi", "audio.bin", audio)

put("nested", "Collection/Disc/game.cue", cue)
put("nested", "Collection/Disc/disc.bin", raw)

put("case", "GAME.CUE", 'FILE "DISC.BIN" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n')
put("case", "disc.bin", raw)

put("unicode", "Jogos/Álbum.cue", 'FILE "Faixa_日本.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n')
put("unicode", "Jogos/Faixa_日本.bin", raw)

put("iso", "Disc/Game.iso", iso)

put("ambiguous", "A/one.cue", 'FILE "one.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n')
put("ambiguous", "A/one.bin", raw)
put("ambiguous", "B/two.cue", 'FILE "two.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n')
put("ambiguous", "B/two.bin", raw)

put("missing", "missing.cue", 'FILE "missing.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n')
put("no-disc", "readme.txt", "synthetic fixture with no disc image\n")

put("traversal", "Sub/game.cue", 'FILE "../../outside.bin" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n')
put("traversal", "outside.bin", raw)

put("collision", "game.cue", 'FILE "TRACK.BIN" BINARY\n  TRACK 01 MODE1/2352\n    INDEX 01 00:00:00\n')
put("collision", "Track.bin", raw)
put("collision", "track.bin", raw)
PY

make_archive() {
  output=$1
  source=$2
  solid=$3
  (cd "$source" && 7z a -t7z -bd -y "-ms=$solid" -mtc=off "$output" . >/dev/null)
}

output_path=$(cd "$fixture_root" && pwd)
make_archive "$output_path/one-cue-one-bin.7z" "$work_root/one" on
make_archive "$output_path/multi-track.7z" "$work_root/multi" on
make_archive "$output_path/nested.7z" "$work_root/nested" on
make_archive "$output_path/case-mismatch.7z" "$work_root/case" on
make_archive "$output_path/unicode.7z" "$work_root/unicode" on
make_archive "$output_path/solid.7z" "$work_root/solid" on
make_archive "$output_path/non-solid.7z" "$work_root/non-solid" off
make_archive "$output_path/direct-iso.7z" "$work_root/iso" on
make_archive "$output_path/ambiguous-cues.7z" "$work_root/ambiguous" on
make_archive "$output_path/missing-bin.7z" "$work_root/missing" on
make_archive "$output_path/no-disc.7z" "$work_root/no-disc" on
make_archive "$output_path/traversal.7z" "$work_root/traversal" on
make_archive "$output_path/case-collision.7z" "$work_root/collision" on

(cd "$work_root/one" && 7z a -t7z -bd -y -psecret -mhe=on "$output_path/password.7z" . >/dev/null)
cp "$output_path/one-cue-one-bin.7z" "$output_path/corrupt.7z"
python3 - "$output_path/corrupt.7z" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
data = path.read_bytes()
path.write_bytes(data[:max(0, len(data) // 2)])
PY

rm -rf "$work_root"
echo "Generated synthetic archive-disc fixtures in $fixture_root"
