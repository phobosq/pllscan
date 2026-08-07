# pllscan

A lightweight command-line replacement for [SetFSB](http://www7a.biglobe.ne.jp/~k_kenj/setfsb/) that gives direct, scriptable control over onboard PLL clock chips via SMBus. Lets you scan, read, decode, and write PLL registers to adjust FSB, HT, PCIe, and memory reference clocks on supported platforms.

Consists of two components:
- **`plldrv.sys`** — a Windows kernel-mode driver that exposes SMBus, PCI config, and I/O port access via IOCTL
- **`pllscan.exe`** — a CLI that talks to the driver and implements platform detection, PLL identification, register dumping/decoding, and controlled writes

---

## ⚠️ Warning — Read Before Use

> **This software directly manipulates hardware clock registers. Incorrect values can cause system instability, data loss, or permanent hardware damage including CPU, motherboard, and memory.**
>
> - Always save a full register dump (`pllscan pll dump <addr>`) **before** writing anything.
> - Never write to the PLL without the `--force` flag — it is required by design to prevent accidents.
> - The author provides this software **as-is, with no warranty of any kind**. Use at your own risk.
> - **The author is not responsible for any damage to hardware, data, or systems resulting from the use of this software.**

---

## Supported Platforms

### Southbridges / PCH (auto-detected via PCI)

| Family | Examples |
|---|---|
| AMD SB600 / SB700 / SB710 / SB750 / SB800 | AMD 790FX, 790GX, 880G platforms |
| Intel ICH7 / ICH8 / ICH9 / ICH10 | LGA775 platforms |
| Intel PCH 5-series (Ibex Peak) | LGA1156 — P55, H55, H57, Q57 |
| Intel PCH 6-series (Cougar Point) | LGA1155 — Z68, P67, H67, Z77 |
| Intel PCH 7-series (Panther Point) | LGA1155 — Z77, H77, B75 |
| Intel PCH 8-series (Lynx Point) | LGA1150 — Z87, H87, B85 |
| Intel PCH 9-series (Wildcat Point) | LGA1150 — Z97, H97 |

### PLL Chips

| Chip | Address |
|---|---|
| RTM880N-793 (Realtek) | 0x69 |
| ICS LPRS477 | 0x69 |
| ICS LPRS918 | 0x69 |
| ICS LPRS919 | 0x69 |

### Tested Boards

- Gigabyte GA-790FXTA-UD5 (AMD SB750 + RTM880N-793)
- Gigabyte GA-P55-UD4 (Intel PCH5 + ICS LPRS477)
- ASUS P7P55D-E Pro (Intel PCH5 + ICS LPRS477)
- Gigabyte GA-Z68X-UD3H (Intel PCH6 + ICS LPRS918)
- ASUS P8Z68-V Pro (Intel PCH6 + ICS LPRS918)
- Gigabyte GA-Z77X-UD5H (Intel PCH7 + ICS LPRS919)

---

## Requirements

- Windows XP or Windows 7 (x86)
- Administrator privileges
- `plldrv.sys` installed and running before invoking `pllscan.exe`
- WDK (for building the driver); Visual Studio (for building the CLI)

---

## Quick Start

### 1. Detect your platform

```
pllscan platform
```

Prints the detected southbridge name, vendor/device IDs, and SMBus base address.

### 2. Scan the SMBus for devices

```
pllscan smbus scan
```

Probes addresses 8–119. The PLL clock chip typically responds at address **105 (0x69)**.

### 3. Identify the PLL chip

```
pllscan pll probe
```

Reads register 0 from candidate addresses and matches against the chip database.

### 4. Dump all PLL registers

```
pllscan pll dump 0x69
```

**Save this output before writing anything.**

### 5. Decode register fields

```
pllscan pll decode 0x69
```

Annotates known or estimated fields against the chip datasheet.

### 6. Write a register (destructive — requires `--force`)

```
pllscan smbus write <addr> <reg> <val> --force
```

See [`MANUAL.md`](MANUAL.md) for the full command reference.

---

## Command Reference

```
pllscan [--smbus-base <base>] <command> ...

  platform
  pci scan
  pci read <bus> <dev> <fn> <reg>
  port read  <port>
  port write <port> <value>
  smbus read  <addr> <reg>
  smbus write <addr> <reg> <val> --force
  smbus scan
  pll probe
  pll dump   <addr>
  pll decode <addr>
```

All numeric arguments accept decimal or `0x`-prefixed hex. If platform detection fails, pass `--smbus-base <addr>` before the command to override the SMBus base address.

---

## Project Structure

```
driver/          Kernel-mode driver (WDK build system)
  DriverEntry.c  Driver init, device/symlink creation
  SMBus.c        Raw SMBus read/write via I/O ports
  Pci.c          PCI config space access
  Port.c         Direct I/O port access
  Ioctl.c/h      IOCTL dispatch

cli/
  pllscan/
    main.c               Entry point and command dispatch
    DriverApi.c/h        Userspace IOCTL interface
    Commands/            One .c file per command group
    Hardware/            Platform, SMBus, chipset, PLL, board databases

Chipsets.ini     External chipset database (overrides compiled-in table)
Boards.ini       Board profiles with PLL type, address, and clock defaults
pllscan.cfg      Runtime config (database paths, selected board)
MANUAL.md        Full user manual
```

---

## License

This software is provided **as-is**, without warranty of any kind, express or implied. The author disclaims all liability for any damage arising from its use. See [LICENSE](LICENSE) if present, or treat as source-available for personal and research use.
