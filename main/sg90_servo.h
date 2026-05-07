/*
 * SG90 舵机控制 - 头文件
 */

#ifndef SG90_SERVO_H
#define SG90_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 设置舵机角度
 * 
 * @param degree 目标角度 (0-180)
 */
void servo_set_angle(int degree);

/**
 * @brief 平滑移动舵机到目标角度
 * 
 * @param from_degree 起始角度
 * @param to_degree 目标角度
 * @param step_delay_ms 每步延迟 (毫秒)
 */
void servo_move_smooth(int from_degree, int to_degree, int step_delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* SG90_SERVO_H */
