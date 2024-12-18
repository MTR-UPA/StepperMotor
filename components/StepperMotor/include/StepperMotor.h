#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <vector>

class StepperMotor {
public:
    StepperMotor(const std::vector<gpio_num_t>& control_pins);
    ~StepperMotor();

    void moveSteps(int steps, int delay_ms);
    void stop();

private:
    std::vector<gpio_num_t> controlPins;
    TaskHandle_t motorTaskHandle;
    volatile bool stopFlag;

    // Mover TaskParams a un ámbito de clase
    struct TaskParams {
        StepperMotor* motor;
        int steps;
        int delay_ms;
    };

    static void motorTask(void* arg);
    void step(int step_index);
};

#endif // STEPPER_MOTOR_H
