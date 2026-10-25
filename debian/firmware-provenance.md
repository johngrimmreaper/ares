# Ares v148 firmware provenance audit

Status: **unresolved**. This is evidence, not a redistribution license.
Reviewed on 2026-10-25 against `debian/unstable` and Debian ares 148+dfsg-1.
Do not reinterpret Ares's general ISC license as permission for third-party ROMs.

## Inventory and build reachability

- 38 ROM files in `mia/Firmware/`, totalling 451,130 bytes.
- 36 listed in `mia/resource/resource.bml` and compiled through the
  generated `mia/resource/resource.cpp` into the `mia` static library.
- The 2 MSX2 files are not listed in that resource manifest but remain
  present in the unrepacked source archive.
- Exact checksums: `debian/firmware-manifest.sha256` (check from repo root
  with `sha256sum -c debian/firmware-manifest.sha256`).

| Firmware family | Files | Firmware family | Files |
| --- | ---: | --- | ---: |
| Game Boy | 3 | Game Boy Color | 2 |
| Mega Drive | 2 | Mega 32X | 3 |
| Nintendo 64 | 7 | Super Famicom | 13 |
| WonderSwan | 1 | WonderSwan Color | 1 |
| Pocket Challenge V2 | 1 | MSX2 | 2 |
| ZX Spectrum | 1 | ZX Spectrum 128 | 2 |

## Specific provenance evidence

- **MSX2/bios.rom:** contains the identifier `C-BIOS 0.29`,
  `cbios.sf.net`, and Japanese-region settings. Upstream commit
  `6baebd0f0` records an update to the C-BIOS Japanese revision.
  The [C-BIOS Association](https://cbios.sourceforge.net/) says its
  BIOS source and binaries are redistributable under BSD-2-Clause.
  **Pending:** match this precise binary to a released C-BIOS build
  and copy the correct copyright/attribution notice. Do not automatically
  extend that conclusion to `MSX2/sub.rom` without separate evidence.
- **Mega Drive/tmss.rom:** contains `SEGA GENESIS (C)SEGA 1990` and
  a Sega licensing notice. No permission for this exact binary was
  found in the examined Ares packaging/source materials.
- **ZX Spectrum/bios.rom** and **ZX Spectrum 128/bios.rom**: contain
  Sinclair Research copyright notices dated 1982 and 1986 respectively.
  Historical Amstrad emulator redistribution permission has been
  discussed, but its exact scope, modifications, and DFSG status
  require an authoritative rights review before declaring these free.
- **Other ROMs:** names identify platform firmware, but no individual
  license grants were found in the inspected tree. Their status is
  *unverified*, not categorically declared non-free by this audit.

## Debian archive comparison

Debian's published [ares 148+dfsg-1 source](https://sources.debian.org/src/ares/148%2Bdfsg-1/mia/)
still includes `mia/Firmware/`. The Debian packaging branch's
`Files-Excluded` stanza removes bundled `thirdparty/libchdr/*`, not
these ROMs (it contains the inactive text `#mia/Firmware/*`).
Consequently, the `+dfsg` suffix does **not** prove firmware clearance.

## Next actions

1. Match each hash to an upstream project, author, and explicit license or
   redistribution grant; prioritize the Sega/Sinclair-marked binaries.
2. For C-BIOS, verify exact release checksums and credit before changing
   `debian/copyright`.
3. Ask the rights holder or Debian Games Team for authoritative guidance
   where redistributability or DFSG compatibility is unclear.
4. Choose the packaging remediation only after the audit: retain with
   documented grant, replace with clearly licensed code, or exclude and
   adjust emulation/resource loading as needed.
5. Do not change firmware contents or builds during the Mega CD regression
   bisect without a controlled baseline comparison.
