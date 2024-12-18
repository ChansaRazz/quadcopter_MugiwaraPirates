//#include "PS3.h"
//#include "MPU6050.h"
#include <Ps3Controller.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

#define m1 32
#define m2 23
#define m3 4
#define m4 27
#define red 13
#define green 2

#define freq  5000
#define resolution   8
#define ledChannel1  0
#define ledChannel2  1
#define ledChannel3  2
#define ledChannel4  3


MPU6050 mpu6050(Wire);
void MPU6050_SETUP() {

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  delay(100);
  Serial.println("MPU6050 calibrating...!!!!!");

}

void PS3_SETUP()
{
  Ps3.begin("38:4f:f0:95:e7:49");
  
  Serial.println("PS3 STARTED");


  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcSetup(ledChannel3, freq, resolution);
  ledcSetup(ledChannel4, freq, resolution);

  ledcAttachPin(m1, ledChannel1);
  ledcAttachPin(m2, ledChannel2);
  ledcAttachPin(m3, ledChannel3);
  ledcAttachPin(m4, ledChannel4);

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
}

void lights(int a) {
  digitalWrite(red, a);
  digitalWrite(green, a);
}


float roll_sample_angle = 0.0, pitch_sample_angle = 0.0, yaw_sample_angle = 0.0;
float roll_angle = 0.0, pitch_angle = 0.0, yaw_angle = 0.0;

float roll_sample_gyro = 0.0, pitch_sample_gyro = 0.0, yaw_sample_gyro = 0.0;
float roll_gyro = 0.0, pitch_gyro = 0.0, yaw_gyro = 0.0;


float esc_1, esc_2, esc_3, esc_4;
int throttle = 127;
int roll = 127;
int pitch = 127;
int yaw = 127;
bool drone = false;


float angle_roll_input = 0, angle_pitch_input = 0, angle_yaw_input = 0;
float pid_error_temp;
float pid_i_mem_roll, pid_roll_setpoint = 0, gyro_roll_input, pid_output_roll, pid_last_roll_d_error;
float pid_i_mem_pitch, pid_pitch_setpoint = 0, gyro_pitch_input, pid_output_pitch, pid_last_pitch_d_error;
float pid_i_mem_yaw, pid_yaw_setpoint = 0, gyro_yaw_input, pid_output_yaw, pid_last_yaw_d_error;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PID gain and limit settings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float pid_p_gain_roll = 0.15;               //Gain setting for the roll P-controller
float pid_i_gain_roll = 0;              //Gain setting for the roll I-controller
float pid_d_gain_roll =  0;              //Gain setting for the roll D-controller
int pid_max_roll = 50;                    //Maximum output of the PID-controller (+/-)

float pid_p_gain_pitch = 0.2;  //Gain setting for the pitch P-controller.
float pid_i_gain_pitch = pid_i_gain_roll;  //Gain setting for the pitch I-controller.
float pid_d_gain_pitch = pid_d_gain_roll;  //Gain setting for the pitch D-controller.
int pid_max_pitch = pid_max_roll;          //Maximum output of the PID-controller (+/-)

float pid_p_gain_yaw = 0.2;                //Gain setting for the pitch P-controller. //4.0
float pid_i_gain_yaw = 0;               //Gain setting for the pitch I-controller. //0.02
float pid_d_gain_yaw = 0;                //Gain setting for the pitch D-controller.
int pid_max_yaw = 50;                     //Maximum output of the PID-controller (+/-)



