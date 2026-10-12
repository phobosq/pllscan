# AM3 Platform Master Record

## Board Identity

- Board: Gigabyte GA-790FXTA-UD5
- CPU socket: AM3
- Northbridge: AMD 790FX
- Southbridge: AMD SB750
- PLL chip: Realtek RTM880N-793
- PLL SMBus address: 0x69
- Expected status: unverified until live hardware dump confirms signature and register map

## Current Known Good Configuration

| Field | Value |
|---|---|
| Chipset key | AMD_SB750 |
| Northbridge | AMD 790FX |
| PLL type | RTM880N_793 |
| PLL address | 0x69 |
| Board profile | Gigabyte GA-790FXTA-UD5 |
| Expected read protocol | default / chip-specific fallback |
| Known probe behavior | SMBus scan and PLL probe should respond at 0x69 |

## Runtime Data Snapshot

This file is intended to be updated with actual measured data from the target board once testing begins.

### Required live measurements

- `pllscan platform` output
- `pllscan smbus scan` output
- `pllscan pll probe` output
- `pllscan pll dump 0x69` output
- `pllscan pll decode 0x69` output
- Any register 0 / signature readback before writes

### Register baseline to capture

| Register | Purpose | Measured value |
|---|---|---|
| 0x00 | chip signature / revision |  |
| 0x01 | CPU / FSB PLL control |  |
| 0x02 | CPU / FSB N divider |  |
| 0x03 | CPU / FSB M divider |  |
| 0x04 | PCIe reference control |  |
| 0x05 | DDR / memory clock control |  |
| 0x06 | spread spectrum amount |  |
| 0x07 | spread spectrum enable / mode |  |
| 0x08 | HT / SB reference clock |  |
| 0x09 | clock output enables |  |

## Known Master Data Entries

These values are already represented in the project config and should be treated as the starting point, not the final hardware truth:

- [Boards.ini](Boards.ini): board profile for Gigabyte GA-790FXTA-UD5
- [PllChips.ini](PllChips.ini): RTM880N_793 and ICS-related chip signatures
- [Chipsets.ini](Chipsets.ini): chipset + SMBus BAR metadata for AMD SB750

## Verification Checklist

Before any write operation, confirm:

- [ ] SMBus device responds at 0x69
- [ ] `pll probe` identifies RTM880N_793 or the expected alternate signature
- [ ] `pll dump 0x69` completes cleanly
- [ ] register 0 matches the verified chip signature
- [ ] board defaults in [Boards.ini](Boards.ini) align with the live dump
- [ ] a full dump is saved before any `--force` write

## Notes for Next Test Run

- Use the board’s actual live dump to replace placeholder values in [PllChips.ini](PllChips.ini).
- If the signature is different from the current assumption, update `SignatureMask` and `SignatureValue` before attempting writes.
- Preserve raw output from the first successful dump as the baseline for future comparisons.
