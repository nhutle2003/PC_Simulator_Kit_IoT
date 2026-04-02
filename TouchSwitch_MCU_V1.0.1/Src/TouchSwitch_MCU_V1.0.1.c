/*
 * TOUCH SWITCH MCU V1.0.1
 */
#include "system_stm32f4xx.h"
#include "eventman.h"
#include "timer.h"
#include "led.h"
#include "melody.h"
#include "lightsensor.h"
#include "temhumsensor.h"
#include "eventbutton.h"
#include "button.h"
#include "Ucglib.h"
#include "uartcmd.h"
#include "serial.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define CYCLE_LED_CHANGE 20
#define PERIOD_SCAN_MULTISENSOR		1000
/****************************************************************************************/
/*                                  STRUCTs AND ENUMs                           		*/
/****************************************************************************************/
typedef enum
{
	EVENT_EMPTY,
	EVENT_APP_INIT,
	EVENT_APP_FLUSHMEM_READY
} event_api_t, *event_api_p;

typedef enum
{
	STATE_APP_STARTUP,
	STATE_APP_IDLE,
	STATE_APP_RESET
} state_app_t;

/****************************************************************************************/
/*                                  VARIABLEs                           		*/
/****************************************************************************************/

static state_app_t eCurrentState;
static ucg_t ucg;
static uint8_t ledChange = 0;
static uint8_t 		idTimerStartIncrease = NO_TIMER;
static uint8_t 		idTimerStartDecrease = NO_TIMER;
static led_color_t currentColor = LED_COLOR_GREEN;
static uint16_t 	temperature, humidity, light;
static char 		strTemp[30] = "";
static char 		strHumi[30] = "";
static char 		strLight[30] = "";
static uint8_t 		idTimerSensorUpdate = NO_TIMER;
char				text[] = "IOT Programming by Lumi Smarthome";

#define CMD_ID           byRxBuffer[2]
#define CMD_TYPE         byRxBuffer[3]
#define CMD_DATA_EPOINT  byRxBuffer[4]
#define CMD_DATA_STATE   byRxBuffer[5]
/****************************************************************************************/
/*                                      FUNCTIONs                             		    */
/****************************************************************************************/
static void SetStateApp(state_app_t state);
static void LoadConfiguration(void);
static void AppStateManager(uint8_t event);
static state_app_t GetStateApp();
void Increase_LedLevel ();
void Decrease_LedLevel ();
void MultiSensorScan ();
void Task_multiSensorScan ();
void AppInitCommon ();
void DeviceStateMachine (uint8_t event);
int Clamp (int value, int min , int max);
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
void LedHandle (uint8_t led_id, \
				uint8_t led_color, \
				uint8_t led_num_blink, \
				uint8_t led_interval, \
				uint8_t led_last_state) {
	if (led_id == LED_KIT_ID0)
		{
			if (led_last_state == LED_COLOR_RED)
			{
				LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_RED, 50);
			}
			else if (led_last_state == LED_COLOR_GREEN)
			{
				LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_GREEN, 50);
			}
			else if (led_last_state == LED_COLOR_BLUE)
			{
				LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_BLUE, 50);
			}
			else if (led_last_state == LED_COLOR_BLACK)
			{
				LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_RED, 0);
				LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_GREEN, 0);
				LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_BLUE, 0);
			}
		}
		else if (led_id == LED_KIT_ID1)
		{
			if (led_last_state == LED_COLOR_RED)
			{
				LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_RED, 50);
			}
			else if (led_last_state == LED_COLOR_GREEN)
			{
				LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_GREEN, 50);
			}
			else if (led_last_state == LED_COLOR_BLUE)
			{
				LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_BLUE, 50);
			}
			else if (led_last_state == LED_COLOR_BLACK)
			{
				LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_RED, 0);
				LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_GREEN, 0);
				LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_BLUE, 0);
			}
		}
}