void setup() {
  Serial.begin(115200);
  PS3_SETUP();
  MPU6050_SETUP();

  lights(1);

  for (int x = 0; x < 2000; x++) {
    mpu6050.update();
    roll_sample_angle +=  mpu6050.getAngleY();
    pitch_sample_angle +=  mpu6050.getAngleX();
    yaw_sample_angle += (mpu6050.getAngleZ() * -1);


    roll_sample_gyro +=  mpu6050.getGyroY();
    pitch_sample_gyro +=  mpu6050.getGyroX();
    yaw_sample_gyro +=  (mpu6050.getGyroZ() * -1);

  }

  roll_sample_angle /=  2000;
  pitch_sample_angle /= 2000;
  yaw_sample_angle /= 2000;

  roll_sample_gyro /=  2000;
  pitch_sample_gyro /=  2000;
  yaw_sample_gyro /=  2000;

  lights(0);

  Serial.println("MPU6050 calibrated");

//    Serial.println();
//  Serial.println();
//  delay(1000);
//
//  Serial.print(roll_sample_angle);
//  Serial.print("\t");
//  Serial.print(pitch_sample_angle);
//  Serial.print("\t");
//  Serial.println(yaw_sample_angle);
//  Serial.println();
//  Serial.println();
//  delay(1000);
//  
//
//  Serial.print(roll_sample_gyro);
//  Serial.print("\t");
//  Serial.print(pitch_sample_gyro);
//  Serial.print("\t");
//  Serial.println(yaw_sample_gyro);
//  delay(5000);
  

}
void calculate_pid() {
  //Roll calculations
  pid_error_temp = gyro_roll_input - pid_roll_setpoint;
  pid_i_mem_roll += pid_i_gain_roll * pid_error_temp;
  if (pid_i_mem_roll > pid_max_roll)pid_i_mem_roll = pid_max_roll;
  else if (pid_i_mem_roll < pid_max_roll * -1)pid_i_mem_roll = pid_max_roll * -1;

  pid_output_roll = pid_p_gain_roll * pid_error_temp + pid_i_mem_roll + pid_d_gain_roll * (pid_error_temp - pid_last_roll_d_error);
  if (pid_output_roll > pid_max_roll)pid_output_roll = pid_max_roll;
  else if (pid_output_roll < pid_max_roll * -1)pid_output_roll = pid_max_roll * -1;

  pid_last_roll_d_error = pid_error_temp;
  
//  Serial.println(pid_output_roll);
//  Serial.print("\t");
 // delay(1000);

  //Pitch calculations
  pid_error_temp = gyro_pitch_input - pid_pitch_setpoint;
  pid_i_mem_pitch += pid_i_gain_pitch * pid_error_temp;
  if (pid_i_mem_pitch > pid_max_pitch)pid_i_mem_pitch = pid_max_pitch;
  else if (pid_i_mem_pitch < pid_max_pitch * -1)pid_i_mem_pitch = pid_max_pitch * -1;

  pid_output_pitch = pid_p_gain_pitch * pid_error_temp + pid_i_mem_pitch + pid_d_gain_pitch * (pid_error_temp - pid_last_pitch_d_error);
  if (pid_output_pitch > pid_max_pitch)pid_output_pitch = pid_max_pitch;
  else if (pid_output_pitch < pid_max_pitch * -1)pid_output_pitch = pid_max_pitch * -1;

  pid_last_pitch_d_error = pid_error_temp;
  
//  Serial.println(pid_output_pitch);
//  Serial.print("\t");
//  delay(1000);
  //Yaw calculations
  pid_error_temp = gyro_yaw_input - pid_yaw_setpoint;
  pid_i_mem_yaw += pid_i_gain_yaw * pid_error_temp;
  if (pid_i_mem_yaw > pid_max_yaw)pid_i_mem_yaw = pid_max_yaw;
  else if (pid_i_mem_yaw < pid_max_yaw * -1)pid_i_mem_yaw = pid_max_yaw * -1;

  pid_output_yaw = pid_p_gain_yaw * pid_error_temp + pid_i_mem_yaw + pid_d_gain_yaw * (pid_error_temp - pid_last_yaw_d_error);
  if (pid_output_yaw > pid_max_yaw)pid_output_yaw = pid_max_yaw;
  else if (pid_output_yaw < pid_max_yaw * -1)pid_output_yaw = pid_max_yaw * -1;

  pid_last_yaw_d_error = pid_error_temp;
    
//  Serial.println(pid_output_yaw);
//  Serial.print("\t");
//  delay(1000);
}

