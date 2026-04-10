/*
 * PID.c
 *
 *  Created on: 10 apr 2026
 *      Author: simon
 */
#include "PID.h"

/*
 * @brief Initialiseer de PID controller netjes op nul en stel de limieten in
 */
void PID_Init(PID_Controller *pid, float kp, float ki, float kd, float i_limit, float out_limit) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral = 0.0f;
    pid->previous_error = 0.0f;

    pid->integral_limit = i_limit;
    pid->output_limit = out_limit;

    pid->output = 0.0f;
}

/*
 * @brief Bereken de nieuwe motor-correctie op basis van de fout
 * @param setpoint Wat we willen (Graden per seconde uit de ontvanger)
 * @param measurement Wat we daadwerkelijk doen (Graden per seconde uit de BMI330)
 * @param dt De exacte delta-tijd in seconden sinds de vorige loop
 */
void Calculate_PID(PID_Controller *pid, float setpoint, float measurement, float dt) {

    // 1. Bereken de fout (Error)
    float error = setpoint - measurement;

    // 2. Proportioneel (P) - De directe, "domme" reactie op de fout
    float p_term = pid->kp * error;

    // 3. Integraal (I) - Fout over tijd opbouwen om constante krachten (wind/zwaartepunt) te overwinnen
    pid->integral += error * dt;

    // Anti-Windup: Klem de integraal af zodat hij niet oneindig doorgroeit
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }

    float i_term = pid->ki * pid->integral;

    // 4. Afgeleide (D) - Kijkt naar de verandering (snelheid) van de fout om de P-term af te remmen
    float derivative = 0.0f;
    if (dt > 0.0f) { // Voorkom delen door nul (Divide by Zero error)
        derivative = (error - pid->previous_error) / dt;
    }
    float d_term = pid->kd * derivative;

    // Sla de huidige fout op voor de berekening van de D-term in de volgende loop
    pid->previous_error = error;

    // 5. Totale Output berekenen
    pid->output = p_term + i_term + d_term;

    // 6. Output Limiteren
    // Als de DSHOT range 48 tot 2047 is, is een output limit van 500 een veilige marge
    if (pid->output > pid->output_limit) {
        pid->output = pid->output_limit;
    } else if (pid->output < -pid->output_limit) {
        pid->output = -pid->output_limit;
    }
}
