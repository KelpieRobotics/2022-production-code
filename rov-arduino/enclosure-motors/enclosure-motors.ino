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

const float MOD_FRONT_LEFT = 1.0f;     // Modifier for front left motor, should be between 0 and 1
const float MOD_FRONT_RIGHT = 1.0f;    // Modifier for front right motor, should be between 0 and 1
const float MOD_BACK_LEFT = 1.0f;      // Modifier for back left motor, should be between 0 and 1
const float MOD_BACK_RIGHT = 1.0f;     // Modifier for back right motor, should be between 0 and 1
const float MOD_LEFT_VERTICAL = 1.0f;  // Modifier for left vertical motor, should be between 0 and 1
const float MOD_RIGHT_VERTICAL = 1.0f; // Modifier for right vertical motor, should be between 0 and 1

const int DEBUG_MODE = 0; // Debug mode, 0 = regular, 1 = only Y move, 2 = only X move, 3 = only Z move, 4 = only claw move

// define data globals
float speed_X = 0.0f;        // Stores speed of X axis
float speed_Z = 0.0f;        // Stores speed of Z axis
float speed_Y = 0.0f;        // Stores speed of Y axis
float speed_Rotation = 0.0f; // Stores speed of Rotation
int state_Claw = 0;          // Stores state of Claw, 0 = open, 1 = closed
int verticalLock = 0;        // Stores vertical movement lock, 0 = unlocked, 1 = locked
int rotationLock = 0;        // Stores rotation lock, 0 = unlocked, 1 = locked

int time = 0;

void MoveVertical(float speed_Y)
{
    float totalSpeed = speed_Y * MAX_SPEED;

    // set left vertical motor
    int lv = 1500 + (int)(totalSpeed * MOD_LEFT_VERTICAL);
    leftVertical.writeMicroseconds(lv);

    // set right vertical motor
    int rv = 1500 + (int)(totalSpeed * MOD_RIGHT_VERTICAL);
    rightVertical.writeMicroseconds(rv);
}

void MoveHorizontal(float speed_X, float speed_Z)
{
    float totalSpeed = MAX_SPEED * sqrt(pow(speed_X, 2) + pow(speed_Z, 2)) / sqrt(2.0f);

//    Serial.print("Total speed: ");
//    Serial.print(totalSpeed);

    // create cartesian coordinate struct with coordinates from the x and z speeds
    CartesianCoordinates baseCoords = {1.0f * speed_X, 1.0f * speed_Z};

    // create a polar coordinate struct with the base cartesian coordinates
    PolarCoordinates basePolar = {totalSpeed, atan2(speed_Z, speed_X)};

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

    int fl = 1500 + (int)(rotatedCoords.x * MOD_FRONT_LEFT);
    int fr = 1500 - (int)(rotatedCoords.z * MOD_FRONT_RIGHT);
    int bl = 1500 + (int)(rotatedCoords.x * MOD_BACK_LEFT);
    int br = 1500 - (int)(rotatedCoords.z * MOD_BACK_RIGHT);

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

void Rotate(float rotationSpeed)
{
    float totalSpeed = rotationSpeed * MAX_SPEED;

    // set front left motor
    frontLeft.writeMicroseconds(1500 + (totalSpeed * MOD_FRONT_LEFT));

    // set front right motor
    frontRight.writeMicroseconds(1500 - (totalSpeed * MOD_FRONT_RIGHT));

    // set back left motor
    backLeft.writeMicroseconds(1500 + (totalSpeed * MOD_BACK_LEFT));

    // set back right motor
    backRight.writeMicroseconds(1500 - (totalSpeed * MOD_BACK_RIGHT));
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
     Serial.println(state);
}

void setup()
{
    // start serial
    Serial.begin(9600);

    // initialize servos
    pinMode(PIN__CLAW_1, OUTPUT);
    pinMode(PIN__CLAW_2, OUTPUT);
    
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

            if (rotationLock == 0)
                Rotate(speed_Rotation);

            MoveHorizontal(speed_X, speed_Z);
            SetClawState(state_Claw);
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

    // dataFromPi, if not "MOTOR", will come in the format "firstElement\tsecondElement\tthirdElement"

    // split at tabs into a string array of 3 elements
    String dataFromPiArray[3];
    dataFromPiArray[0] = dataFromPi.substring(0, dataFromPi.indexOf('\t'));
    dataFromPiArray[1] = dataFromPi.substring(dataFromPi.indexOf('\t') + 1, dataFromPi.lastIndexOf('\t'));
    dataFromPiArray[2] = dataFromPi.substring(dataFromPi.lastIndexOf('\t') + 1);
    
    // the first element will come in the format of "speed_X, speed_Z"
    // the second element will come in the format of "speed_Rotation, speed_Y"
    // the third element will come in the format of "L_bumper, L_trigger, R_bumper, R_trigger"

    String speed_X_String = dataFromPiArray[0].substring(0, dataFromPiArray[0].indexOf(','));
    String speed_Z_String = dataFromPiArray[0].substring(dataFromPiArray[0].indexOf(',') + 1);

    // parse the second element
    String speed_Rotation_String = dataFromPiArray[1].substring(0, dataFromPiArray[1].indexOf(','));
    String speed_Y_String = dataFromPiArray[1].substring(dataFromPiArray[1].indexOf(',') + 1);

    // parse the third element
    String tmp_str = dataFromPiArray[2];

    String L_bumper_String = tmp_str.substring(0, tmp_str.indexOf(','));

    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String L_trigger_String = tmp_str.substring(0, tmp_str.indexOf(','));

    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String R_bumper_String = tmp_str.substring(0, tmp_str.indexOf(','));

    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String R_trigger_String = tmp_str;

    // lock state variables
    int R_bumper = R_bumper_String.toInt();
    int R_trigger = R_trigger_String.toFloat() >= 0.5f ? 1 : 0;

    // claw state variables
    int L_bumper = L_bumper_String.toInt();
    int L_trigger = L_trigger_String.toFloat() >= 0.5f ? -1 : 0;

    // set the lock states based on the bumper values
    verticalLock = R_bumper;
    rotationLock = L_bumper;

    // set the variables to the new values
    speed_X = speed_X_String.toFloat();
    speed_Z = speed_Z_String.toFloat();
    speed_Y = speed_Y_String.toFloat();
    speed_Rotation = speed_Rotation_String.toFloat();
    state_Claw = R_trigger + L_trigger; // -1 = open, 0 = stopped, 1 = closed
//    Serial.println("OK");

//    Serial.print(speed_X);
//    Serial.print(speed_Y);
//    Serial.print(speed_Z);
//    Serial.print(state_Claw);
}