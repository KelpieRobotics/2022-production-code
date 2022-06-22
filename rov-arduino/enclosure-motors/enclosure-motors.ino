#include <Arduino.h>

// define pins

int PIN_X = 9;
int PIN_Y = 10;
int PIN_ZL = 11;
int PIN_ZR = 3;
int PIN_C1 = 7;
int PIN_C2 = 8;

// ESC constants; speed range is +/- 40 from 188, but we're using +/- 30 to be within current maximums

const float maxSpeed = 35;
const int stepSize = 7; // the max amount we can move per instance
const int arm = 188;    // value we need to send to stand still, or arm the ESC
const int maxRev = arm - maxSpeed; // max reverse speed
const int maxFwd = arm + maxSpeed; // max forward speed

// current speed

int x = 188;
int y = 188;
int zl = 188;
int zr = 188;

// target speed

int target_x = 188;
int target_y = 188;
int target_zl = 188;
int target_zr = 188;

// joystick reads

float scan_x = 0;
float scan_y = 0;
float scan_z = 0;
int scan_xd = 0;
int scan_zd = 0;
float scan_rotation = 0.0f;

// claw state; -1 = closing, 0 = still, 1 = opening

int claw_state = 0;

// movement locks

int verticalLock = 0; // Stores vertical movement lock, 0 = unlocked, 1 = locked
int rotationLock = 0; // Stores rotation lock, 0 = unlocked, 1 = locked

// stick/DPad-specific variables

char stick_or_pad;
int speedMod = 10;

// debug variables

bool hasReceivedSerial = false;
int DEBUG_MODE = 0;

int getNextSpeed(int currentSpeed, int targetSpeed)
{
    if (currentSpeed - targetSpeed > stepSize)
    {
        return currentSpeed - stepSize;
    }
    else if (targetSpeed - currentSpeed > stepSize)
    {
        return currentSpeed + stepSize;
    }
    else
    {
        return targetSpeed;
    }
}

void MoveVertical(float _target_Y)
{
    target_y = arm + (int)(_target_Y * maxSpeed);
}

void MoveHorizontalDPad(int dpad_x, int dpad_z)
{
    float totalSpeedX = maxSpeed;
    float totalSpeedZ = maxSpeed;

    // hard-coding these commands is kinda painful and I don't like the idea of doing it, but we kinda have

    int cartesianToNumpad = (5) + dpad_x + (dpad_z * 3);

    switch (cartesianToNumpad)
    {
    case 1:
        totalSpeedX *= -1.0f;
        totalSpeedZ *= 0.0f;
        break;
    case 2:
        totalSpeedX *= -1.0f;
        totalSpeedZ *= -1.0f;
        break;
    case 3:
        totalSpeedX *= 0.0f;
        totalSpeedZ *= -1.0f;
        break;
    case 4:
        totalSpeedX *= -1.0f;
        totalSpeedZ *= 1.0f;
        break;
    case 5:
        totalSpeedX *= 0.0f;
        totalSpeedZ *= 0.0f;
        break;
    case 6:
        totalSpeedX *= 1.0f;
        totalSpeedZ *= -1.0f;
        break;
    case 7:
        totalSpeedX *= 0.0f;
        totalSpeedZ *= 1.0f;
        break;
    case 8:
        totalSpeedX *= 1.0f;
        totalSpeedZ *= 1.0f;
        break;
    case 9:
        totalSpeedX *= 1.0f;
        totalSpeedZ *= 0.0f;
        break;
    }

    target_x = arm + (int)(totalSpeedX);
    target_zl = arm + (int)(totalSpeedZ);
    target_zr = arm - (int)(totalSpeedZ);
}

void MoveHorizontalStick(float stick_x, float stick_z)
{
    float totalSpeedX = stick_x * maxSpeed;
    float totalSpeedZ = stick_z * maxSpeed;

    target_x = arm + (int)(totalSpeedX);
    target_zl = arm + (int)(totalSpeedZ);
    target_zr = arm + (int)(totalSpeedZ);
}

