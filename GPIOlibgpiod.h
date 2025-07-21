#ifndef GPIO_LIBGPIOD_H
#define GPIO_LIBGPIOD_H

#ifdef __APPLE__
// Заглушка для macOS - libgpiod недоступен
#include <stdint.h>
#include <stdbool.h>

// GPIO направления
#define GPIO_INPUT  0
#define GPIO_OUTPUT 1

// GPIO состояния
#define GPIO_LOW  0
#define GPIO_HIGH 1

// Структура для хранения GPIO линий (заглушка)
typedef struct {
    void *chip;
    void *lines[3]; // RST, DC, DD
    int pin_numbers[3];
    bool initialized;
} gpio_context_t;

// Инициализация GPIO (заглушка)
int gpio_init(int pinRST, int pinDC, int pinDD);

// Настройка направления пина (заглушка)
void gpio_set_mode(int pin, int mode);

// Запись значения в пин (заглушка)
void gpio_write(int pin, int value);

// Чтение значения с пина (заглушка)
int gpio_read(int pin);

// Очистка ресурсов (заглушка)
void gpio_cleanup(void);

// Проверка инициализации (заглушка)
bool gpio_is_initialized(void);

// Получение контекста GPIO (заглушка)
gpio_context_t* gpio_get_context(void);

#else
// Реальная реализация для Linux с libgpiod
#include <gpiod.h>
#include <stdint.h>
#include <stdbool.h>

// GPIO направления
#define GPIO_INPUT  0
#define GPIO_OUTPUT 1

// GPIO состояния
#define GPIO_LOW  0
#define GPIO_HIGH 1

// Структура для хранения GPIO линий
typedef struct {
    struct gpiod_chip *chip;
    struct gpiod_line *lines[3]; // RST, DC, DD
    int pin_numbers[3];
    bool initialized;
} gpio_context_t;

// Инициализация GPIO
int gpio_init(int pinRST, int pinDC, int pinDD);

// Настройка направления пина
void gpio_set_mode(int pin, int mode);

// Запись значения в пин
void gpio_write(int pin, int value);

// Чтение значения с пина
int gpio_read(int pin);

// Очистка ресурсов
void gpio_cleanup(void);

// Проверка инициализации
bool gpio_is_initialized(void);

// Получение контекста GPIO
gpio_context_t* gpio_get_context(void);

#endif // __APPLE__

#endif // GPIO_LIBGPIOD_H 