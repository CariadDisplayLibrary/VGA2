#include <VGA2.h>


#define VSYNC_OFF vsync_port->lat.set = vsync_pin;
#define VSYNC_ON  vsync_port->lat.clr = vsync_pin;

#define HSYNC_OFF hsync_port->lat.set = hsync_pin;
#define HSYNC_ON  hsync_port->lat.clr = hsync_pin;

VGA2 *VGA2::_unit;

p32_ioport *hsync_port;
p32_ioport *vsync_port;
uint32_t hsync_pin;
uint32_t vsync_pin;

static const uint8_t blank[100] = {0}; // A blank line

static volatile uint8_t *vgaBufferR;
static volatile uint8_t *vgaBufferG;
static volatile uint8_t *vgaBufferB;
static volatile uint8_t pulsePhase = 0;

static volatile uint32_t scanLine = 0;

#define PULSEPOS 2340
#define PULSEWIDTH 10

void __USER_ISR horizPulse() {
    IFS0bits.T5IF = 0;
    HSYNC_ON
    T5CONbits.ON = 0;
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    HSYNC_OFF;
}

void __USER_ISR vgaProcess() {


    static uint32_t _ramPos = 0;

    DCH0INTbits.CHSDIF = 0;
    IFS1bits.DMA0IF = 0;

    pulsePhase = 0;
    TMR5 = 0;
    PR5 = PULSEPOS;
    T5CONbits.ON=1;


    if (scanLine == 0) _ramPos = 0;
    DCH0SSA = (scanLine < VGA2::vgaVDP) ? (((uint32_t)&vgaBufferR[_ramPos]) & 0x1FFFFFFF) : (((uint32_t)&blank[0]) & 0x1FFFFFFF);
    DCH1SSA = (scanLine < VGA2::vgaVDP) ? (((uint32_t)&vgaBufferG[_ramPos]) & 0x1FFFFFFF) : (((uint32_t)&blank[0]) & 0x1FFFFFFF);
    DCH2SSA = (scanLine < VGA2::vgaVDP) ? (((uint32_t)&vgaBufferB[_ramPos]) & 0x1FFFFFFF) : (((uint32_t)&blank[0]) & 0x1FFFFFFF);

    DCH0SSIZ = VGA2::vgaHTOT >> 3;
    DCH1SSIZ = VGA2::vgaHTOT >> 3;
    DCH2SSIZ = VGA2::vgaHTOT >> 3;
    DCH0ECONbits.CFORCE = 1;
    DCH1ECONbits.CFORCE = 1;
    DCH2ECONbits.CFORCE = 1;
    DCH0CONbits.CHEN = 1;
    DCH1CONbits.CHEN = 1;
    DCH2CONbits.CHEN = 1;

    if (scanLine & 1) _ramPos += VGA2::vgaHTOT >> 3;
    scanLine++;

    if (scanLine == 490) {
        VSYNC_ON
    } else if (scanLine == 492) {
        VSYNC_OFF
    } else if (scanLine == 525) {
        scanLine = 0;
    }
}

uint32_t VGA2::millis() {
    return millis();
}