void Rotate(float rotationSpeed)
{
    int totalSpeed = (int)(rotationSpeed * maxSpeed);

    target_zl = arm + totalSpeed;
    target_zr = arm + totalSpeed;
}

void SetClawState(int state)
{
    switch (state)
    {

    case 1: // open
        digitalWrite(PIN_C1, LOW);
        digitalWrite(PIN_C2, HIGH);
        break;

    case -1: // close
        digitalWrite(PIN_C1, HIGH);
        digitalWrite(PIN_C2, LOW);
        break;

    default: // stop
        digitalWrite(PIN_C1, LOW);
        digitalWrite(PIN_C2, LOW);
        break;
    }
    //  Serial.println(state);
}

void setup()
{
    Serial.begin(9600);

    pinMode(PIN_X, OUTPUT);
    pinMode(PIN_Y, OUTPUT);
    pinMode(PIN_ZL, OUTPUT);
    pinMode(PIN_ZR, OUTPUT);
    pinMode(PIN_C1, OUTPUT);
    pinMode(PIN_C2, OUTPUT);

    // set all motors to stand still
    analogWrite(PIN_X, arm);
    analogWrite(PIN_Y, arm);
    analogWrite(PIN_ZL, arm);
    analogWrite(PIN_ZR, arm);

    SetClawState(0);
    SetClawState(1);

    delay(3000);

    Serial.println("READY");
    SetClawState(0);
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
    String L_trigger_String = tmp_str.substring(0, tmp_str.indexOf(','));
    tmp_str = tmp_str.substring(tmp_str.indexOf(',') + 1);
    String R_button_String = tmp_str.substring(0, tmp_str.indexOf(','));
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
        scan_x = x_String.toFloat();
        scan_z = z_String.toFloat();
        break;

    case 'D':
        // dpad
        scan_xd = -x_String.toInt();
        scan_zd = z_String.toInt();
        break;

    default:
        break;
    }

    scan_rotation = -1.0f * (float) ((int)(speed_Rotation_String.toFloat() * 100.0f)) / 100.0f;
    scan_y = -1.0f * speed_Y_String.toFloat();

    int L_trigger = L_trigger_String.toFloat() >= 0.5f ? -1 : 0;
    int R_trigger = R_trigger_String.toFloat() >= 0.5f ? 1 : 0;

    claw_state = R_trigger + L_trigger;

    int L_button = L_button_String.toInt();
    int R_button = R_button_String.toInt();

    verticalLock = R_button;
    rotationLock = L_button;

    int top_button = top_button_String.toInt();       // x on XBOX
    int bottom_button = bottom_button_String.toInt(); // a on XBOX

    Serial.println("OK");
}

void mainLoop()
{
    // vertical moves if not locked...
    if (verticalLock == 0)
        MoveVertical(scan_y);

    // then rotation checks if it's large enough of a value to justify rotation

    switch (stick_or_pad)
        {
        case 'S':
            // stick
            MoveHorizontalStick(scan_x, scan_z);
            break;

        case 'D':
            // dpad
            MoveHorizontalDPad(scan_xd, scan_zd);
            break;

        default:
            break;
        }
    
    if ((scan_rotation >= 0.2f || scan_rotation <= -0.2f) && rotationLock == 0)
        Rotate(scan_rotation);

    // claw
    SetClawState(claw_state);

    // perform steps for each thruster
    x = getNextSpeed(x, target_x);
    zl = getNextSpeed(zl, target_zl);
    zr = getNextSpeed(zr, target_zr);
    y = getNextSpeed(y, target_y);

    // set the motor speeds
    analogWrite(PIN_X, x);
    analogWrite(PIN_ZL, zl);
    analogWrite(PIN_ZR, zr);
    analogWrite(PIN_Y, y);
}

void loop()
{
    if (Serial.available() > 0)
    {
        hasReceivedSerial = true;
        String inputData = Serial.readStringUntil('\n');
        ParseCommands(inputData);
    }

    if (DEBUG_MODE == 0 && hasReceivedSerial)
        mainLoop();

    delay(50);
}