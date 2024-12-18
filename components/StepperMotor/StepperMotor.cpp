#include "StepperMotor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <vector>
#include <cmath>

StepperMotor::StepperMotor(const std::vector<gpio_num_t>& control_pins)
    : controlPins(control_pins), motorTaskHandle(nullptr), stopFlag(false) {
    // Configure each pin as output
    for (auto pin : controlPins) {
        gpio_reset_pin(pin);
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        gpio_set_level(pin, 0);
    }
    printf("Creando motor\n");
}

StepperMotor::~StepperMotor() {
    stop();
}

void StepperMotor::moveSteps(int steps, int delay_ms) {
    stop(); // Ensure no previous task is running

    struct TaskParams {
        StepperMotor* motor;
        int steps;
        int delay_ms;
    };

    auto params = new TaskParams{this, steps, delay_ms};

    xTaskCreate(&StepperMotor::motorTask, "StepperMotorTask", 2048, params, 1, &motorTaskHandle);
}

void StepperMotor::stop() {
    if (motorTaskHandle != nullptr) {
        stopFlag = true;
        // Espera hasta que la tarea termine (máximo 1 segundo)
        for (int i = 0; i < 100; ++i) {
            if (motorTaskHandle == nullptr) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        // Eliminar la tarea si no se auto-eliminó
        if (motorTaskHandle != nullptr) {
            vTaskDelete(motorTaskHandle);
            motorTaskHandle = nullptr;
        }
    }
}


void StepperMotor::motorTask(void* arg) {
    auto params = static_cast<TaskParams*>(arg);
    StepperMotor* motor = params->motor;
    int steps = params->steps;
    int delay_ms = params->delay_ms;
    delete params;

    int step_index = 0;
    int direction = (steps > 0) ? 1 : -1;
    int remaining_steps = abs(steps);

    while (!motor->stopFlag && remaining_steps > 0) {
        motor->step(step_index);
        step_index = (step_index + direction + 4) % 4; // Ciclo entre 0-3
        remaining_steps--;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
      printf("%u %u\n", remaining_steps, !motor->stopFlag && remaining_steps > 0);    
    }
    printf("Done\n");
    motor->stopFlag = false; // Reiniciar la bandera
    motor->motorTaskHandle = nullptr; // Limpia el handle antes de autoeliminar
    vTaskDelete(nullptr); // Elimina la tarea actual
}


void StepperMotor::step(int step_index) {
    static const int step_sequence[4][4] = {
        {1, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 1, 1, 0},
        {1, 0, 1, 0},
    };

    for (size_t i = 0; i < controlPins.size(); ++i) {
        gpio_set_level(controlPins[i], step_sequence[step_index][i]);
    }
}