void VGA2::initializeDevice() {
    if (_hsync_pin == 0 || _vsync_pin == 0) {
        return;
    }

    memset((void *)_bufferR0, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    memset((void *)_bufferG0, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    memset((void *)_bufferB0, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    memset((void *)_bufferR1, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    memset((void *)_bufferG1, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    memset((void *)_bufferB1, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    vgaBufferR = _bufferR0;
    vgaBufferG = _bufferG0;
    vgaBufferB = _bufferB0;
    _activeBufferR = _bufferR1;
    _activeBufferG = _bufferG1;
    _activeBufferB = _bufferB1;

    _hsync_port->tris.clr = _hsync_pin;
    _vsync_port->tris.clr = _vsync_pin;

    hsync_port = _hsync_port;
    vsync_port = _vsync_port;
    hsync_pin = _hsync_pin;
    vsync_pin = _vsync_pin;

    // Now congfigure SPI4 for display data transmission.
    SPI4CON = 0;
    SPI4CONbits.MSTEN = 1;
    SPI4CONbits.STXISEL = 0; //0b11;
    SPI4BRG = 2;
    SPI4CONbits.ON = 1;

    SPI2CON = 0;
    SPI2CONbits.MSTEN = 1;
    SPI2CONbits.STXISEL = 0; //0b11;
    SPI2BRG = 2;
    SPI2CONbits.ON = 1;

    SPI3CON = 0;
    SPI3CONbits.MSTEN = 1;
    SPI3CONbits.STXISEL = 0; //0b11;
    SPI3BRG = 2;
    SPI3CONbits.ON = 1;

    // And now a DMA channel for transferring the data
    DCH0DSA = ((unsigned int)&SPI2BUF) & 0x1FFFFFFF;
    DCH0DSIZ = 1;
    DCH0CSIZ = 1;
    DCH0ECONbits.SIRQEN = 1;
    DCH0ECONbits.CHSIRQ = _SPI2_TX_IRQ;
    DCH0CONbits.CHAEN = 0;
    DCH0CONbits.CHEN = 0;
    DCH0CONbits.CHPRI = 3;
    DCH0INTbits.CHSDIF = 0;
    DCH0INTbits.CHSDIE = 1;

    DCH1DSA = ((unsigned int)&SPI3BUF) & 0x1FFFFFFF;
    DCH1DSIZ = 1;
    DCH1CSIZ = 1;
    DCH1ECONbits.SIRQEN = 1;
    DCH1ECONbits.CHSIRQ = _SPI3_TX_IRQ;
    DCH1CONbits.CHAEN = 0;
    DCH1CONbits.CHEN = 0;
    DCH1CONbits.CHPRI = 3;
    DCH1INTbits.CHSDIF = 0;
    DCH1INTbits.CHSDIE = 0;

    DCH2DSA = ((unsigned int)&SPI4BUF) & 0x1FFFFFFF;
    DCH2DSIZ = 1;
    DCH2CSIZ = 1;
    DCH2ECONbits.SIRQEN = 1;
    DCH2ECONbits.CHSIRQ = _SPI4_TX_IRQ;
    DCH2CONbits.CHAEN = 0;
    DCH2CONbits.CHEN = 0;
    DCH2CONbits.CHPRI = 3;
    DCH2INTbits.CHSDIF = 0;
    DCH2INTbits.CHSDIE = 0;

    setIntVector(_DMA_0_VECTOR, vgaProcess);
    setIntPriority(_DMA_0_VECTOR, 6, 0);
    clearIntFlag(_DMA0_IRQ);
    setIntEnable(_DMA0_IRQ);

    VSYNC_OFF
    HSYNC_OFF

    DMACONbits.ON = 1;

    DCH0SSA = ((uint32_t)&blank[0]) & 0x1FFFFFFF;
    DCH0SSIZ = vgaHBP;
    DCH1SSA = ((uint32_t)&blank[0]) & 0x1FFFFFFF;
    DCH1SSIZ = vgaHBP;
    DCH2SSA = ((uint32_t)&blank[0]) & 0x1FFFFFFF;
    DCH2SSIZ = vgaHBP;

    DCH0ECONbits.CFORCE = 1;
    DCH1ECONbits.CFORCE = 1;
    DCH2ECONbits.CFORCE = 1;
    DCH0CONbits.CHEN = 1;
    DCH1CONbits.CHEN = 1;
    DCH2CONbits.CHEN = 1;

    T5CON = 0;
    T5CONbits.TCKPS = 0;
    PR5=0xFFFF;
    setIntVector(_TIMER_5_VECTOR, horizPulse);
    setIntPriority(_TIMER_5_VECTOR, 6, 0);
    clearIntFlag(_TIMER_5_IRQ);
    setIntEnable(_TIMER_5_IRQ);

    // Unfortunately the core timer really plays havoc with the VGA timing.
    // So we have to disable it. That means no millis() and no delay().
    // Makes things a little more interesting...
    clearIntEnable(_CORE_TIMER_IRQ);
}

VGA2::VGA2(uint8_t hsync, uint8_t vsync) {
    _unit = this;
    uint32_t port = 0;

    scanLine = 0;

    _hsync_pin = 0;
    _vsync_pin = 0;

    if (hsync >= NUM_DIGITAL_PINS_EXTENDED) { return; }
    if (vsync >= NUM_DIGITAL_PINS_EXTENDED) { return; }

    port = digitalPinToPort(hsync);
    if (port == NOT_A_PIN) { return; }
    _hsync_port = (p32_ioport *)portRegisters(port);
    _hsync_pin = digitalPinToBitMask(hsync);

    port = digitalPinToPort(vsync);
    if (port == NOT_A_PIN) { return; }
    _vsync_port = (p32_ioport *)portRegisters(port);
    _vsync_pin = digitalPinToBitMask(vsync);
}

void VGA2::setPixel(int x, int y, color_t c) {
    if (x < 0 || y < 0 || x >= Width || y >= Height) {
        return;
    }
    uint32_t poff = ((x-4) >> 3) + y * (vgaHTOT >> 3);
    uint8_t ppos = (x-4) & 0x07;
    poff += 1;
    if (c & 0b0000000000000001) {
        _activeBufferB[poff] |= (0x80>>ppos);
    } else {
        _activeBufferB[poff] &= ~(0x80>>ppos);
    }


    poff = ((x-2) >> 3) + y * (vgaHTOT >> 3);
    ppos = (x-2) & 0x07;
    poff += 1;
    if (c & 0b0000000000100000) {
        _activeBufferG[poff] |= (0x80>>ppos);
    } else {
        _activeBufferG[poff] &= ~(0x80>>ppos);
    }


    poff = ((x) >> 3) + y * (vgaHTOT/8);
    ppos = (x) & 0x07;
    poff += 1;
    if (c & 0b0000100000000000) {
        _activeBufferR[poff] |= (0x80>>ppos);
    } else {
        _activeBufferR[poff] &= ~(0x80>>ppos);
    }
}

void VGA2::vblank() {
    while (scanLine != 480);
}

void VGA2::flip() {
    vblank();
    if (_activeBufferR == _bufferR0) {
        _activeBufferR = _bufferR1;
        _activeBufferG = _bufferG1;
        _activeBufferB = _bufferB1;
        vgaBufferR = _bufferR0;
        vgaBufferG = _bufferG0;
        vgaBufferB = _bufferB0;
    } else {
        _activeBufferR = _bufferR0;
        _activeBufferG = _bufferG0;
        _activeBufferB = _bufferB0;
        vgaBufferR = _bufferR1;
        vgaBufferG = _bufferG1;
        vgaBufferB = _bufferB1;
    }
}

void VGA2::fillScreen(color_t c) {
    if (c & 0b0000000000000001) {
        for (int i = 0; i < (vgaVDP >> 1); i++) {
            memset((void *)&_activeBufferB[(vgaHTOT >> 3) * i + 1], 255, vgaHDP>>3);
        }
    } else {
        memset((void *)_activeBufferB, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    }
    if (c & 0b0000000000100000) {
        for (int i = 0; i < (vgaVDP >> 1); i++) {
            memset((void *)&_activeBufferG[(vgaHTOT >> 3) * i + 1], 255, vgaHDP>>3);
        }
    } else {
        memset((void *)_activeBufferG, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    }
    if (c & 0b0000100000000000) {
        for (int i = 0; i < (vgaVDP >> 1); i++) {
            memset((void *)&_activeBufferR[(vgaHTOT >> 3) * i + 1], 255, vgaHDP>>3);
        }
    } else {
        memset((void *)_activeBufferR, 0, (vgaHTOT >> 3) * (vgaVDP >> 1));
    }
}

