# pllscan — User Manual

## Prerequisites

- **Driver loaded**: `plldrv.sys` must be installed and running before any command will work.
- **Run as Administrator**: required for driver communication and hardware port access.
- All numeric arguments accept **decimal** (plain number) or **hex** with `0x` prefix. Examples: `105` and `0x69` are equivalent.

---

## Recommended First-Run Sequence

### 1. Confirm platform detection

```
pllscan platform
```

Expected output: chipset name (e.g. `AMD SB750`), vendor/device IDs, and the SMBus base address (typically around `0x0B00`).  
If this fails, the southbridge was not recognized — check that the driver is loaded.

---

### 2. Scan for SMBus devices

```
pllscan smbus scan
```

Probes addresses 8–119 using Quick Read, then Byte Read as fallback.  
Output lists every responding address and which probe method succeeded.  
The PLL clock chip on AMD 790FX boards (e.g. RTM880N-793) typically appears at address **105 (0x69)**.

---

### 3. Read a register from the PLL chip

```
pllscan smbus read <addr> <reg>
```

| Argument | Accepts        | Examples        |
|----------|----------------|-----------------|
| `addr`   | decimal or hex | `105` or `0x69` |
| `reg`    | decimal or hex | `0` or `0x00`   |

Example — read register 0 (chip signature) from addr 0x69:
```
pllscan smbus read 0x69 0
```

---

### 4. Identify the PLL chip automatically

```
pllscan pll probe
```

Checks addresses 0x69, 0x6A, and 0x58–0x5F.  
Reads register 0 from each responding address and attempts to match against the chip database.  
If the chip is not identified by signature (signature not yet verified), the raw R0 value is printed alongside a likely chip name based on the address.

---

### 5. Dump all registers from the PLL chip

```
pllscan pll dump <addr>
```

| Argument | Accepts        | Examples        |
|----------|----------------|-----------------|
| `addr`   | decimal or hex | `105` or `0x69` |

Example:
```
pllscan pll dump 105
```

Reads 32 registers (or the chip-specific count if identified) and prints an indexed table.  
**Save this output** — it is the baseline for correlating register fields against the datasheet.

---

### 6. Decode register fields

```
pllscan pll decode <addr>
```

Same as `dump` but annotates known or estimated fields.  
Field names for RTM880N-793 are derived from RTM880N-792 reverse engineering and are marked as estimates until verified on hardware.

Example:
```
pllscan pll decode 105
```

---

### 7. Write a single SMBus byte  ⚠ destructive

```
pllscan smbus write <addr> <reg> <val> --force
```

| Argument | Accepts        | Examples      |
|----------|----------------|---------------|
| `addr`   | decimal or hex | `105` `0x69`  |
| `reg`    | decimal or hex | `9` `0x09`    |
| `val`    | decimal or hex | `0` `0x00`    |

The `--force` flag is **mandatory**. The command exits with an error if it is omitted.  
Do not write to the PLL chip without first saving a full register dump (`pll dump`) so you can restore the original values.

---

## All Commands Reference

```
GLOBAL OPTIONS:
  pllscan [--smbus-base <base>] <command> ...

PLATFORM:
  pllscan platform

PCI:
  pllscan pci scan
  pllscan pci read <bus> <dev> <fn> <reg>       (all decimal)

PORT:
  pllscan port read  <port>                      (decimal)
  pllscan port write <port> <value>              (decimal)

SMBUS:
  pllscan smbus read  <addr> <reg>               (decimal or 0x hex)
  pllscan smbus write <addr> <reg> <val> --force (decimal or 0x hex)
  pllscan smbus scan

PLL:
  pllscan pll probe
  pllscan pll dump   <addr>                      (decimal or 0x hex)
  pllscan pll decode <addr>                      (decimal or 0x hex)
```

---

## Notes

- `smbus scan` may report false negatives on Quick Read for chips that do not implement the Quick Command protocol. Byte Read fallback handles most cases.
- PLL chip signature values are configured in `PllChips.ini` per chip key section. After running `pll dump 105` and correlating R0 against the -792/-793 documentation, update `SignatureMask` and `SignatureValue` in `PllChips.ini`.
- Identity is key-based:
  - `PllChips.ini` section name is the canonical PLL key.
  - `Boards.ini` `PllType` must match that PLL key.
  - `Chipsets.ini` `Type` is a runtime chipset key string.
- The CLI reads the SMBus base address from SB750 PCI config at startup. If platform detection fails (unsupported chipset), pass `--smbus-base` to override it manually:
  ```
  pllscan --smbus-base 0xB00 smbus scan
  pllscan --smbus-base 2816  pll dump 0x69
  ```
  The flag must come before the command name. Decimal and `0x`-prefixed hex are both accepted.
