#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_rom_sys.h> // Essencial no IDF v5 para o esp_rom_delay_us
#include <esp_log.h>
#include "rg_system.h"
#include "config.h"

// Definições do Barramento e Chips
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  400000
#define AXP2101_ADDR        0x34
#define ES8311_ADDR         0x18

static const char *TAG = "WaveshareTarget";

// ---------------------------------------------------------
// 1. HACK DE HARDWARE: Destrava o I2C após o Soft Reset
// ---------------------------------------------------------
static void clear_i2c_bus(void) {
    ESP_LOGI(TAG, "Forcando Bus Recovery do I2C...");
    gpio_config_t io_conf = {0};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = (1ULL << RG_GPIO_I2C_SCL) | (1ULL << RG_GPIO_I2C_SDA);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level((gpio_num_t)RG_GPIO_I2C_SDA, 1); esp_rom_delay_us(100);
    for (int i = 0; i < 9; i++) {
        gpio_set_level((gpio_num_t)RG_GPIO_I2C_SCL, 0); esp_rom_delay_us(100);
        gpio_set_level((gpio_num_t)RG_GPIO_I2C_SCL, 1); esp_rom_delay_us(100);
    }
    gpio_set_level((gpio_num_t)RG_GPIO_I2C_SDA, 0); esp_rom_delay_us(100);
    gpio_set_level((gpio_num_t)RG_GPIO_I2C_SCL, 1); esp_rom_delay_us(100);
    gpio_set_level((gpio_num_t)RG_GPIO_I2C_SDA, 1); esp_rom_delay_us(100);

    gpio_reset_pin((gpio_num_t)RG_GPIO_I2C_SCL);
    gpio_reset_pin((gpio_num_t)RG_GPIO_I2C_SDA);
}

// ---------------------------------------------------------
// 2. INICIALIZA O DRIVER I2C NATIVO DO ESP-IDF
// ---------------------------------------------------------
static void i2c_bus_init(void) {
    i2c_config_t conf = {0};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = RG_GPIO_I2C_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = RG_GPIO_I2C_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    // O IDF v5 exige o clk_flags se não for 0, mas aqui 0 está OK para default.
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    ESP_LOGI(TAG, "Driver I2C instalado nos pinos SDA:%d SCL:%d", RG_GPIO_I2C_SDA, RG_GPIO_I2C_SCL);
}

// Função auxiliar para escrever nos registradores
static void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t val) {
    uint8_t data[2] = {reg, val};
    i2c_master_write_to_device(I2C_MASTER_NUM, addr, data, 2, pdMS_TO_TICKS(100));
}

// ---------------------------------------------------------
// 3. A FUNÇÃO MESTRA CHAMADA PELO RETRO-GO NO BOOT
// ---------------------------------------------------------
void rg_target_init(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, " INICIANDO HARDWARE WAVESHARE AMOLED    ");
    ESP_LOGI(TAG, "========================================");

    // Passo A: Prepara o barramento
    clear_i2c_bus();
    i2c_bus_init();

    // Passo B: Acorda o Gerenciador de Energia (AXP2101)
    ESP_LOGI(TAG, "Enviando pulsos de energia via PMU...");
    i2c_write_reg(AXP2101_ADDR, 0x10, 0x01); 
    
    // Passo C: Prepara o chip de Áudio (ES8311)
    ESP_LOGI(TAG, "Acordando Codec de Audio ES8311...");
    i2c_write_reg(ES8311_ADDR, 0x00, 0x00); 

    ESP_LOGI(TAG, "Target Init finalizado. Repassando o controle pro emulador...");
}