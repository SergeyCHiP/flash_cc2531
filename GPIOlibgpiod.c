#include "GPIOlibgpiod.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
// Заглушки для macOS

// Глобальный контекст GPIO
static gpio_context_t gpio_ctx = {0};

// Получение контекста GPIO
gpio_context_t* gpio_get_context(void) {
    return &gpio_ctx;
}

// Проверка инициализации
bool gpio_is_initialized(void) {
    return gpio_ctx.initialized;
}

// Инициализация GPIO (заглушка)
int gpio_init(int pinRST, int pinDC, int pinDD) {
    printf("GPIO initialization (stub for macOS)\n");
    printf("Pins: RST=%d, DC=%d, DD=%d\n", pinRST, pinDC, pinDD);
    
    gpio_ctx.pin_numbers[0] = pinRST;
    gpio_ctx.pin_numbers[1] = pinDC;
    gpio_ctx.pin_numbers[2] = pinDD;
    gpio_ctx.initialized = true;
    
    return 0;
}

// Настройка направления пина (заглушка)
void gpio_set_mode(int pin, int mode) {
    printf("GPIO set mode: pin=%d, mode=%s\n", pin, mode == GPIO_OUTPUT ? "OUTPUT" : "INPUT");
}

// Запись значения в пин (заглушка)
void gpio_write(int pin, int value) {
    printf("GPIO write: pin=%d, value=%s\n", pin, value == GPIO_HIGH ? "HIGH" : "LOW");
}

// Чтение значения с пина (заглушка)
int gpio_read(int pin) {
    printf("GPIO read: pin=%d (returning LOW)\n", pin);
    return GPIO_LOW;
}

// Очистка ресурсов (заглушка)
void gpio_cleanup(void) {
    printf("GPIO cleanup (stub for macOS)\n");
    gpio_ctx.initialized = false;
}

#else
// Реальная реализация для Linux с libgpiod
#include <errno.h>

// Глобальный контекст GPIO
static gpio_context_t gpio_ctx = {0};

// Получение контекста GPIO
gpio_context_t* gpio_get_context(void) {
    return &gpio_ctx;
}

// Проверка инициализации
bool gpio_is_initialized(void) {
    return gpio_ctx.initialized;
}

// Инициализация GPIO
int gpio_init(int pinRST, int pinDC, int pinDD) {
    // Очистка предыдущего состояния
    if (gpio_ctx.initialized) {
        gpio_cleanup();
    }
    
    // Сохранение номеров пинов
    gpio_ctx.pin_numbers[0] = pinRST;
    gpio_ctx.pin_numbers[1] = pinDC;
    gpio_ctx.pin_numbers[2] = pinDD;
    
    // Открытие GPIO чипа (обычно gpiochip0 на Raspberry Pi)
    gpio_ctx.chip = gpiod_chip_open("/dev/gpiochip0");
    if (!gpio_ctx.chip) {
        fprintf(stderr, "Failed to open GPIO chip: %s\n", strerror(errno));
        return -1;
    }
    
    // Получение линий GPIO
    gpio_ctx.lines[0] = gpiod_chip_get_line(gpio_ctx.chip, pinRST);
    gpio_ctx.lines[1] = gpiod_chip_get_line(gpio_ctx.chip, pinDC);
    gpio_ctx.lines[2] = gpiod_chip_get_line(gpio_ctx.chip, pinDD);
    
    if (!gpio_ctx.lines[0] || !gpio_ctx.lines[1] || !gpio_ctx.lines[2]) {
        fprintf(stderr, "Failed to get GPIO lines\n");
        gpio_cleanup();
        return -1;
    }
    
    // Установка начального состояния - все пины как выходы с низким уровнем
    for (int i = 0; i < 3; i++) {
        if (gpiod_line_request_output(gpio_ctx.lines[i], "flash_cc2531", 0) < 0) {
            fprintf(stderr, "Failed to configure GPIO line %d as output: %s\n", 
                    gpio_ctx.pin_numbers[i], strerror(errno));
            gpio_cleanup();
            return -1;
        }
    }
    
    gpio_ctx.initialized = true;
    return 0;
}

// Настройка направления пина
void gpio_set_mode(int pin, int mode) {
    if (!gpio_ctx.initialized) return;
    
    // Определяем индекс пина
    int pin_index = -1;
    for (int i = 0; i < 3; i++) {
        if (gpio_ctx.pin_numbers[i] == pin) {
            pin_index = i;
            break;
        }
    }
    
    if (pin_index == -1) {
        fprintf(stderr, "Unknown pin: %d\n", pin);
        return;
    }
    
    // Освобождаем линию перед перенастройкой
    gpiod_line_release(gpio_ctx.lines[pin_index]);
    
    if (mode == GPIO_OUTPUT) {
        if (gpiod_line_request_output(gpio_ctx.lines[pin_index], "flash_cc2531", 0) < 0) {
            fprintf(stderr, "Failed to set pin %d as output: %s\n", pin, strerror(errno));
        }
    } else {
        if (gpiod_line_request_input(gpio_ctx.lines[pin_index], "flash_cc2531") < 0) {
            fprintf(stderr, "Failed to set pin %d as input: %s\n", pin, strerror(errno));
        }
    }
}

// Запись значения в пин
void gpio_write(int pin, int value) {
    if (!gpio_ctx.initialized) return;
    
    // Определяем индекс пина
    int pin_index = -1;
    for (int i = 0; i < 3; i++) {
        if (gpio_ctx.pin_numbers[i] == pin) {
            pin_index = i;
            break;
        }
    }
    
    if (pin_index == -1) {
        fprintf(stderr, "Unknown pin: %d\n", pin);
        return;
    }
    
    if (gpiod_line_set_value(gpio_ctx.lines[pin_index], value) < 0) {
        fprintf(stderr, "Failed to write to pin %d: %s\n", pin, strerror(errno));
    }
}

// Чтение значения с пина
int gpio_read(int pin) {
    if (!gpio_ctx.initialized) return -1;
    
    // Определяем индекс пина
    int pin_index = -1;
    for (int i = 0; i < 3; i++) {
        if (gpio_ctx.pin_numbers[i] == pin) {
            pin_index = i;
            break;
        }
    }
    
    if (pin_index == -1) {
        fprintf(stderr, "Unknown pin: %d\n", pin);
        return -1;
    }
    
    int value;
    if (gpiod_line_get_value(gpio_ctx.lines[pin_index], &value) < 0) {
        fprintf(stderr, "Failed to read from pin %d: %s\n", pin, strerror(errno));
        return -1;
    }
    
    return value;
}

// Очистка ресурсов
void gpio_cleanup(void) {
    if (gpio_ctx.initialized) {
        // Освобождение линий
        for (int i = 0; i < 3; i++) {
            if (gpio_ctx.lines[i]) {
                gpiod_line_release(gpio_ctx.lines[i]);
                gpio_ctx.lines[i] = NULL;
            }
        }
        
        // Закрытие чипа
        if (gpio_ctx.chip) {
            gpiod_chip_close(gpio_ctx.chip);
            gpio_ctx.chip = NULL;
        }
        
        gpio_ctx.initialized = false;
    }
}

#endif // __APPLE__ 