#pragma once
// Waveshare ESP32-S3-RGB-Matrix, SKU 34422, N32R16.
// Standard HUB75 64x32 1/16 scan, two panels chained horizontally.
// Green/blue permutation confirmed on the user's two panels with the autonomous test.
constexpr int R1_PIN=4, G1_PIN=6, B1_PIN=5, R2_PIN=7, G2_PIN=16, B2_PIN=15;
constexpr int A_PIN=18, B_PIN=8, C_PIN=3, D_PIN=42, E_PIN=-1;
constexpr int CLK_PIN=41, LAT_PIN=40, OE_PIN=2;
constexpr int SKR_RX_PIN=46, SKR_TX_PIN=45;
constexpr int AUDIO_BCLK_PIN=43, AUDIO_LRCLK_PIN=38, AUDIO_DATA_PIN=21;
constexpr int AUDIO_MCLK_PIN=12, AUDIO_ENABLE_PIN=11;
constexpr int AUDIO_SDA_PIN=47, AUDIO_SCL_PIN=48;
constexpr uint8_t DISPLAY_BRIGHTNESS=10;
constexpr bool CLOCK_PHASE=false;
constexpr auto PANEL_DRIVER=HUB75_I2S_CFG::SHIFTREG;
