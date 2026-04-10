/*
 * RC_Math.c
 *
 *  Created on: 10 apr 2026
 *      Author: simon
 */


// Definieer de grenzen van het ExpressLRS 11-bit protocol
#define RC_MIN 172.0f
#define RC_MID 992.0f
#define RC_MAX 1811.0f

// Definieer hoe "snel" je drone maximaal mag flippen (400 graden per seconde is een mooie beginnerswaarde)
#define MAX_RATE_ROLL_PITCH 400.0f
#define MAX_RATE_YAW        400.0f

// Dit zijn de variabelen die straks direct je PID-loop ingaan!
float target_rate_roll  = 0.0f;
float target_rate_pitch = 0.0f;
float target_rate_yaw   = 0.0f;
uint16_t target_throttle = 48; // 48 is DSHOT voor 'motoren draaien stationair / uit'

/*
 * @brief Rekent de 11-bit getallen (172 - 1811) om naar bruikbare eenheden
 */
void Calculate_Target_Rates(void) {

    // 1. Roll en Pitch (-400 tot +400 graden per seconde)
    // We trekken het midden (992) eraf, zodat we rondom 0 balanceren.
    target_rate_roll  = ((float)rc_roll - RC_MID) / (RC_MAX - RC_MID) * MAX_RATE_ROLL_PITCH;
    target_rate_pitch = ((float)rc_pitch - RC_MID) / (RC_MAX - RC_MID) * MAX_RATE_ROLL_PITCH;

    // 2. Yaw (Hetzelfde trucje)
    target_rate_yaw = ((float)rc_yaw - RC_MID) / (RC_MAX - RC_MID) * MAX_RATE_YAW;

    // 3. Throttle (Mappen naar DSHOT 48 t/m 2047)
    // Throttle heeft geen "midden" (als je stick beneden is, is het gas uit, niet 'achteruit')
    if (rc_throttle <= RC_MIN + 10) { // Een beetje 'deadband' onderin tegen trillende sticks
        target_throttle = 48;
    } else {
        // Map lineair van (172-1811) naar (48-2047)
        float throttle_percentage = ((float)rc_throttle - RC_MIN) / (RC_MAX - RC_MIN);
        target_throttle = 48 + (uint16_t)(throttle_percentage * (2047 - 48));
    }

    // Veiligheidscheck (zorgen dat DSHOT nooit boven 2047 komt)
    if(target_throttle > 2047) target_throttle = 2047;
}
