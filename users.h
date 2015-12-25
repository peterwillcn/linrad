
// Give each hardware unit its own unique number.
// With few units, make these numbers powers of two to make
// address decoding unnecessary.
#define RX10700 16
#define RX70 32
#define RX144 8
#define RXHFA 2
#define RXHFA_GAIN 1
#define TX10700 4
#define TX70 64
// These two tx units may not be used simultaneously
// with this definition:
#define TX144 128
#define TXHFA 128


// These parameters define the frequency control window.
#define FREQ_MHZ_DECIMALS 3
#define FREQ_MHZ_DIGITS 4
#define FREQ_MHZ_ROUNDCORR (0.5*pow(10,-FREQ_MHZ_DECIMALS))
#define FG_HSIZ ((FREQ_MHZ_DECIMALS+FREQ_MHZ_DIGITS+6)*text_width)
#define FG_VSIZ (3*text_height)
