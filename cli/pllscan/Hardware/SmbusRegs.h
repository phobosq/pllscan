#pragma once

//
// SMBus Host Controller register offsets
//

#define SMBHSTSTS      0x00
#define SMBHSTCNT      0x02
#define SMBHSTCMD      0x03
#define SMBHSTADD      0x04
#define SMBHSTDAT0     0x05
#define SMBHSTDAT1     0x06

//
// SMBHSTSTS bits
//

#define SMB_STS_HOST_BUSY   0x01
#define SMB_STS_INTR        0x02
#define SMB_STS_DEV_ERR     0x04
#define SMB_STS_BUS_ERR     0x08
#define SMB_STS_FAILED      0x10
#define SMB_STS_SMBALERT    0x20
#define SMB_STS_INUSE       0x40
#define SMB_STS_BYTE_DONE   0x80

/* Only these three bits signal a real transaction error */
#define SMB_ERR_MASK  (SMB_STS_DEV_ERR | SMB_STS_BUS_ERR | SMB_STS_FAILED)

//
// SMBHSTCNT values
//

#define SMB_CMD_QUICK       0x40
#define SMB_CMD_BYTE        0x44
#define SMB_CMD_BYTE_DATA   0x48
#define SMB_CMD_WORD_DATA   0x4C