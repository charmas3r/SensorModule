#include "mbed.h"
#include <string>
#include "pindef.h"
#include "math.h"

//mbed interface variables
I2C temp_sensor(I2C_SDA, I2C_SCL);
Serial pc(UART_TX, UART_RX);
PwmOut fan_one(PA_9);
PwmOut speaker(PA_10);
AnalogIn pitch(PA_0);
DigitalOut led1(PA_8);

//threading variables
Thread thread_one;
Thread thread_two;
Thread thread_three;
Thread thread_four;
Thread thread_five;
Mutex mutex;

//functions
void display_main_menu();
void main_menu_input();
void display_current_temp();
void return_to_menu_input();
void uart_controller();
void set_temp_to_celsius();
void set_temp_to_fah();
void set_control_temp();
void set_alarm_variance();
void set_alarm_volume();
void set_led_colors();
void display_current_prefs();
float convert_to_fah(float temp_in_cel);
void start_fan();
void start_led();
void display_preferences();
void start_speaker();


//variables
bool authorization = false;
bool isCelsius = true;
string username = "";
string password = "";
float current_temp_f;
float current_temp_c;
float control_temp = 25.1f;
float alarm_variance = 5.0f;
int alarm_volume;

enum COLORS {RED, BLUE, GREEN, WHITE};

//constants
const string USERNAME = "admin";
const string PASSWORD = "password";
const int PERIOD = 46;
const int ONE = 49;
const int NINE = 57;
const int DIGIT_OFFSET = 48;
const int BUFFER_SIZE = 50;
const float DEFAULT_FAN_SPEED = .25;
const float FAN_MULTIPLIER = .5;
const int DEFAULT_FAN_STOP = 0;
const int DEFUALT_LIGHT_OFF = 0;
const int DEFAULT_SPEAKER_ON = 0.99;
const int DEFAULT_SPEAKER_OFF = 0;
const float LED_TIMEOUT = .5;
const float CEL_TO_FAH_QUOTIENT = 9/5;
const float FAH_OFFSET = 32;
const float SHORT_WAIT = 0.1f;
const int POWER_TW0 = 2;
const int POWER_THREE = 3;
const int VARIABLE_SPEED_DENOM = 100000;
const float WARNING_FREQ = 1/2000;
const float ALARM_FREQ = 1/6000;

/*
* This function sets the variance level from the control tempature. When the threshold variance
* is exceeded it will trigger an alarm.
*/
void set_alarm_variance()
{
    mutex.lock();
    pc.printf("The current alarm variance is: %.2f%\r\n", alarm_variance);
    pc.printf("Enter new alarm variance: ");
    int user_selection;
    char buffer[BUFFER_SIZE];
    int i = 0;
    while (1) {
        char c = pc.getc();
        if (c == '\r') {
            pc.printf("\r\n");
            break;
        } else if (c == '\b') {
            //not sure about backspace?
        } else if (c >= ONE && c <= NINE) {
            user_selection = ((int) c - DIGIT_OFFSET);
            pc.printf("%d", user_selection);
            buffer[i++] = c;
        } else if (c == PERIOD) {
            buffer[i++] = c;
            pc.printf("%c", c);
        }
    }
    alarm_variance = (float)atof(buffer);
    pc.printf("Alarm variance is now set to: %.2f%\r\n", alarm_variance);
    mutex.unlock();
    display_main_menu();
}


void set_alarm_volume()
{

}


/*
* This function handles client validation of username and password to hardcoded user/pass on board
*/
bool validate_account(string username, string password)
{
    return ((username.compare(USERNAME) == 0) && (password.compare(PASSWORD) == 0));
}


/*
* This function displays temperature in either Celsius or Fahrenheit (default Celsius).
*/
void display_current_temp()
{
    mutex.lock();
    if (isCelsius) {
        pc.printf("Current Temp: %.2f degrees Celsius \r\n\n", current_temp_c);
    } else {
        pc.printf("Current Temp: %.2f degrees Fahrenheit \r\n\n", current_temp_f);
    }
    mutex.unlock();
    display_main_menu();
}


/*
* This function sets control temperature. This temperature is the target for the device
* to stay within range of. For example, if the control temperature is set to 70 degrees
* Farenheit then the board will try to stay within range of it (variance is also based on user).
* If the device will give an auditory and led indication.
*/
void set_control_temp()
{
    mutex.lock();
    if (isCelsius) {
        pc.printf("The control temperature is currently set to %.2f degrees Celsius \r\n", control_temp);
    } else {
        pc.printf("The control temperature is currently set to %.2f degrees Fahrenheit \r\n", convert_to_fah(control_temp));
    }
    pc.printf("Enter new control temperature: ");
    int user_selection;
    char buffer[BUFFER_SIZE];
    int i = 0;
    while (1) {
        char c = pc.getc();
        if (c == '\r') {
            pc.printf("\r\n");
            break;
        } else if (c == '\b') {
            //not sure about backspace?
        } else if (c >= ONE && c <= NINE) {
            user_selection = ((int) c - DIGIT_OFFSET);
            pc.printf("%d", user_selection);
            buffer[i++] = c;
        } else if (c == PERIOD) {
            buffer[i++] = c;
            pc.printf("%c", c);
        }
    }
    control_temp = (float)atof(buffer);
    pc.printf("Control temperature is now set to %f\r\n", control_temp);
    mutex.unlock();
    display_main_menu();
}


