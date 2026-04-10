/*
 * PID.h
 *
 *  Created on: 10 apr 2026
 *      Author: simon
 */

#ifndef INC_PID_H_
#define INC_PID_H_

#include <stdint.h>

typedef struct {
    // Tuning parameters
    float kp;
    float ki;
    float kd;

    // Geheugen voor de berekeningen
    float integral;
    float previous_error;

    // Beveiligingen (Cruciaal voor drones)
    float integral_limit; // Voorkomt I-term "spool up" op de grond (Anti-Windup)
    float output_limit;   // Zorgt dat de mixer niet overstuurd raakt

    // Het uiteindelijke resultaat dat naar de motor mixer gaat
    float output;
} PID_Controller;

// Functie prototypes
void PID_Init(PID_Controller *pid, float kp, float ki, float kd, float i_limit, float out_limit);
void Calculate_PID(PID_Controller *pid, float setpoint, float measurement, float dt);

#endif /* INC_PID_H_ */
