#include <iostream>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "StepperMotor.h"

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024

constexpr gpio_num_t in11 = GPIO_NUM_23; // A
constexpr gpio_num_t in12 = GPIO_NUM_22;
constexpr gpio_num_t in21 = GPIO_NUM_19;
constexpr gpio_num_t in22 = GPIO_NUM_21;
constexpr gpio_num_t led = GPIO_NUM_2;


extern "C" void app_main() {
    // Configuración UART
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Configuración de los motores a pasos
    StepperMotor motor1({GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_21, GPIO_NUM_22});
    StepperMotor motor2({GPIO_NUM_23, GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27});

    char data[BUF_SIZE];
    printf("Statring program...\n");
    while (true) {
        // Leer datos desde UART
        int length = uart_read_bytes(UART_NUM, (uint8_t*)data, BUF_SIZE - 1, pdMS_TO_TICKS(1000));

        if (length > 0) {
            data[length] = '\0'; // Asegurar que el buffer es una cadena válida
            std::string input(data);

            try {
                // Parsear la entrada: formato esperado "motor1_steps motor2_steps"
                size_t space_pos = input.find(' ');
                if (space_pos == std::string::npos) {
                    std::cout << "Entrada inválida. Formato: motor1_steps motor2_steps\n";
                    continue;
                }

                int motor1_steps = std::stoi(input.substr(0, space_pos));
                int motor2_steps = std::stoi(input.substr(space_pos + 1));

                // Mover los motores
                motor1.moveSteps(motor1_steps, 10); // 10 ms entre pasos
                motor2.moveSteps(motor2_steps, 10); // 10 ms entre pasos

                std::cout << "Moviendo motor1: " << motor1_steps << " pasos, motor2: " << motor2_steps << " pasos\n";
            } catch (const std::exception& e) {
                std::cout << "Error al procesar la entrada: " << e.what() << "\n";
            }
        }
    vTaskDelay(pdMS_TO_TICKS(10));

    }
}