void loop() {

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //ANGLE & GYRO INPUTS
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mpu6050.update();

  roll_angle = mpu6050.getAngleY() - roll_sample_angle;
  pitch_angle = mpu6050.getAngleX() - pitch_sample_angle;
  yaw_angle = (mpu6050.getAngleZ() * -1) - yaw_sample_angle;

  roll_gyro =  mpu6050.getGyroY() - roll_sample_gyro;
  pitch_gyro =  mpu6050.getGyroX() - pitch_sample_gyro;
  yaw_gyro =  (mpu6050.getGyroZ() * -1) - yaw_sample_gyro;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //COMPLIMENTARY FILTER 100 = 70(of previous value) + 30(of updated value)
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  angle_roll_input = (angle_roll_input * 0.94) + ((roll_angle) * 0.06);     //roll angle
  angle_pitch_input = (angle_pitch_input * 0.94) + ((pitch_angle) * 0.06);  //pitch angle
  angle_yaw_input = (angle_yaw_input * 0.94) + ((yaw_angle) * 0.06);        //yaw angle

  gyro_roll_input = (gyro_roll_input * 0.7) + ((roll_gyro) * 0.3)  ;     //roll gyro
  gyro_pitch_input = (gyro_pitch_input * 0.7) + ((pitch_gyro) * 0.3);    //pitch gyro
  gyro_yaw_input = (gyro_yaw_input * 0.7) + ((yaw_gyro) * 0.3);          //yaw gyro



  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //PS3 INPUTS
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  throttle = Ps3.data.analog.stick.ly;
  throttle = map(throttle, 127, -128, 0, 255);

  pid_roll_setpoint = 0;
  roll = Ps3.data.analog.stick.rx;
  pid_roll_setpoint = map(roll, 127, -128, 80, -80);
  pid_roll_setpoint -= angle_roll_input;
  pid_roll_setpoint /= 2;



  pid_pitch_setpoint = 0;
  pitch = Ps3.data.analog.stick.ry;
  pid_pitch_setpoint = map(pitch, 127, -128, 80, -80);
  pid_pitch_setpoint -= angle_pitch_input;
  pid_pitch_setpoint /= 2;

  pid_yaw_setpoint = 0;
  pid_yaw_setpoint -= angle_yaw_input;
  pid_yaw_setpoint /= 3.0;

  //  Serial.println(pid_roll_setpoint);
  //  Serial.print("\t");
  //  Serial.println(pid_pitch_setpoint);
  //  Serial.print("\t");
  //  Serial.println(pid_yaw_setpoint);

  //  Serial.print(gyro_roll_input);
  //  Serial.print("\t");
  //  Serial.print(gyro_pitch_input);
  //  Serial.print("\t");
  //  Serial.println(gyro_yaw_input);




  if ( Ps3.data.button.cross)
  { drone = true;


    //Reset the PID controllers for a bumpless start.
    pid_i_mem_roll = 0;
    pid_last_roll_d_error = 0;
    pid_i_mem_pitch = 0;
    pid_last_pitch_d_error = 0;
    pid_i_mem_yaw = 0;
    pid_last_yaw_d_error = 0;
  }

  if (Ps3.data.button.circle)
  {
    drone = false;
  }

  calculate_pid();



  if ( drone == true ) {

    lights(1);
    esc_2 = throttle - pid_output_pitch + pid_output_roll - pid_output_yaw; //Calculate the pulse for esc 1 (front-right - CCW)
    esc_3 = throttle + pid_output_pitch + pid_output_roll + pid_output_yaw; //Calculate the pulse for esc 2 (rear-right - CW)
    esc_4 = throttle + pid_output_pitch - pid_output_roll - pid_output_yaw; //Calculate the pulse for esc 3 (rear-left - CCW)
    esc_1 = throttle - pid_output_pitch - pid_output_roll + pid_output_yaw; //Calculate the pulse for esc 4 (front-left - CW)

    if (esc_1 < 10) esc_1 = 0;
    if (esc_2 < 10) esc_2 = 0;
    if (esc_3 < 10) esc_3 = 0;
    if (esc_4 < 10) esc_4 = 0;

    if (esc_1 > 255)esc_1 = 255;
    if (esc_2 > 255)esc_2 = 255;
    if (esc_3 > 255)esc_3 = 255;
    if (esc_4 > 255)esc_4 = 255;

//            Serial.print("angle Pitch:");
//            Serial.print(mpu6050.getAngleX());
//            Serial.print("angle Roll:");
//            Serial.print(mpu6050.getAngleY());
//            Serial.print("angle Yaw:");
//            Serial.println(mpu6050.getAngleZ());
//            Serial.print("gyro Pitch:");
//            Serial.print(mpu6050.getAngleX());
//            Serial.print("gyro Roll:");
//            Serial.print(mpu6050.getAngleY());
//            Serial.print("gyro Yaw:");
//            Serial.println(mpu6050.getAngleZ());
          Serial.print("angle Roll:");
          Serial.print(roll_angle);
          Serial.print("angle Pitch:");
          Serial.print(pitch_angle);
          Serial.print("angle Yaw:");
          Serial.println(yaw_angle);

//          Serial.print("Erorr Roll:");
//          Serial.print(pid_last_roll_d_error);
//          Serial.print("Erorr Pitch:");
//          Serial.print(pid_last_pitch_d_error);
//          Serial.print("Erorr Yaw:");
//          Serial.println(pid_last_yaw_d_error);

        float q = pid_roll_setpoint - roll_angle;
        float w = pid_pitch_setpoint - pitch_angle;
        float e = pid_yaw_setpoint - yaw_angle;
          Serial.print("Erorr Steady State Roll: ");
          Serial.print(q);
          Serial.print("Erorr Steady State Pitch: ");
          Serial.print(w);
          Serial.print("Erorr Steady State Yaw: ");
          Serial.println(e);
//          Serial.print(pid_output_roll);
//          Serial.print("\t");
//          Serial.print(pid_output_pitch);
//          Serial.print("\t");
//          Serial.println(pid_output_yaw);
          //Serial.print("\t");
//        Serial.print(esc_1);
//        Serial.print("\t");
//        Serial.print(esc_2);
//        Serial.print("\t");
//        Serial.print(esc_3);
//        Serial.print("\t");
//        Serial.println(esc_4);



    ledcWrite(ledChannel1, esc_1);
    ledcWrite(ledChannel2, esc_2);
    ledcWrite(ledChannel3, esc_3);
    ledcWrite(ledChannel4, esc_4);
  }
  else {
    lights(0);
    ledcWrite(ledChannel1, 0);
    ledcWrite(ledChannel2, 0);
    ledcWrite(ledChannel3, 0);
    ledcWrite(ledChannel4, 0);
  }



}
