#ifndef RG_TARGET_WAVESHARE_WATCH
#define RG_TARGET_WAVESHARE_WATCH 1
#endif

#pragma once

// ============================================================================
// WAVESHARE ESP32-S3 TOUCH AMOLED 2.06 - HARDWARE ABSTRACTION LAYER
// ============================================================================

#define RG_TARGET_NAME             "WAVESHARE-WATCH"

// ---------------------------------------------------------
// 1. STORAGE (CARTÃO SD VIA SPI)
// ---------------------------------------------------------
#define RG_STORAGE_ROOT             "/sdcard"
#define RG_STORAGE_SDSPI_HOST       SPI2_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT

#define RG_GPIO_SDSPI_MISO          GPIO_NUM_13
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_11
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_12
#define RG_GPIO_SDSPI_CS            GPIO_NUM_10

// ---------------------------------------------------------
// 2. I2C BUS (FUNDAMENTAL PARA TOUCH E AXP2101)
// ---------------------------------------------------------
#define RG_GPIO_I2C_SDA             GPIO_NUM_15
#define RG_GPIO_I2C_SCL             GPIO_NUM_14

// ---------------------------------------------------------
// 3. AUDIO (ES8311 I2S CODEC)
// ---------------------------------------------------------
#define RG_AUDIO_USE_INT_DAC        0   // ESP32-S3 não possui DAC interno
#define RG_AUDIO_USE_EXT_DAC        1   // Ativa saída I2S externa

#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_42
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_41
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_2  // DOUT (Din do ES8311)
#define RG_GPIO_SND_I2S_MCLK        GPIO_NUM_40 // Master Clock (Requisito do ES8311)
// O Codec não usa pino AMP_ENABLE direto, o power é via I2C pelo AXP2101
#define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_NC 

// ---------------------------------------------------------
// 4. BATERIA E ENERGIA (Lido via AXP2101 depois, marcamos dummy aqui)
// ---------------------------------------------------------
#define RG_BATTERY_DRIVER           0 // Vamos injetar a leitura do I2C no target.c

// ---------------------------------------------------------
// 5. INPUT (TOUCH VIRTUAL)
// ---------------------------------------------------------
// Deixamos os mapas vazios. O Retro-Go aceitará as flags que 
// injetaremos interceptando a leitura do CST816S no target.c
#define RG_GAMEPAD_MAP {}
#define RG_GAMEPAD_VIRT_MAP {\
    {RG_KEY_MENU,   .src = RG_KEY_START | RG_KEY_SELECT},\
    {RG_KEY_OPTION, .src = RG_KEY_SELECT | RG_KEY_A    },\
}

// ---------------------------------------------------------
// 6. DISPLAY (AMOLED SH8601 VIA QSPI/SPI)
// ---------------------------------------------------------
#define RG_SCREEN_DRIVER            1   // 1 = Driver Customizado/Genérico
#define RG_SCREEN_HOST              SPI2_HOST // Pode conflitar com o SD? Avaliaremos.
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_BACKLIGHT         1

#define RG_SCREEN_WIDTH             410
#define RG_SCREEN_HEIGHT            502
#define RG_SCREEN_ROTATE            0

// A JANELA DE MEMÓRIA (VIEWPORT)
// O emulador de NES/GB usará apenas a parte superior da tela.
// Definiremos a área segura nos eixos (Deixando o espaço inferior livre pro controle)
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 410, 288} 
#define RG_SCREEN_SAFE_AREA         {0, 0, 410, 288}

#define RG_GPIO_LCD_MOSI            GPIO_NUM_7
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_CLK             GPIO_NUM_6
#define RG_GPIO_LCD_CS              GPIO_NUM_5
#define RG_GPIO_LCD_DC              GPIO_NUM_4
#define RG_GPIO_LCD_RST             GPIO_NUM_8
#define RG_GPIO_LCD_BCKL            GPIO_NUM_9

// A INIT SEQUENCE DO AMOLED
// Esta é a macro que o Retro-Go usará para acordar sua tela
#define ILI9341_CMD(cmd, ...) { \
    const uint8_t data[] = {__VA_ARGS__}; \
    rg_display_write(cmd, data, sizeof(data)); \
}

#define RG_SCREEN_INIT() \
    ILI9341_CMD(0x11, 0x00); \
    vTaskDelay(pdMS_TO_TICKS(120)); \
    ILI9341_CMD(0x44, 0x01, 0xD1); \
    ILI9341_CMD(0x35, 0x00); \
    ILI9341_CMD(0x53, 0x20); \
    ILI9341_CMD(0x2A, 0x00, 0x00, 0x01, 0x99); \
    ILI9341_CMD(0x2B, 0x00, 0x00, 0x01, 0xF5); \
    ILI9341_CMD(0x51, 0x00); \
    ILI9341_CMD(0x29, 0x00); \
    ILI9341_CMD(0x51, 0xFF); 

// ---------------------------------------------------------
// 7. MULTI-BOOT (UPDATER)
// ---------------------------------------------------------
// Desligamos o Updater nativo do Retro-Go, pois o nosso App Store
// gerencia as atualizações de forma muito mais segura.
#define RG_UPDATER_ENABLE           0