void ButtonHadle (uint8_t button_id, uint8_t button_state) {
	switch (button_id)
		{
			case EVENT_OF_BUTTON_3_PRESS_5_TIMES:
			{
				//code
				ucg_ClearScreen(&ucg);
				ucg_SetFont(&ucg, ucg_font_ncenR08_hf);
				ucg_DrawString(&ucg, 0, 12, 0, "Device: Board");
				ucg_DrawString(&ucg, 0, 24, 0, "STM32 Nucleo.");
				ucg_DrawString(&ucg, 0, 36, 0, "Code: STM32F401RE_");
				ucg_DrawString(&ucg, 0, 48, 0, "NUCLEO.");
				ucg_DrawString(&ucg, 0, 60, 0, "Manufacturer:");
				ucg_DrawString(&ucg, 0, 72, 0, "STMicroelectronics.");
				ucg_DrawString(&ucg, 0, 84, 0, "Kit expansion:");
				ucg_DrawString(&ucg, 0, 97, 0, "Lumi Smarthome.");
				ucg_DrawString(&ucg, 0, 110, 0, "Project:");
				ucg_DrawString(&ucg, 0, 123, 0, "Simulator touch switch.");

				// Blink the green LED 5 times

				LedControl_BlinkStart(LED_ALL_ID, BLINK_GREEN, 10, 1000, LED_COLOR_BLACK);

				//Temp, hum, light value
				TimerStart("MultiSensorScan", 7000, 1, MultiSensorScan, NULL);
			} break;

			case EVENT_OF_BUTTON_1_PRESS_LOGIC:
			{
				LedControl_SetAllColor(LED_COLOR_RED, 50);
				LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_RED,50);
				LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_RED,50);

				BuzzerControl_SetMelody(pbeep);
				BuzzerControl_SendPacketRespond(1);
				currentColor = LED_COLOR_RED;

			} break;

			case EVENT_OF_BUTTON_2_PRESS_LOGIC:
			{
				LedControl_SetAllColor(LED_COLOR_GREEN, 50);

				LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_GREEN,50);
				LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_GREEN,50);

				BuzzerControl_SetMelody(pbeep);
				BuzzerControl_SendPacketRespond(1);
				currentColor = LED_COLOR_GREEN;
			} break;

			case EVENT_OF_BUTTON_4_PRESS_LOGIC:
			{
				LedControl_SetAllColor(LED_COLOR_WHITE, 50);

				LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_WHITE,50);
				LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_WHITE,50);

				BuzzerControl_SetMelody(pbeep);
				BuzzerControl_SendPacketRespond(1);
				currentColor = LED_COLOR_WHITE;
			} break;
			case EVENT_OF_BUTTON_5_PRESS_LOGIC:
			{
				LedControl_SetAllColor(LED_COLOR_BLUE, 50);

				LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_BLUE,50);
				LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_BLUE,50);

				BuzzerControl_SetMelody(pbeep);
				BuzzerControl_SendPacketRespond(1);
				currentColor = LED_COLOR_BLUE;
			} break;

			case EVENT_OF_BUTTON_1_HOLD_1S:
			{
				if(idTimerStartIncrease != NO_TIMER)
							{
								TimerStop(idTimerStartIncrease);
								idTimerStartIncrease = NO_TIMER;
							}

							idTimerStartIncrease = TimerStart("Increase_Led",
																CYCLE_LED_CHANGE,
																100,
																Increase_LedLevel,
																NULL);
			} break;

			case EVENT_OF_BUTTON_5_HOLD_1S:
			{
				if(idTimerStartDecrease != NO_TIMER)
							{
								TimerStop(idTimerStartDecrease);
								idTimerStartDecrease = NO_TIMER;
							}

							idTimerStartDecrease = TimerStart("Decrease_Led",
																CYCLE_LED_CHANGE,
																100,
																Decrease_LedLevel,
																NULL);
			} break;

			case EVENT_OF_BUTTON_1_RELEASED_1S:
			{
				TimerStop(idTimerStartIncrease);
			} break;

			case EVENT_OF_BUTTON_5_RELEASED_1S:
			{
				TimerStop(idTimerStartDecrease);
			} break;

			default:
				break;
		}
}
/* CMD ID + buzzer_state*/
void BuzzerHandle(uint8_t buzzer_state) {
	BuzzerControl_SetMelody(pbeeep);
	BuzzerControl_SendPacketRespond(1);
}
//CMD + text
char srcLcd[100] ={0};
void LcdHandle(char *text) {
	ucg_SetFont(&ucg, ucg_font_ncenR08_hf);
	ucg_ClearScreen(&ucg);
	//memset(srcLcd, 0);
	strcpy(srcLcd, text);
	ucg_DrawString(&ucg, 0, 12, 0, srcLcd);
}

