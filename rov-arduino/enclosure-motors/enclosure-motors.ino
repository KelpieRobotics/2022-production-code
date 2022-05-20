// Copyright 2022 Kelpie Robotics
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// define coordinate structs
struct CartesianCoordinates
{
    float x;
    float z;
};

struct PolarCoordinates
{
    float r;
    float theta;
};

// define pins
int PIN__CLAW_1         = 8;           // Claw servo = J7
int PIN__CLAW_2         = 9;           // Claw servo = HOTWIRED
int PIN__FRONT_LEFT     = 2;     // Front left motor = J1
int PIN__FRONT_RIGHT    = 3;    // Front right motor = J2
int PIN__BACK_LEFT      = 4;      // Back left motor = J3
int PIN__BACK_RIGHT     = 5;     // Back right motor = J4
int PIN__LEFT_VERTICAL  = 6;  // Left vertical motor = J5
int PIN__RIGHT_VERTICAL = 7;        // Right vertical motor = J6
LiquidCrystal_I2C lcd(0x27,20,4);

// define outputs
Servo claw;
Servo frontLeft;
Servo frontRight;
Servo backLeft;
Servo backRight;
Servo leftVertical;
Servo rightVertical;

// define data constants
const float MAX_SPEED = 200.0f; // Max speed of motors, should be between 0 and 400

const float MOD_FRONT_LEFT = -1.0f;     // Modifier for front left motor, should be between 0 and 1
const float MOD_FRONT_RIGHT = 1.0f;    // Modifier for front right motor, should be between 0 and 1
const float MOD_BACK_LEFT = -0.80f;      // Modifier for back left motor, should be between 0 and 1
const float MOD_BACK_RIGHT = -1.0f;     // Modifier for back right motor, should be between 0 and 1
const float MOD_LEFT_VERTICAL = -1.0f;  // Modifier for left vertical motor, should be between 0 and 1
const float MOD_RIGHT_VERTICAL = -1.0f; // Modifier for right vertical motor, should be between 0 and 1

const int DEBUG_MODE = 0; // Debug mode, 0 = regular, 1 = only Y move, 2 = only X move, 3 = only Z move, 4 = only claw move

// define data globals
int x = 0;                   // Stores horizontal dpad value
int z = 0;                   // Stores vertical dpad value
float speed_X = 0.0f;          // Stores speed of X axis
float speed_Z = 0.0f;          // Stores speed of Z axis
float speed_Y = 0.0f;        // Stores speed of Y axis
float speed_Rotation = 0.0f; // Stores speed of Rotation
int state_Claw = 0;          // Stores state of Claw, 0 = open, 1 = closed
int verticalLock = 0;        // Stores vertical movement lock, 0 = unlocked, 1 = locked
int rotationLock = 0;        // Stores rotation lock, 0 = unlocked, 1 = locked
char stick_or_pad;
int speedMod = 10;

int last_top = 0;             // Stores last top value
int last_bottom = 0;          // Stores last bottom value

int time = 0;

short fl_buffer[64] = {};
short fr_buffer[64] = {};
short bl_buffer[64] = {};
short br_buffer[64] = {};
short vl_buffer[64] = {};
short vr_buffer[64] = {};

short fl;
short fr;
short bl;
short br;
short vl;
short vr;

int fl_sum;
int fr_sum;
int bl_sum;
int br_sum;
int vl_sum;
in6/33333333333333333333333333t vr_sum;

int counter = 0;

void MoveVertical(float speed_Y)
{
    float totalSpeed = speed_Y * MAX_SPEED;

    // set left vertical motor
    vl = 1500 + (int)(totalSpeed * MOD_LEFT_VERTICAL);
    // set right vertical motor
    vr = 1500 + (int)(totalSpeed * MOD_RIGHT_VERTICAL);
}

