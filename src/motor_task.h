#pragma once
/**
    motor_task.h - A class that contains all functions to control a bipolar stepper motor.
    Author: Jason Chen, 2022

    To control the stepper motor, send PWM signals to the stepper motor driver, TMC2209. To operate
    the driver: (1) start the driver by taking it out of STANDBY), and (2) ENABLE the motor coils.
    Both are done automatically.
**/
#include <Arduino.h>
#include <HardwareSerial.h>       // Hardwareserial for uart
#include <TMCStepper.h>
#include <FastAccelStepper.h>
#include <AS5600.h>
#include <Preferences.h>
#include <FunctionalInterrupt.h>  // std:bind()
#include "task.h"
#include "logger.h"
#include "commands.h"


class MotorTask : public Task<MotorTask> {
    friend class Task<MotorTask>;

public:
    MotorTask(const uint8_t task_core);
    ~MotorTask();
    void addWirelessTaskQueue(QueueHandle_t queue);
    void addSystemSleepTimer(xTimerHandle timer);

protected:
    void run();

private:
    // TMCStepper library for interfacing with the stepper motor driver hardware, to read/write
    // registers for setting speed, acceleration, current, etc.
    TMC2209Stepper driver_ = TMC2209Stepper(&Serial1, R_SENSE, DRIVER_ADDR);

    // FastAccelStepper library for generating PWM signal to the stepper driver to move/accelerate
    // and stop/deccelerate the stepper motor.
    FastAccelStepperEngine engine_ = FastAccelStepperEngine();
    FastAccelStepper *motor_ = NULL;

    // Rotary encoder for keeping track of the actual motor position because motor could slip and
    // cause the position to be incorrect, i.e. closed-loop system.
    AS5600 encoder_;

    // Saving motor settings, such as motor's max position and other attributes
    Preferences motor_settings_;

    QueueHandle_t wireless_task_queue_;   // To receive messages from wireless task
    xTimerHandle system_sleep_timer_;     // To prevent system from sleeping before motor stops

    // User adjustable TMC2209 motor driver settings
    int microsteps_           = 16;
    int full_steps_per_rev_   = 200;  // NEMA motors have 200 full steps/rev
    int microsteps_per_rev_   = full_steps_per_rev_ * microsteps_;
    int velocity_             = static_cast<int>(microsteps_per_rev_ * 3);
    int acceleration_         = static_cast<int>(velocity_ * 0.5);
    bool direction_           = false;
    int opening_current_      = 200;
    int closing_current_      = 75;  // 1, 3: 200; 2: 400; 4: 300
    int stallguard_threshold_ = 10;

    // Motor task's internal states
    volatile bool stalled_    = false;
    portMUX_TYPE stalled_mux_ = portMUX_INITIALIZER_UNLOCKED;
    bool stallguard_enabled_  = true;
    bool driver_standby_      = false;

    // Encoder's attributes: we keep track of the overall position via encoder's position and then
    // convert it into motor's position and percentage.
    int32_t encod_pos_            = 0;
    int32_t encod_max_pos_        = 0;
    int8_t  last_updated_percent_ = -100;
    float motor_encoder_ratio_    = microsteps_per_rev_ / 4096.0;
    float encoder_motor_ratio_    = 4096.0 / microsteps_per_rev_;

    void stallguardInterrupt();
    void loadSettings(); // Load motor settings from flash
    void moveToPercent(int percent);
    void stop();
    bool setMin();
    bool setMax();
    bool motorEnable(uint8_t enable_pin, uint8_t value);
    inline int getPercent();
    inline int positionToSteps(int encoder_position);
    // For quick configuration guide, please refer to p70-72 of TMC2209's datasheet rev1.09
    // TMC2209's UART interface automatically becomes enabled when correct UART data is sent. It
    // automatically adapts to uC's baud rate. Block until UART is finished initializing so ESP32
    // can send settings to the driver via UART.
    void driverStartup();
    void driverStandby();

    // TODO set/get
    // void setMicrosteps()
    // void setVelocity() {}
    // void setAcceleration() {}
    // void setOpeningCurrent() {}
    // void setClosingCurrent() {}
    // void setDirection() {}
    // void disableStallguard() {}
    // void enableStallguard() {}
};