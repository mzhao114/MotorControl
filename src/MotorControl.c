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
int rampDelay = 80000;		//How long to delay in between each ramp, in us.
int changeDirRampDelay = 100000; //How long to delay in between each ramp, in us, for when dir changes.

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
	int ramp = 0;

	printf("Enter Speed, Enable, Direction, Ramp (int(0-100) int(0,1) int(0,1) int(0,1)):");
	scanf("%f %d %d %d", &inputSpeed, &inputEnable, &inputDirection, &ramp);

	//Note: ESCON Controller does not accept Duty Cycles of <10 or >90. Thus, if input speed is
	//Less than 10, set it to 10, and if it is greater than 90, set it to 90.
	if(inputSpeed <= 10){
		speed = 0.1f;

	}
	else if(inputSpeed >= 90){
		speed = 0.9f;
	}
	else{
		speed = ((float)inputSpeed)/100.0;
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

	printf("Speed: %f, Enable: %d, Direction: %d, Ramp: %d\n", speed, enable, direction, ramp);
	return ramp;
}

void updateMotor(){
	//Update speed. If speed is outside of controller tolerances, set to max/min speeds within tolerance
	if(speed <= 0.1f){
		speed = 0.1f;
	}
	else if (speed >= 0.9f){
		speed = 0.9f;
	}
	//Else speed remains the same.
	mraa_pwm_write(pin_pwm, speed);
	mraa_gpio_write(pin_enb, enable);
	mraa_gpio_write(pin_dir, direction); //CCW
	printf("Speed: %f, Enable: %d, Direction: %d\n", speed, enable, direction);
}


int main()
{
	setup();
	int ramp = getMotorParameters();
	updateMotor();

	//Always run
	while(1){
		//Get the current values for speed and direction
		float currentSpeed = speed;
		int currentDirection = direction;
		ramp = getMotorParameters();
		//Store inputs to confirm that final values are assigned to inputs
		float inputSpeed = speed;
		//if ramp is true, then ramp the change in motor speeds
		if(ramp){

			//If directions differ, take that into account
			if(currentDirection == direction){
				//Get the difference in the current speed and the new speed
				float deltaSpeed = currentSpeed - inputSpeed; //Negative speed means that new speed is higher than old
				float delta = deltaSpeed / 100.0;

				//Now set the current speed to the old speed
				speed = currentSpeed;

				//Increment by delta 100 times to reach the final speed.
				int i = 0;
				while(i < 100){
					speed = speed - delta;
					updateMotor();
					usleep(rampDelay);
					i = i + 1;
				}
				//Assign speed to input speed to ensure input value is assigned after delay.
				speed = inputSpeed;
				updateMotor();
			}
			else{
				//New direction is different than current direction
				float deltaSpeed = currentSpeed + speed - .2f; //To account for low threshold
				float delta = deltaSpeed / 100.0;
				direction = currentDirection;		//Don't change the direction yet.
				int changedDirection = 0;			//=1 when the direction has changed.
				//Increment by delta 100 times to reach final speed
				speed = currentSpeed;
				int i = 0;
				while(i<100){
					//If the current speed is less than 2 times delta speed, change the direction
					if(speed <= (0.1f + 2*delta)){
						if(currentDirection == 0){
							direction = 100;
						}
						else{
							direction = 0;
						}

					}
					//Now if "currentDirection" is different from direction, then the direction has changed.
					//Thus, increase speed now.
					if(direction == currentDirection){
						speed = speed - delta;
					}
					else{
						speed = speed + delta;
						if(speed >= currentSpeed){
							i = 100; //End loop
						}
					}
					updateMotor();
					usleep(changeDirRampDelay);
					i = i + 1;
				}
				//Assign speed to the input speed to ensure that the input value is finally assigned.
				speed = inputSpeed;
				updateMotor();
			}
		}
		else{
			updateMotor();
		}
	}
	enable = 0;
	updateMotor();
	return 0;
}
/*  NO RAMP FUNCTIONALITY
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
*/