// defunct now
void MoveHorizontal(float speed_X, float speed_Z)
{
    float x_norm = (speed_X == 0.0f) ? 0.00000001f : speed_X;
    float z_norm = (speed_Z == 0.0f) ? 0.00000001f : speed_Z;
    
    float cleanX = 0.0;
    float cleanZ = 0.0;

    int good_angle = 0;
    int good_mag = 0;
    
    float angle = atan2(speed_Z, speed_X);
    
    float result_angle = angle * 180 / PI;
    
    float mag = sqrt(pow(speed_X, 2) + pow(speed_Z, 2));
    
    int quad_angle = (int) result_angle % 45;
    
    good_angle = (quad_angle >= 5.0f) && (quad_angle <= 40.0f);
    good_mag = mag >= 0.1f;
    
    if (!good_angle)
    {
        int quad = result_angle / 45.0f;
        
        result_angle = (quad_angle < 5.0f) 
        ? quad * 45.0f
        : (quad == 7)
        ? 0.0f
        : (quad + 1.0f) * 45.0f;
    }
    
    cleanX = (good_mag) ? mag * cos(result_angle * PI / 180) : 0.0f;
    cleanZ = (good_mag) ? mag * sin(result_angle * PI / 180) : 0.0f;
    
    float totalSpeed = MAX_SPEED * sqrt(pow(cleanX, 2) + pow(cleanZ, 2)) / sqrt(2.0f);

//    Serial.print("Total speed: ");
//    Serial.print(totalSpeed);

    // create cartesian coordinate struct with coordinates from the x and z speeds
    CartesianCoordinates baseCoords = {1.0f * cleanX, 1.0f * cleanZ};

    // create a polar coordinate struct with the base cartesian coordinates
    PolarCoordinates basePolar = {totalSpeed, atan2(cleanZ, cleanX)};

    // rotate the basePolar struct by 1/4*PI clockwise
    basePolar.theta = (basePolar.theta > (7/4) * PI) ? basePolar.theta - (2.0f * PI) + (PI/4.0f) : basePolar.theta + (PI/4.0f);

    // create cartesian coordinate struct with the rotated polar coordinates
    CartesianCoordinates rotatedCoords = {basePolar.r * cos(basePolar.theta), basePolar.r * sin(basePolar.theta)};

//    Serial.print("coords: ");
//    Serial.print(rotatedCoords.x);
//    Serial.print(", ");
//    Serial.print(rotatedCoords.z);

    // set motor speeds with the new proper values, with the following mappings:
    // front left motor/back right motor: positive x axis
    // front right motor/back left motor: negative z axis

    int fl = 1500 + (int)(rotatedCoords.z * MOD_FRONT_LEFT);
    int fr = 1500 - (int)(rotatedCoords.x * MOD_FRONT_RIGHT);
    int bl = 1500 - (int)(rotatedCoords.x * MOD_BACK_LEFT);
    int br = 1500 + (int)(rotatedCoords.z * MOD_BACK_RIGHT);

//    Serial.print("All speeds: ");
//    Serial.print(fl);
//    Serial.print(", ");
//    Serial.print(fr);
//    Serial.print(", ");
//    Serial.print(bl);
//    Serial.print(", ");
//    Serial.print(br);

    // set front left motor
    frontLeft.writeMicroseconds(fl);

    // set front right motor
    frontRight.writeMicroseconds(fr);

    // set back left motor
    backLeft.writeMicroseconds(bl);

    // set back right motor
    backRight.writeMicroseconds(br);
}

void MoveUsingDPad(int z, int x, int speedMod)
{
    float speedModFloat = ((float) speedMod / 10.0f) * MAX_SPEED;

    float totalSpeedX = speedModFloat;
    float totalSpeedZ = speedModFloat;

    // hard-coding these commands is kinda painful and I don't like the idea of doing it, but we kinda have 
    int cartesianToNumpad = (5) + x + (z * 3);

    switch (cartesianToNumpad)
    {
    case 1:
        totalSpeedX *= -1.0f;
        totalSpeedZ *=  0.0f;
        break;
    case 2:
        totalSpeedX *= -1.0f;
        totalSpeedZ *= -1.0f;
        break;
    case 3:
        totalSpeedX *=  0.0f;
        totalSpeedZ *= -1.0f;
        break;
    case 4:
        totalSpeedX *=  1.0f;
        totalSpeedZ *=  0.0f;
        break;
    case 5:
        totalSpeedX *=  0.0f;
        totalSpeedZ *=  0.0f;
        break;
    case 6:
        totalSpeedX *= -1.0f;
        totalSpeedZ *=  1.0f;
        break;
    case 7:
        totalSpeedX *=  0.0f;
        totalSpeedZ *=  1.0f;
        break;
    case 8:
        totalSpeedX *=  1.0f;
        totalSpeedZ *=  1.0f;
        break;
    case 9:
        totalSpeedX *=  1.0f;
        totalSpeedZ *=  0.0f;
        break;
    }

    fr = 1500 + (int)(totalSpeedX * MOD_FRONT_RIGHT);
    bl = 1500 + (int)(totalSpeedX * MOD_BACK_LEFT);
    fl = 1500 + (int)(totalSpeedZ * MOD_FRONT_LEFT);
    br = 1500 + (int)(totalSpeedZ * MOD_BACK_RIGHT);
}