/*
* This function sets temp to celsius
*/
void set_temp_to_celsius()
{
    mutex.lock();
    if(!isCelsius) {
        isCelsius = true;
        pc.printf("Temperature set to Celsius\r\n");
    } else {
        pc.printf("Temperature already set to Celsius\r\n");
    }
    mutex.unlock();
    display_main_menu();
}


/*
* This function sets temp to fahrenheit
*/
void set_temp_to_fah()
{
    mutex.lock();
    if(isCelsius) {
        isCelsius = false;
        pc.printf("Temperature set to Fahrenheit\r\n");
    } else {
        pc.printf("Temperature already set to Fahrenheit\r\n");
    }
    mutex.unlock();
    display_main_menu();
}


/*
* This returns user to main menu if they press '9'
*/
void return_to_menu_input()
{
    int user_selection;
    mutex.lock();
    while (1) {
        char c = pc.getc();
        if (c == '\r') {
            pc.printf("\r\n");
            break;
        } else if (c == '\b') {
            //not sure about backspace?
        } else if (c == NINE) {
            user_selection = ((int) c - DIGIT_OFFSET);
            pc.printf("%d\r\n", user_selection);
            break;
        } else {
            pc.printf("Invalid input, try again\r\n");
            pc.printf("Enter selection: \r\n");
        }
    }
    mutex.unlock();
    if (user_selection == 9) {
        display_main_menu();
    }

}


/*
* This function displays and controls menu settings by allowing client to interface with board
* and modify temperature and general settings via UART.
*/
void display_main_menu()
{
    mutex.lock();
    pc.printf("============================================\r\n");
    pc.printf("      Temperature Settings Main Menu        \r\n");
    pc.printf("                                            \r\n");
    pc.printf("1. Read current temperature                 \r\n");
    pc.printf("2. Set temperature to Celsius               \r\n");
    pc.printf("3. Set temperature to Fahrenheit            \r\n");
    pc.printf("4. Set control temperature                  \r\n");
    pc.printf("5. Set alarm variance                       \r\n");
    pc.printf("6. Current preferrences                     \r\n");
    pc.printf("                                            \r\n");
    mutex.unlock();
    main_menu_input();
}


void main_menu_input()
{
    int user_selection;
    mutex.lock();
    pc.printf("Enter selection: ");
    while (1) {
        char c = pc.getc();
        if (c == '\r') {
            pc.printf("\r\n");
            break;
        } else if (c == '\b') {
            //not sure about backspace?
        } else if (c >= ONE && c <= NINE) {
            user_selection = ((int) c - DIGIT_OFFSET);
            pc.printf("%d\r\n", user_selection);
            break;
        } else {
            pc.printf("Invalid input, try again\r\n");
        }
    }
    mutex.unlock();
    switch(user_selection) {
        case 1:
            display_current_temp();
            break;
        case 2:
            set_temp_to_celsius();
            break;
        case 3:
            set_temp_to_fah();
            break;
        case 4:
            set_control_temp();
            break;
        case 5:
            set_alarm_variance();
            break;
        case 6:
            display_preferences();
            break;
        case 9:
            break;
    }
}


void display_preferences()
{
    mutex.lock();
    pc.printf("                                             \r\n");
    pc.printf("           Current Preferences               \r\n");
    pc.printf("                                             \r\n");
    pc.printf("Username: %s\r\n", username);
    if (isCelsius) {
        pc.printf("Temperature Setting: Celsius\r\n");
        pc.printf("Control Temperature (Celsius): %.2f\r\n", control_temp);

    } else {
        pc.printf("Temperature Setting: Fahrenheit\r\n");
        pc.printf("Control Temperature (Fahrenheit): %.2f\r\n", control_temp);
    }
    pc.printf("Alarm variance: %.2f\r\n", alarm_variance);
    //hardcoding some details for now
    pc.printf("Motors: 1\r\n");
    pc.printf("Temperature sensors: 1\r\n");
    pc.printf("Account validation: True\r\n\n");
    mutex.unlock();
    display_main_menu();
}


