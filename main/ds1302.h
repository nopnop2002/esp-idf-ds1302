/*
 * I ported from here.
 * https://github.com/Erriez/ErriezDS1302
 */


#ifndef MAIN_DS1302_H_
#define MAIN_DS1302_H_

//! DS1302 address/command register
#define DS1302_ACB              0x80    //!< Address command date/time
#define DS1302_ACB_RAM          0x40    //!< Address command RAM
#define DS1302_ACB_CLOCK        0x00    //!< Address command clock
#define DS1302_ACB_READ         0x01    //!< Address command read
#define DS1302_ACB_WRITE        0x00    //!< Address command write

//! DS1302 read clock register
#define DS1302_CMD_READ_CLOCK_REG(reg)  (DS1302_ACB | DS1302_ACB_CLOCK | (((reg) & 0x1F) << 1) | DS1302_ACB_READ)
//! DS1302 write clock register
#define DS1302_CMD_WRITE_CLOCK_REG(reg) (DS1302_ACB | DS1302_ACB_CLOCK | (((reg) & 0x1F) << 1) | DS1302_ACB_WRITE)
//! DS1302 read clock register with burst
#define DS1302_CMD_READ_CLOCK_BURST     (DS1302_ACB | DS1302_ACB_CLOCK | 0x3E | DS1302_ACB_READ)
//! DS1302 writeclock register with burst
#define DS1302_CMD_WRITE_CLOCK_BURST    (DS1302_ACB | DS1302_ACB_CLOCK | 0x3E | DS1302_ACB_WRITE)
//! DS1302 read RAM register
#define DS1302_CMD_READ_RAM(addr)       (DS1302_ACB | DS1302_ACB_RAM | (((addr) & 0x1F) << 1) | DS1302_ACB_READ)
//! DS1302 write RAM register
#define DS1302_CMD_WRITE_RAM(addr)      (DS1302_ACB | DS1302_ACB_RAM | (((addr) & 0x1F) << 1) | DS1302_ACB_WRITE)
//! DS1302 read RAM register with burst
#define DS1302_CMD_READ_RAM_BURST       (DS1302_ACB | DS1302_ACB_RAM | 0x3E | DS1302_ACB_READ)
//! DS1302 write RAM register with burst
#define DS1302_CMD_WRITE_RAM_BURST      (DS1302_ACB | DS1302_ACB_RAM | 0x3E | DS1302_ACB_WRITE)

//! DS1302 registers
#define DS1302_REG_SECONDS      0x00    //!< Seconds register
#define DS1302_REG_MINUTES      0x01    //!< Minutes register
#define DS1302_REG_HOURS        0x02    //!< Hours register
#define DS1302_REG_DAY_MONTH    0x03    //!< Day of the month register
#define DS1302_REG_MONTH        0x04    //!< Month register
#define DS1302_REG_DAY_WEEK     0x05    //!< Day of the week register
#define DS1302_REG_YEAR         0x06    //!< Year register
#define DS1302_REG_WP           0x07    //!< Write protect register
#define DS1302_REG_TC           0x08    //!< Tickle Charger register

//! DS1302 number of RAM registers
#define NUM_DS1302_RAM_REGS     31

//! DS1302 register bit defines
#define DS1302_BIT_CH           7       //!< Clock halt bit
#define DS1302_BIT_WP           7       //!< Write protect bit
#define DS1302_BIT_READ         0       //!< Bit read

#define DS1302_TCS_DISABLE      0x5C    //!< Tickle Charger disable value



/*!
 * \brief Date time structure
 */
typedef struct {
    uint8_t second;     //!< Second 0..59
    uint8_t minute;     //!< Minute 0..59
    uint8_t hour;       //!< Hour 0..23
    uint8_t dayWeek;    //!< Day of the week (0 as Sunday/ 1 as Monday)
    uint8_t dayMonth;   //!< Day of the month 1..31
    uint8_t month;      //!< Month 1..12
    uint16_t year;      //!< Year 2000..2099
} DS1302_DateTime;

typedef struct {
    uint8_t clkPin;     //!< GPIO for clk
    uint8_t ioPin;      //!< GPIO for io
    uint8_t cePin;      //!< GPIO for ce
} DS1302_Dev;

bool DS1302_begin(DS1302_Dev *dev, uint8_t clkPin, uint8_t ioPin, uint8_t cePin);
void DS1302_writeProtect(DS1302_Dev *dev, bool enable);
bool DS1302_isWriteProtected(DS1302_Dev *dev);
void DS1302_halt(DS1302_Dev *dev, bool halt);
bool DS1302_isHalted(DS1302_Dev *dev);
void DS1302_setDateTime(DS1302_Dev *dev, DS1302_DateTime *dateTime);
bool DS1302_getDateTime(DS1302_Dev *dev, DS1302_DateTime *dateTime);
void DS1302_setTime(DS1302_Dev *dev, uint8_t hour, uint8_t minute, uint8_t second);
bool DS1302_getTime(DS1302_Dev *dev, uint8_t *hour, uint8_t *minute, uint8_t *second);

void DS1302_writeClockRegister(DS1302_Dev *dev, uint8_t reg, uint8_t value);
uint8_t DS1302_readClockRegister(DS1302_Dev *dev, uint8_t reg);

void DS1302_writeByteRAM(DS1302_Dev *dev, uint8_t addr, uint8_t value);
void DS1302_writeBufferRAM(DS1302_Dev *dev, uint8_t *buf, uint8_t len);

uint8_t DS1302_readByteRAM(DS1302_Dev *dev, uint8_t addr);
void DS1302_readBufferRAM(DS1302_Dev *dev, uint8_t *buf, uint8_t len);

// RTC interface functions
void DS1302_transferBegin(DS1302_Dev *dev);
void DS1302_transferEnd(DS1302_Dev *dev);
void DS1302_writeAddrCmd(DS1302_Dev *dev, uint8_t value);
void DS1302_writeByte(DS1302_Dev *dev, uint8_t value);
uint8_t DS1302_readByte(DS1302_Dev *dev);
void DS1302_readBuffer(DS1302_Dev *dev, void *buf, uint8_t len);

// BCD conversions
uint8_t bcdToDec(uint8_t bcd);
uint8_t decToBcd(uint8_t dec);

#endif // MAIN_DS1302_H_

