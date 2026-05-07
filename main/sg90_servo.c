/*
 * SG90 舵机控制 - ESP32-S3 LEDC PWM
 * 
 * 硬件接线:
 * SG90 VCC -> ESP32-S3 5V (或外部 5V 电源)
 * SG90 GND -> ESP32-S3 GND
 * SG90 SIG  -> ESP32-S3 GPIO2
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

// 标签用于日志输出
static const char *TAG = "SG90_SERVO";

// SG90 舵机参数
#define SERVO_MIN_DEGREE    0       // 最小角度
#define SERVO_MAX_DEGREE    180     // 最大角度
#define SERVO_MIN_US        500     // 0° 对应的脉宽 (微秒)
#define SERVO_MAX_US        2500    // 180° 对应的脉宽 (微秒)

// LEDC 配置
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_GPIO           GPIO_NUM_2      // PWM 输出引脚
#define LEDC_FREQ_HZ        50              // 50Hz (20ms 周期)
#define LEDC_TIMER_BIT      LEDC_TIMER_13_BIT
#define LEDC_MAX_DUTY       (1 << LEDC_TIMER_BIT)

/**
 * @brief 初始化 LEDC PWM
 */
static void servo_pwm_init(void)
{
    // 配置 LEDC 定时器
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_TIMER_BIT,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    // 配置 LEDC 通道
    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_GPIO,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));

    ESP_LOGI(TAG, "LEDC 初始化完成，频率=%dHz, GPIO=%d", LEDC_FREQ_HZ, LEDC_GPIO);
}

/**
 * @brief 将角度转换为占空比
 * 
 * @param degree 目标角度 (0-180)
 * @return uint32_t 占空比值
 */
static uint32_t angle_to_duty(int degree)
{
    // 限制角度范围
    if (degree < SERVO_MIN_DEGREE) {
        degree = SERVO_MIN_DEGREE;
    } else if (degree > SERVO_MAX_DEGREE) {
        degree = SERVO_MAX_DEGREE;
    }

    // 线性映射：角度 -> 脉宽 -> 占空比
    // 脉宽范围：500us - 2500us
    // 周期：20ms = 20000us
    float pulse_width_us = SERVO_MIN_US + 
        (float)(degree - SERVO_MIN_DEGREE) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) * 
        (SERVO_MAX_US - SERVO_MIN_US);
    
    // 计算占空比：duty = (pulse_width / period) * max_duty
    uint32_t duty = (uint32_t)((pulse_width_us / 20000.0f) * LEDC_MAX_DUTY);
    
    return duty;
}

/**
 * @brief 设置舵机角度
 * 
 * @param degree 目标角度 (0-180)
 */
void servo_set_angle(int degree)
{
    uint32_t duty = angle_to_duty(degree);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL));
    
    ESP_LOGD(TAG, "设置角度=%d°, 占空比=%lu", degree, duty);
}

/**
 * @brief 平滑移动舵机到目标角度
 * 
 * @param from_degree 起始角度
 * @param to_degree 目标角度
 * @param step_delay_ms 每步延迟 (毫秒)
 */
void servo_move_smooth(int from_degree, int to_degree, int step_delay_ms)
{
    int step = (to_degree > from_degree) ? 1 : -1;
    
    for (int angle = from_degree; 
         (step > 0) ? (angle <= to_degree) : (angle >= to_degree); 
         angle += step) {
        servo_set_angle(angle);
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }
}

/**
 * @brief 主任务
 */
void app_main(void)
{
    ESP_LOGI(TAG, "SG90 舵机控制程序启动");
    
    // 初始化 PWM
    servo_pwm_init();
    
    // 初始位置：90 度
    ESP_LOGI(TAG, "移动到初始位置 90°");
    servo_set_angle(90);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    while (1) {
        // 测试 1: 平滑扫描 0° -> 180°
        ESP_LOGI(TAG, "=== 测试：平滑扫描 0° -> 180° ===");
        servo_move_smooth(0, 180, 15);  // 约 2.7 秒完成
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 测试 2: 平滑扫描 180° -> 0°
        ESP_LOGI(TAG, "=== 测试：平滑扫描 180° -> 0° ===");
        servo_move_smooth(180, 0, 15);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 测试 3: 关键角度验证
        ESP_LOGI(TAG, "=== 测试：关键角度验证 ===");
        int test_angles[] = {0, 45, 90, 135, 180};
        for (int i = 0; i < 5; i++) {
            ESP_LOGI(TAG, "移动到 %d°", test_angles[i]);
            servo_set_angle(test_angles[i]);
            vTaskDelay(pdMS_TO_TICKS(1000));  // 每个角度停留 1 秒
        }
        
        // 测试 4: 往复运动
        ESP_LOGI(TAG, "=== 测试：往复运动 ===");
        for (int cycle = 0; cycle < 3; cycle++) {
            ESP_LOGI(TAG, "循环 %d/3", cycle + 1);
            servo_move_smooth(0, 180, 10);
            servo_move_smooth(180, 0, 10);
        }
        
        ESP_LOGI(TAG, "一轮测试完成，5 秒后重新开始...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}