/*
* This handles communication and with client via and univeral asynchronous reciever/transmitter.
*/
void uart_controller()
{
    mutex.lock();
    pc.printf("                                             \r\n");
    pc.printf("@@@@@@@  @@@@@@ @@@  @@@ @@@@@@  @@@@@@@     \r\n");
    pc.printf("!@@      !@@     @@@  @@@     @@! !@@        \r\n");
    pc.printf("!@!       !@@!!  @!@!@!@!  @!!!:  !!@@!!     \r\n");
    pc.printf(":!!          !:!      !!!     !!:     !:!    \r\n");
    pc.printf(":: :: : ::.: :       : : ::: ::  :: : :      \r\n");
    pc.printf("                                             \r\n");
    pc.printf("     Evan Smith & Forrest Joy Dec. 2019      \r\n\n");
    mutex.unlock();
    do {
        mutex.lock();
        pc.printf("Username: ");
        while (1) {
            char c = pc.getc();
            if (c == '\r') {
                pc.printf("\r\n");
                break;
            } else if (c == '\b') {
                //not sure about backspace?
            } else {
                pc.printf("%c", c);
                username += c;
            }
        }
        pc.printf("Password: ");
        while (1) {
            char c = pc.getc();
            if (c == '\r') {
                pc.printf("\r\n");
                break;
            } else if (c == '\b') {
                //not sure about backspace?
            } else {
                pc.printf("*", c);
                password += c;
            }
        }
        bool status = validate_account(username, password);
        if (status) {
            authorization = true;
            pc.printf("Successfully logged in.\r\n\n");
        } else {
            pc.printf("Unsuccessful, try again.\r\n\n");
        }
        username = "";
        password = "";
        mutex.unlock();
    } while (authorization == false);

    display_main_menu();
}

/*
* This function starts the led when the temperature associated with the alarm variance threshold
* is broached. The warning lights turn off once the threshold temperature is recovered by the fans.
*/
void start_led()
{
    float temp_variance = 0;
    float target_temp_c = 0;
    float target_temp_f = 0;
    while(1)
    {
        temp_variance = control_temp * (alarm_variance/100);
        target_temp_c = control_temp - temp_variance;
        target_temp_f = convert_to_fah(target_temp_c);
        if (isCelsius) {
            if (current_temp_c >= control_temp+0.2) {
                led1 = !led1;
            } else {
                led1 = DEFUALT_LIGHT_OFF;
            }
        } else {
            if (current_temp_f >= target_temp_f) {
                led1 = !led1;
            } else {
                led1 = DEFUALT_LIGHT_OFF;
            }
        }
        wait(LED_TIMEOUT);
    }
}



/*
* This function starts the fan when the alarm threshold is reached and stops the fan when
* current temp drops under the threshold. The threshold here is the control temperature set
* by the user.
*/
void start_fan()
{
    double temp_ratio_c;
    while(1)
    {
        temp_ratio_c = pow(current_temp_c, POWER_THREE)/control_temp;
        if (current_temp_c >= control_temp) {
            fan_one = (DEFAULT_FAN_SPEED + pow(DEFAULT_FAN_SPEED*(temp_ratio_c), POWER_TW0) + FAN_MULTIPLIER) / VARIABLE_SPEED_DENOM;
        } else {
            fan_one = DEFAULT_FAN_STOP;
        }
    }
}
void start_speaker()
{
    float target_temp;
    float temp_variance;
    while(1)
    {
        temp_variance = control_temp * (alarm_variance/100);
        target_temp = control_temp - temp_variance;
        if (current_temp_c >= target_temp && current_temp_c < control_temp) {
            speaker.period(WARNING_FREQ);
            speaker = DEFAULT_SPEAKER_ON;
            wait(SHORT_WAIT);
            speaker = DEFAULT_SPEAKER_OFF;
            wait(SHORT_WAIT);
        } else if (current_temp_c >= target_temp && current_temp_c > control_temp) {
            speaker.period(ALARM_FREQ);
            speaker = DEFAULT_SPEAKER_ON;
        } else {
            speaker = DEFAULT_SPEAKER_OFF;
        }
    }
}


float convert_to_fah(float temp_in_cel)
{
    return (current_temp_c * CEL_TO_FAH_QUOTIENT + FAH_OFFSET);
}

void read_temp()
{
    const int temp_addr = 0x90;
    char cmd[] = {0x51, 0xAA};
    char read_temp[2];
    while(1)
    {
        cmd[0] = 0x51;
        cmd[1] = 0xAA;
        temp_sensor.write(temp_addr, cmd, 1);
        wait(SHORT_WAIT);
        temp_sensor.write(temp_addr, &cmd[1], 1);
        wait(SHORT_WAIT);
        temp_sensor.read(temp_addr, read_temp, 2);
        current_temp_c = (float((read_temp[0] << 8) | read_temp[1]) / 256.0);
        current_temp_f = convert_to_fah(current_temp_c);
    }
}

int main()
{
    thread_one.start(uart_controller);
    thread_two.start(read_temp);
    thread_three.start(start_fan);
    thread_four.start(start_led);
    //thread_five.start(start_speaker);
}


