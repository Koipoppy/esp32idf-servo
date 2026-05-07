# SG90 Servo Control Component
idf_component_register(
    SRCS "sg90_servo.c"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_driver_ledc
        esp_driver_gpio
        freertos
)