void MoveWithNoOffset(int speed_X, int speed_Z)
{
    float totalSpeedX = speed_X * MAX_SPEED;
    float totalSpeedZ = speed_Z * MAX_SPEED;

    fr = 1500 + (int)(totalSpeedZ * MOD_FRONT_RIGHT);
    bl = 1500 + (int)(totalSpeedZ * MOD_BACK_LEFT);

    fl = 1500 + (int)(totalSpeedX * MOD_FRONT_LEFT);
    br = 1500 + (int)(totalSpeedX * MOD_BACK_RIGHT);
}

void Rotate(float rotationSpeed)
{
    int totalSpeed = (int) (rotationSpeed * MAX_SPEED);
    //Serial.println(totalSpeed);

    // set front left motor
    fl = (1500 + (int) (totalSpeed * MOD_FRONT_LEFT));

    // set front right motor
    fr = (1500 - (int) (totalSpeed * MOD_FRONT_RIGHT));

    // set back left motor
    bl = (1500 + (int) (totalSpeed * MOD_BACK_LEFT));

    // set back right motor
    br = (1500 - (int) (totalSpeed * MOD_BACK_RIGHT));
}

void RotateDigital(float rotationSpeed)
{
    int _rotSpeed = (rotationSpeed > 0.4f) ? 200 : (rotationSpeed < -0.4f) ? -200 : 0;

    fl = (1500 + (int) (_rotSpeed  * MOD_FRONT_LEFT));

    // set front right motor
    fr = (1500 - (int) (_rotSpeed  * MOD_FRONT_RIGHT));

    // set back left motor
    bl = (1500 + (int) (_rotSpeed  * MOD_BACK_LEFT));

    // set back right motor
    br = (1500 - (int) (_rotSpeed  * MOD_BACK_RIGHT));
}

void SetClawState(int state)
{
    switch (state)
    {

    case 1: // open
        digitalWrite(PIN__CLAW_1, LOW);
        digitalWrite(PIN__CLAW_2, HIGH);
        break;

    case -1: // close
        digitalWrite(PIN__CLAW_1, HIGH);
        digitalWrite(PIN__CLAW_2, LOW);
        break;

    default: // stop
        digitalWrite(PIN__CLAW_1, LOW);
        digitalWrite(PIN__CLAW_2, LOW); 
        break;
    }
    //  Serial.println(state);
}

