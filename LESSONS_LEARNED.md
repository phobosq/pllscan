# Lessons Learned

## Things That Went Well

1. We tightened driver IOCTL buffer validation and access controls early, which reduced risk while debugging transport issues.
2. We fixed legacy build blockers (including secure device creation signature mismatches) and restored a reliable compile path.
3. We introduced deterministic PLL matching logic, which improved consistency when probe addresses overlap.
4. We moved key SMBus behavior into configuration (`SmbPollTimeout`, `SmbYieldEvery`, `SmbRecoverPolls`), enabling tuning without recompiles.
5. We iterated in small, testable changes across CLI and driver code, making regressions easier to isolate.

## Things To Avoid Later

1. Avoid hardcoded protocol/address assumptions in SMBus and PLL paths; they create hidden coupling and platform-specific failures.
2. Avoid making large cross-layer changes (driver + CLI + config) without an explicit validation matrix, because root-cause isolation becomes slow.
3. Avoid non-deterministic matching or fallback ordering in chip detection; ambiguity leads to misidentification and fragile behavior.

## How To Use This Reference

- Revisit this file before adding new chip support or changing SMBus transaction flow.
- Keep updates short and append new lessons after each hardware validation cycle.
