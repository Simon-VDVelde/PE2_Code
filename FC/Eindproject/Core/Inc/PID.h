/*
 * PID.h
 *
 *  Created on: 9 apr 2026
 *      Author: simon
 */

#ifndef INC_PID_H_
#define INC_PID_H_

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float previous_error;
    float output;
} PID_Controller;

#endif /* INC_PID_H_ */
