/*
 * qst_log_v0.2.h
 *
 *  Created on: 28.05.2018
 *      Author: KustovAV
 */



#ifndef QST_LOG_H_
#define QST_LOG_H_

#include <../uos/runtime/mips/types.h>

#define ALLIGNED_MOD __attribute__ ((aligned(4)))

typedef enum{		//перечень ретернов
	SUCCSES = 0,
	NOTHING2SEND,
	MALLOC_FAILED,
	BAD_PARAM,
	TEST_ERR
}LOG_RES;

typedef enum{		//c/без таймером
	NOTIME 	= 0,
	TIME 	= 1,
}TimeOpt_t;


typedef enum{			//характеризует событие
	OCH_ADD= 0,			//LogTTAs_t

/*TEST ENUMS*/
	TEST0, 			//LogT_t
	TEST0_TI,			//LogTT_t

	TEST1,				//LogTA_t
	TEST1_TI,			//LogTTA_t

	TEST2,				//LogTAs_t
	TEST2_TI,			//LogTTAs_t

	TEST3,				//LogTD_t
	TEST3_TI,			//LogTTD_t




	ALL					= 13371337
}Event_t;

typedef struct {
	uint32_t * BankStart;
	uint32_t * BankEnd;
}ALLIGNED_MOD MemTable_t;






/*ПРОТОТИПЫ ФУНКЦИЙ ЛОГИРОВАНИЯ И СТРУКТУРЫ ДАННЫХ*/

void LogPrintRam(uint32_t log_count, ...);										//печать логов

void LogHelp(void);
void LogPrintState(void);														//печать состояния памяти системы
void TEST_LOG_SYSTEM(void);

LOG_RES LogSend(LOG_RES(*SendData)(uint32_t size, uint32_t * data));															//отправка логов в МД
LOG_RES LogPoint_Type(Event_t e, bool_t TimeOption);							//для логирования события c(без) временем(ни)
/*__________________________________________________________________________________________________________________________________________*/
typedef  struct {	//8 байт, time TRUE
	uint16_t type;
	uint32_t time;
}ALLIGNED_MOD LogTT_t;
typedef  struct {	//4 байт, time FALSE
	uint16_t type;
}ALLIGNED_MOD LogT_t;
/*********************************************************************************************************************************************/



LOG_RES LogPoint_TypeArg(Event_t e, bool_t TimeOption, uint32_t arg);				//для логирования события, c/без времени, аргумента
/*__________________________________________________________________________________________________________________________________________*/
typedef struct {		//12 байт, time TRUE
	uint16_t type;
	uint32_t time;
	uint32_t arg;
}ALLIGNED_MOD LogTTA_t;
typedef struct {		//8 байт, time FALSE
	uint16_t type;
	uint32_t arg;
}ALLIGNED_MOD LogTA_t;
/*********************************************************************************************************************************************/



LOG_RES LogPoint_TypeArgs(Event_t e, bool_t TimeOption, uint32_t arg_count, ...);	//для логирования события, c/без времени, и нескольких аргументов
/*__________________________________________________________________________________________________________________________________________*/
typedef struct {		//12 байт, time TRUE
	uint16_t type;
	uint32_t time;
	uint32_t arg_count;
}ALLIGNED_MOD LogTTAs_t;
typedef struct {		//8 байт, time FALSE
	uint16_t type;
	uint32_t arg_count;
}ALLIGNED_MOD LogTAs_t;
/*********************************************************************************************************************************************/



LOG_RES LogPoint_TypeData(Event_t e, bool_t TimeOption, void * ptr_to_data, uint32_t size);	//для логирования события, времени события и массива данных
/*__________________________________________________________________________________________________________________________________________*/
typedef struct {		//12 байт , time TRUE
	uint16_t type;
	uint32_t time;
	uint32_t size;//в байтах
}ALLIGNED_MOD LogTTD_t;
typedef struct {		//8 байт, time FALSE
	uint16_t type;
	uint32_t size;//в байтах
}ALLIGNED_MOD LogTD_t;
/*********************************************************************************************************************************************/





#endif QST_LOG_H_


/*!!SEE SOON!!
ПОДСИСТЕМА ЛОГГИРОВАНИЯ РАБОТЫ ИНТЕРФЕЙСОВ
typedef struct{
	uint8_t flag0;
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;
	uint8_t flag4;
	uint8_t flag5;
}InterfaceStruct_t;

typedef struct{
	uint8_t type;
	InterfaceStruct_t InterfaceStruct;
}InterfaceLogStruct_t;

ОДНА ОБЩАЯ СТРУКТУРА, МОЖНО ПЕРЕДАВАТЬ ЕЕ.
typedef struct
{
	uint16_t type;
	uint16_t text_code;

	uint32_t arg;

	uint32_t size;
	InterfaceStruct_t InterfaceStruct;
}LogStruct_t;
*/