void setup()
{
    // start serial
    Serial.begin(9600);

    // initialize servos
    pinMode(PIN__CLAW_1, OUTPUT);
    pinMode(PIN__CLAW_2, OUTPUT);

    for (int i = 0; i < 64; i ++)
    {
	fl_buffer[i] = 1500;
	fr_buffer[i] = 1500;
	bl_buffer[i] = 1500;
	br_buffer[i] = 1500;
	vl_buffer[i] = 1500;
	vr_buffer[i] = 1500;
    }
  
    frontLeft.attach(PIN__FRONT_LEFT);
    frontRight.attach(PIN__FRONT_RIGHT);
    backLeft.attach(PIN__BACK_LEFT);
    backRight.attach(PIN__BACK_RIGHT);
    leftVertical.attach(PIN__LEFT_VERTICAL);
    rightVertical.attach(PIN__RIGHT_VERTICAL);

    // set servo positions
    digitalWrite(PIN__CLAW_1, LOW);        // set claw as open
    digitalWrite(PIN__CLAW_2, LOW);
    frontLeft.writeMicroseconds(1500);     // set front left motor as stopped
    frontRight.writeMicroseconds(1500);    // set front right motor as stopped
    backLeft.writeMicroseconds(1500);      // set back left motor as stopped
    backRight.writeMicroseconds(1500);     // set back right motor as stopped
    leftVertical.writeMicroseconds(1500);  // set left vertical motor as stopped
    rightVertical.writeMicroseconds(1500); // set right vertical motor as stopped

    delay(3000);

    Serial.println("READY");
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void loop()
{
    // read in the command and set variables accordingly if Serial has values
    if (DEBUG_MODE >= 1 && DEBUG_MODE <= 5)
    {
        switch (DEBUG_MODE)
        {
            case 1: // just Y
                speed_X = 0;
                speed_Z = 0;
                speed_Y = 1.0f;
                speed_Rotation = 0;
                state_Claw = 0;
                break;

            case 2: // just Z
                speed_X = 0;
                speed_Z = 1.0f;
                speed_Y = 0;
                speed_Rotation = 0;
                state_Claw = 0;
                break;

            case 3: // just X
                speed_X = 1.0f;
                speed_Z = 0;
                speed_Y = 0;
                speed_Rotation = 0;
                state_Claw = 0;
                break;

            case 5: // all!
                frontRight.writeMicroseconds(1800);
                backRight.writeMicroseconds(1800);
                frontLeft.writeMicroseconds(1800);
                backLeft.writeMicroseconds(1800);
                leftVertical.writeMicroseconds(1800);
                rightVertical.writeMicroseconds(1800);
                break;
            
            case 4: // just claw open
                speed_X = 1.0f;
                speed_Z = 0.0f;
                speed_Y = 1.0f;
                speed_Rotation = 0;
                state_Claw = 1;
                break;
        }
        MoveHorizontal(speed_X, speed_Z);
//        Serial.print("MOVED");
        SetClawState(state_Claw);
        MoveVertical(speed_Y);
    }
    else
    {
        if (Serial.available() > 0)
        {
            String inputData = Serial.readStringUntil('\n');
            ParseCommands(inputData);
        }

        if (DEBUG_MODE == 6)
        {
            // display on 20x4 lcd display the values of speed x, speed y, speed z, and claw
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("X: ");
            lcd.print(speed_X);
            lcd.setCursor(0, 1);
            lcd.print("Y: ");
            lcd.print(speed_Y);
            lcd.setCursor(0, 2);
            lcd.print("Z: ");
            lcd.print(speed_Z);
            lcd.setCursor(0, 3);
            lcd.print("Claw: ");
            lcd.print(state_Claw);
        }

        else if (DEBUG_MODE == 9)
        {
            time = millis();
//            Serial.print("LEFT FRONT - J1");
            while (millis() - time <= 3000)
            {
                frontLeft.writeMicroseconds(1800);
            }
            frontLeft.writeMicroseconds(1500);
            delay(1000);

            time = millis();
//            Serial.println("FRONT RIGHT - J2");
            while (millis() - time <= 3000)
            {
                frontRight.writeMicroseconds(1800);
            }
            frontRight.writeMicroseconds(1500);
            delay(1000);

            time = millis();
//            Serial.println("BACK LEFT - J3");
            while (millis() - time <= 3000)
            {
                backLeft.writeMicroseconds(1800);
            }
            backLeft.writeMicroseconds(1500);
            delay(1000);
            

            time = millis();
//            Serial.println("BACK RIGHT - J4");
            while (millis() - time <= 3000)
            {
                backRight.writeMicroseconds(1800);
            }
            backRight.writeMicroseconds(1500);
            delay(1000);

            time = millis();
//            Serial.println("LEFT VERTICAL - J5");
            while (millis() - time <= 3000)
            {
                leftVertical.writeMicroseconds(1800);
            }
            leftVertical.writeMicroseconds(1500);
            delay(1000);

            time = millis();
//            Serial.println("RIGHT VERTICAL - J6");
            while (millis() - time <= 3000)
            {
                rightVertical.writeMicroseconds(1800);
            }
            rightVertical.writeMicroseconds(1500);
            delay(1000);
        }
        else
        {
            // call commands if locks not set
            if (verticalLock == 0)
                MoveVertical(speed_Y);

            if (rotationLock == 0 && (speed_Rotation > 0.4f || speed_Rotation < -0.4f))
	    {
		RotateDigital(speed_Rotation);
		delay(1);
	    }
               
            if (speed_Rotation <= 0.4f && speed_Rotation >= -0.4f) 
            {
                switch (stick_or_pad)
                {
                    case 'S':
                        MoveWithNoOffset(speed_X, speed_Z);
                        break;
                    case 'D':
                        MoveUsingDPad(z, x, speedMod);
                        break;
                    default:
                        break;
                }
            }
            SetClawState(state_Claw);

            fl_buffer[counter] = fl;
	    fr_buffer[counter] = fr;
	    bl_buffer[counter] = bl;
	    br_buffer[counter] = br;
	    vl_buffer[counter] = vl;
	    vr_buffer[counter] = vr;

	    counter = (counter + 1) % 255;

	    fl_sum = 0;
            fr_sum = 0;
	    bl_sum = 0;
	    br_sum = 0;
	    vl_sum = 0;
	    vr_sum = 0;

	    for (int i = 0; i < 64; i++)
	    {
		fl_sum += fl_buffer[i];
		fr_sum += fr_buffer[i];
		bl_sum += bl_buffer[i];
		br_sum += br_buffer[i];
		vl_sum += vl_buffer[i];
		vr_sum += vr_buffer[i];
	    }

	    fl_sum /= 64;
	    fr_sum /= 64;
	    bl_sum /= 64;
	    br_sum /= 64;
	    vl_sum /= 64;
	    vr_sum /= 64;

	    frontLeft.writeMicroseconds(fl_sum);
	    frontRight.writeMicroseconds(fr_sum);
	    backLeft.writeMicroseconds(bl_sum);
	    backRight.writeMicroseconds(br_sum);
	    leftVertical.writeMicroseconds(vl_sum);
	    rightVertical.writeMicroseconds(vr_sum);
	}
    }

    delay(1);
}

void ParseCommands(String dataFromPi)
{
    if (dataFromPi == "TYPE") // if the Pi polled for type rather than sending a motor call, reply with "MOTOR" and return
    {
        Serial.println("MOTOR");
        return;
    }

    // this array is as follows: [0] = movement inputs, [1] = right stick, [2] = claw states + buttons
    String dataFromPiArray[4];
    String tmp_str = dataFromPi;

    // split the data into the three elements
    dataFromPiArray[3] = tmp_str.substring(0, tmp_str.indexOf('\t'));
    tmp_str = tmp_str.substring(tmp_str.indexOf('\t') + 1);
    dataFromPiArray[0] = tmp_str.substring(0, tmp_str.indexOf('\t'));
    tmp_str = tmp_str.substring(tmp_str.indexOf('\t') + 1);
    dataFromPiArray[1] = tmp_str.substring(0, tmp_str.indexOf('\t'));
    tmp_str = tmp_str.substring(tmp_str.indexOf('\t') + 1);
    dataFromPiArray[2] = tmp_str.substring(0, tmp_str.indexOf('\t'));

    // parse the first element
    tmp_str = dataFromPiArray[0];
    String x_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String z_String = tmp_str.substring(0, tmp_str.indexOf(','));

    // parse the second element
    tmp_str = dataFromPiArray[1];
    String speed_Rotation_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String speed_Y_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    
    // parse the third element
    tmp_str = dataFromPiArray[2];
    String L_button_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String R_button_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String L_trigger_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String R_trigger_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String top_button_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String bottom_button_String = tmp_str;

    stick_or_pad = dataFromPiArray[3].charAt(0);

    // convert the strings to ints and floats

    switch (stick_or_pad)
    {

    case 'S':
        // stick
        speed_X = x_String.toFloat();
        speed_Z = z_String.toFloat();
        break;

    case 'D':
        // dpad
        x = x_String.toInt();
        z = -1 * z_String.toInt();
        break;

    default:
        break;

    }

    speed_Rotation = speed_Rotation_String.toFloat();
    speed_Y = speed_Y_String.toFloat();

    int L_trigger = L_trigger_String.toFloat() >= 0.5f ? -1 : 0;
    int R_trigger = R_trigger_String.toFloat() >= 0.5f ? 1 : 0;

    state_Claw = R_trigger + L_trigger;

    int L_button = L_button_String.toInt();
    int R_button = R_button_String.toInt();

    verticalLock = R_button;
    rotationLock = L_button;

    int top_button = top_button_String.toInt();         // x on XBOX
    int bottom_button = bottom_button_String.toInt();   // a on XBOX

    if (top_button == 1 && last_top == 0) // rising edge
    {
        if (speedMod < 10)
        {
            speedMod++;
        }
    }
    else if (bottom_button == 1 && last_bottom == 0) 
    {
        if (speedMod > 0)
        {
            speedMod--;
        }
    }

    last_top = top_button;
    last_bottom = bottom_button;

    Serial.println(state_Claw);
}
