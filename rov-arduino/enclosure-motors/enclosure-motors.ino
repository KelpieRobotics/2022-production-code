// Import Libraries
#include <Servo.h>
// Define Pins
int PIN__CLAW_B = 9;         //
int PIN__CLAW_A = 8;         //
int PIN__FRONT_LEFT = 2;     // Front left motor = J1
int PIN__FRONT_RIGHT = 3;    // Front right motor = J2
int PIN__BACK_LEFT = 4;      // Back left motor = J3
int PIN__BACK_RIGHT = 5;     // Back right motor = J4
int PIN__LEFT_VERTICAL = 6;  // Left vertical motor = J5
int PIN__RIGHT_VERTICAL = 7; // Right vertical motor = J6

// Define Variables
String outputData;

// define outputs
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

// define data globals
float speed_X = 0.0f;        // Stores speed of X axis
float speed_Z = 0.0f;        // Stores speed of Z axis
float speed_Y = 0.0f;        // Stores speed of Y axis
float speed_Rotation = 0.0f; // Stores speed of Rotation
int state_Claw = 0;          // Stores state of Claw, 0 = open, 1 = closed
int verticalLock = 0;        // Stores vertical movement lock, 0 = unlocked, 1 = locked
int rotationLock = 0;        // Stores rotation lock, 0 = unlocked, 1 = locked

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

void setup()
{
    // Start Serial
    Serial.begin(9600);
    // Prepare Pins
    frontLeft.attach(PIN__FRONT_LEFT);
    frontRight.attach(PIN__FRONT_RIGHT);
    backLeft.attach(PIN__BACK_LEFT);
    backRight.attach(PIN__BACK_RIGHT);
    leftVertical.attach(PIN__LEFT_VERTICAL);
    rightVertical.attach(PIN__RIGHT_VERTICAL);
    pinMode(PIN__CLAW_A, OUTPUT);
    pinMode(PIN__CLAW_B, OUTPUT);
    // TODO : VERIFY if this requires a loop
    frontLeft.writeMicroseconds(1500);     // set front left motor as stopped
    frontRight.writeMicroseconds(1500);    // set front right motor as stopped
    backLeft.writeMicroseconds(1500);      // set back left motor as stopped
    backRight.writeMicroseconds(1500);     // set back right motor as stopped
    leftVertical.writeMicroseconds(1500);  // set left vertical motor as stopped
    rightVertical.writeMicroseconds(1500); // set right vertical motor as stopped
    digitalWrite(PIN__CLAW_A, LOW);
    digitalWrite(PIN__CLAW_B, LOW);
    // Serial Ready
    Serial.println("READY");
}

void loop()
{

    if (Serial.available() > 0)
    {
        String inputData = Serial.readStringUntil('\n');
        Serial.println(processData(inputData));
    }
}

String processData(String inputData)
{
    if (inputData == "TYPE")
    {
        return "MOTOR";
    }
    else
    {
        // dataFromPi, if not "MOTOR", will come in the format "firstElement\tsecondElement\tthirdElement"

        // split at tabs into a string array of 3 elements
        String dataFromPiArray[3];
        dataFromPiArray[0] = inputData.substring(0, inputData.indexOf('\t'));
        dataFromPiArray[1] = inputData.substring(inputData.indexOf('\t') + 1, dataFromPi.lastIndexOf('\t'));
        dataFromPiArray[2] = inputData.substring(inputData.lastIndexOf('\t') + 1);

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

        return "OK";
    }
}

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

    // create cartesian coordinate struct with coordinates from the x and z speeds
    CartesianCoordinates baseCoords = {1.0f * speed_X, 1.0f * speed_Z};

    // create a polar coordinate struct with the base cartesian coordinates
    PolarCoordinates basePolar = {totalSpeed, atan2(speed_Z, speed_X)};

    // rotate the basePolar struct by 1/4*PI clockwise
    basePolar.theta = (basePolar.theta > (7 / 4) * PI) ? basePolar.theta - (2.0f * PI) + (PI / 4.0f) : basePolar.theta + (PI / 4.0f);

    // create cartesian coordinate struct with the rotated polar coordinates
    CartesianCoordinates rotatedCoords = {basePolar.r * cos(basePolar.theta), basePolar.r * sin(basePolar.theta)};

    // set motor speeds with the new proper values, with the following mappings:
    // front left motor/back right motor: positive x axis
    // front right motor/back left motor: negative z axis

    int fl = 1500 + (int)(rotatedCoords.x * MOD_FRONT_LEFT);
    int fr = 1500 - (int)(rotatedCoords.z * MOD_FRONT_RIGHT);
    int bl = 1500 + (int)(rotatedCoords.x * MOD_BACK_LEFT);
    int br = 1500 - (int)(rotatedCoords.z * MOD_BACK_RIGHT);

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

    case 1:
        digitalWrite(PIN__CLAW_A, HIGH);
        digitalWrite(PIN__CLAW_B, LOW);
        break;

    case -1:
        digitalWrite(PIN__CLAW_A, LOW);
        digitalWrite(PIN__CLAW_B, HIGH);
        break;

    default:
        digitalWrite(PIN__CLAW_A, LOW);
        digitalWrite(PIN__CLAW_B, LOW);
        break;
    }
}
