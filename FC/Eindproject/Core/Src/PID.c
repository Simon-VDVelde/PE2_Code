/*
 * PID.c
 *
 *  Created on: 9 apr 2026
 *      Author: simon
 */

#include "PID.h"

PID_Controller pid_roll  = { .kp = 1.2, .ki = 0.05, .kd = 15.0 };
PID_Controller pid_pitch = { .kp = 1.2, .ki = 0.05, .kd = 15.0 };
PID_Controller pid_yaw   = { .kp = 2.0, .ki = 0.1,  .kd = 0.0  };

// dt is de tijd in seconden sinds de vorige loop (bijv. 0.001 voor 1kHz)
void Calculate_PID(PID_Controller *pid, float setpoint_rate, float current_gyro_rate, float dt) {
    // 1. Bereken de fout (Error)
    float error = setpoint_rate - current_gyro_rate;

    // 2. Proportioneel (P) - Directe reactie op de fout
    float p_term = pid->kp * error;

    // 3. Integraal (I) - Fout over tijd opbouwen (tegen externe krachten zoals wind/zwaartepunt)
    pid->integral += error * dt;
    // Let op: Voeg hier "Anti-windup" toe (limiteer de maximale waarde van pid->integral)
    float i_term = pid->ki * pid->integral;

    // 4. Afgeleide (D) - Kijkt naar de verandering van de fout om overcorrectie af te remmen
    float derivative = (error - pid->previous_error) / dt;
    float d_term = pid->kd * derivative;

    // Sla fout op voor de volgende loop
    pid->previous_error = error;

    // 5. Totale output
    pid->output = p_term + i_term + d_term;
}



