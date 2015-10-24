#include <stdio.h>
#include <stdlib.h>
#include <mraa.h>

#define pinDIR			9		//Direction
#define pinENB			8		//Enable
#define pinPWM			3		//Speed,PWM

mraa_gpio_context pin_dir, pin_enb;
mraa_pwm_context pin_pwm;

float speed = 0.1f;			//Speed of the motor, from 0.0f to 1.0f
int enable = 0;  			//Enable active high, 0 = LOW, 100 = HIGH
int direction = 0;			//Direction CCW Low, CW High, 0 = LOW, 100 = HIGH

//Initalize Pins
void setup(){
	pin_pwm = mraa_pwm_init(pinPWM);
	pin_enb = mraa_gpio_init(pinENB);
	pin_dir = mraa_gpio_init(pinDIR);
	mraa_gpio_dir(pin_enb, MRAA_GPIO_OUT);
	mraa_gpio_dir(pin_dir, MRAA_GPIO_OUT);
	mraa_pwm_period_us(pin_pwm, 1500);
	mraa_pwm_enable(pin_pwm, 1);

}

//Update Motor Parameters from console window
//Return 1 if continue running. Return 0 to terminate run.
int getMotorParameters()
{
	float inputSpeed = 0.0f;
	int inputEnable = 0;
	int inputDirection = 0;
	int terminate = 0;

	printf("Enter Speed(0.0f - 1.0f), Enable, Direction, Terminate (float int int int):");
	scanf("%f %d %d %d", &inputSpeed, &inputEnable, &inputDirection, &terminate);

	if(inputSpeed < 0.1f){
		speed = 0.1f;
	}
	else if(inputSpeed > 0.9f){
		speed = 0.9f;
	}
	else{
		speed = inputSpeed;
	}

	if(inputEnable == 0){
		enable = 0;
	}
	else{
		enable = 100;
	}

	if(inputDirection == 0){
		direction = 0;
	}
	else{
		direction = 100;
	}

	printf("Speed: %f, Enable: %d, Direction: %d\n", speed, enable, direction);
	return terminate;
}

void updateMotor(){
	//Update speed
	mraa_pwm_write(pin_pwm, speed);
	mraa_gpio_write(pin_enb, enable);
	mraa_gpio_write(pin_dir, direction); //CCW
}


int main()
{
	setup();
	int terminate = getMotorParameters();
	updateMotor();
	while(!terminate){
		terminate = getMotorParameters();
		updateMotor();
	}
	enable = 0;
	updateMotor();
	return 0;
}