int main ()
{
	AppInitCommon();
    // Initialize the main program with the device's previous state before power off
	SetStateApp(STATE_APP_STARTUP);

	// Register initialization events (Add EVENT_APP_INIT initialization events to the Queue)
	EventSchedulerAdd(EVENT_APP_INIT);

	while (1)
	{
		// Handle events according to the pre-set schedule
		processTimerScheduler();

		// Handle events added to the buffer (button...)
		processEventScheduler();

		//Handle information from simulator
		processSerialReceiver();
	}
}
void AppInitCommon ()
{
	// System clock configuration is 84 MHz
	SystemCoreClockUpdate();

	// Initialize timer system tick to handle time-based events
	TimerInit();
	// Register execution events (Initialize the buffer to store the program's event list)
	EventSchedulerInit(AppStateManager);

	EventSerial_Init();
	EventSerial_SetEventLedCallback(LedHandle);
	EventSerial_SetEventButtonCallback(ButtonHadle);
	EventSerial_SetEventBuzzerCallback(BuzzerHandle);
	EventSerial_SetEventLcdCallback(LcdHandle);

	// Configure GPIO pins for the push buttons on the board
	EventButton_Init();

	// Configure GPIO pin for the buzzer on the board
	BuzzerControl_Init();

	// Configure GPIO pins for the RGB LEDs on the board
	LedControl_Init();

	// Configure the ADC peripheral to operate in DMA mode to read data from the light sensor
	// on the board
	LightSensor_Init(ADC_READ_MODE_DMA);

	// Configure the I2C peripheral to communicate with the temperature and humidity sensor
	TemHumSensor_Init();

	// Configure the SPI pins of the STM32 for the LCD screen
	Ucglib4WireSWSPI_begin(&ucg, UCG_FONT_MODE_SOLID);

	// Clear the LCD screen
	ucg_ClearScreen(&ucg);

	// Set the ncenR10_hr font for display
	ucg_SetFont(&ucg, ucg_font_ncenR10_hf);

	// Set the color white for the displayed text
	ucg_SetColor(&ucg, 0, 255, 255, 255);

	// Set the color black for the display background
	ucg_SetColor(&ucg, 1, 0, 0, 0);

	// Rotate the screen 180 degrees
	ucg_SetRotate180(&ucg);
}


/*
 * @func:  		AppStateManager

 * @brief:		The function to handle the main program events

 * @param:		none

 * @retval:		None
 */

static void AppStateManager (uint8_t event)
{
	switch (GetStateApp())
	{
		case STATE_APP_STARTUP:
			if (event == EVENT_APP_INIT)
			{
				//Load configuration
				LoadConfiguration();
				Serial_SendPacket(CMD_OPT_NOT_USE, CMD_ID_LCD, CMD_TYPE_SET,(uint8_t *)text, sizeof(text));
				SetStateApp(STATE_APP_IDLE);
			}
			break;

		case STATE_APP_IDLE:
			DeviceStateMachine(event);
			break;

		case STATE_APP_RESET:
			break;

		default:
			break;
	}
}

/*
 * @func:  		SetStateApp
 * @brief:		Set state of application
 * @param:		state
 * @retval:		None
 */
static void SetStateApp (state_app_t state)
{
	// Set the program state
	eCurrentState = state;
}

/*
 * @func:  		GetStateApp
 * @brief:		Get state of application
 * @param:		None
 * @retval:		State of application
 */
static state_app_t GetStateApp ()
{
	// Return the program state
	return eCurrentState;
}



/*
 * @func:  		LoadConfiguration
 * @brief:		Display information
 * @param:		None
 * @retval:		None
 */
void LoadConfiguration ()
{
	ucg_DrawString(&ucg, 0, 40, 0, "IOT Programming");
	ucg_DrawString(&ucg, 55, 65, 0, "by");
	ucg_DrawString(&ucg, 5, 90, 0, "Lumi Smarthome");

}

/**
 * @func    DeviceStateMachine
 * @brief   State machine of the Device
 * @param   ev
 * @retval  None
 */
void DeviceStateMachine (uint8_t event)
{
	switch (event)
	{
		case EVENT_OF_BUTTON_3_PRESS_5_TIMES:
		{
			//code
			ucg_ClearScreen(&ucg);
			ucg_SetFont(&ucg, ucg_font_ncenR08_hf);
			ucg_DrawString(&ucg, 0, 12, 0, "Device: Board");
			ucg_DrawString(&ucg, 0, 24, 0, "STM32 Nucleo.");
			ucg_DrawString(&ucg, 0, 36, 0, "Code: STM32F401RE_");
			ucg_DrawString(&ucg, 0, 48, 0, "NUCLEO.");
			ucg_DrawString(&ucg, 0, 60, 0, "Manufacturer:");
			ucg_DrawString(&ucg, 0, 72, 0, "STMicroelectronics.");
			ucg_DrawString(&ucg, 0, 84, 0, "Kit expansion:");
			ucg_DrawString(&ucg, 0, 97, 0, "Lumi Smarthome.");
			ucg_DrawString(&ucg, 0, 110, 0, "Project:");
			ucg_DrawString(&ucg, 0, 123, 0, "Simulator touch switch.");

			// Blink the green LED 5 times

			LedControl_BlinkStart(LED_ALL_ID, BLINK_GREEN, 10, 1000, LED_COLOR_BLACK);

			//Temp, hum, light value
			TimerStart("MultiSensorScan", 7000, 1, MultiSensorScan, NULL);
		} break;

		case EVENT_OF_BUTTON_1_PRESS_LOGIC:
		{
			LedControl_SetAllColor(LED_COLOR_RED, 50);
			LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_RED,50);
			LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_RED,50);

			BuzzerControl_SetMelody(pbeep);
			BuzzerControl_SendPacketRespond(1);
			currentColor = LED_COLOR_RED;

		} break;

		case EVENT_OF_BUTTON_2_PRESS_LOGIC:
		{
			LedControl_SetAllColor(LED_COLOR_GREEN, 50);

			LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_GREEN,50);
			LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_GREEN,50);

			BuzzerControl_SetMelody(pbeep);
			BuzzerControl_SendPacketRespond(1);
			currentColor = LED_COLOR_GREEN;
		} break;

		case EVENT_OF_BUTTON_4_PRESS_LOGIC:
		{
			LedControl_SetAllColor(LED_COLOR_WHITE, 50);

			LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_WHITE,50);
			LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_WHITE,50);

			BuzzerControl_SetMelody(pbeep);
			BuzzerControl_SendPacketRespond(1);
			currentColor = LED_COLOR_WHITE;
		} break;
		case EVENT_OF_BUTTON_5_PRESS_LOGIC:
		{
			LedControl_SetAllColor(LED_COLOR_BLUE, 50);

			LedControl_SendPacketRespond(LED_KIT_ID0, LED_COLOR_BLUE,50);
			LedControl_SendPacketRespond(LED_KIT_ID1, LED_COLOR_BLUE,50);

			BuzzerControl_SetMelody(pbeep);
			BuzzerControl_SendPacketRespond(1);
			currentColor = LED_COLOR_BLUE;
		} break;

		case EVENT_OF_BUTTON_1_HOLD_1S:
		{
			if(idTimerStartIncrease != NO_TIMER)
						{
							TimerStop(idTimerStartIncrease);
							idTimerStartIncrease = NO_TIMER;
						}

						idTimerStartIncrease = TimerStart("Increase_Led",
															CYCLE_LED_CHANGE,
															100,
															Increase_LedLevel,
															NULL);
		} break;

		case EVENT_OF_BUTTON_5_HOLD_1S:
		{
			if(idTimerStartDecrease != NO_TIMER)
						{
							TimerStop(idTimerStartDecrease);
							idTimerStartDecrease = NO_TIMER;
						}

						idTimerStartDecrease = TimerStart("Decrease_Led",
															CYCLE_LED_CHANGE,
															100,
															Decrease_LedLevel,
															NULL);
		} break;

		case EVENT_OF_BUTTON_1_RELEASED_1S:
		{
			TimerStop(idTimerStartIncrease);
		} break;

		case EVENT_OF_BUTTON_5_RELEASED_1S:
		{
			TimerStop(idTimerStartDecrease);
		} break;

		default:
			break;
	}
}

void Increase_LedLevel ()
{
	ledChange = Clamp((int)(ledChange + 1), 0 , 100);
	LedControl_SetAllColor(currentColor, ledChange);
}

void Decrease_LedLevel ()
{
	ledChange = Clamp((int)(ledChange - 1), 0, 100);
	LedControl_SetAllColor(currentColor, ledChange);
}

int Clamp (int value, int min , int max)
{
	if(value < min) value = min;
	if(value > max) value = max;
	return value;
}

void Task_multiSensorScan()
{
	// Get temperature, humidity, and light intensity values from the sensor
	temperature = (TemHumSensor_GetTemp() / 100);
	humidity =  (TemHumSensor_GetHumi() / 100);
	light = LightSensor_MeasureUseDMAMode();

	sprintf(strTemp, "Temp = %d oC     ", temperature);
	ucg_DrawString(&ucg, 0, 40, 0, strTemp);
	TempSensor_SendPacketRespond(strTemp);

	sprintf(strHumi, "Humi = %d %%     ", humidity);
	ucg_DrawString(&ucg, 0, 65, 0, strHumi);
	HumiSensor_SendPacketRespond(strHumi);

	sprintf(strLight, "Light = %d Lux     ", light);
	ucg_DrawString(&ucg, 0, 90, 0, strLight);
	LightSensor_SendPacketRespond(strLight);
}

void MultiSensorScan ()
{
	ucg_ClearScreen(&ucg);
	ucg_SetFont(&ucg, ucg_font_ncenR10_hf);

	if (idTimerSensorUpdate != NO_TIMER)
	{
		TimerStop(idTimerSensorUpdate);
		idTimerSensorUpdate = NO_TIMER;
	}

	idTimerSensorUpdate = TimerStart("Task_multiSensorScan",
									PERIOD_SCAN_MULTISENSOR,
									TIMER_REPEAT_FOREVER,
									Task_multiSensorScan,
									NULL);
}
