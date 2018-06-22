/*
 * qst_log.c
 *
 *  Created on: 28.05.2018
 *      Author: KustovAV
 */
#include "qst_log_v0.2.h"
#include "smo_typ.h"
#include "ochfunc.h"
#include "timer/timer.h"
#include <mem/mem.h>
#include "print_func.h"
#include "send_func.h"

#define OTL_PRINT_LOG 	0


/*Конфигурация системы выделения памяти*/
#define LOG_BANK_COUNT 	5				//количество банков выделяемой памяти
#define LOG_BANK_SIZE 	51200				//количество выделяемой памяти на банк, в байтах. Далее - выделяемая память = динамика.



extern mem_pool_t pmr_pool;

MemTable_t MemTab[LOG_BANK_COUNT] 				= {};	//массив структур, содержащий адресы начал и концов действительных данных банков динамик.
static uint8_t num_of_log_part 					= 0;	//индекс банка динамики размером LOG_BANK_SIZE
static uint32_t * curr_mem_ptr 					= 0;	//текущий указатель на динамику
static uint32_t * mem_border[LOG_BANK_COUNT] 	= {};	//массив указателей на границу банков динамики
static LOG_RES MemAdd()
{
	/*ПОПЫТКА ЗАНЯТЬ У ПУЛА 50кб*/
	COLORPRNT_RED(); debug_printf("\nЗАДЕЙСТВУЕТСЯ %u БАНК!", num_of_log_part); COLORPRNT_RESET();
	curr_mem_ptr = MemTab[num_of_log_part].BankStart = mem_alloc(&pmr_pool, LOG_BANK_SIZE);
	if(curr_mem_ptr == 0)	return MALLOC_FAILED;

	/*ВЫЧИСЛЕНИЕ ГРАНИЦЫ ДИНАМИКИ*/
	mem_border[num_of_log_part] = (uint32_t*)((uint32_t)MemTab[num_of_log_part].BankStart + (uint32_t)LOG_BANK_SIZE);
#if OTL_PRINT_LOG
	debug_printf("\nНачало - %#x\nКонец - %#x.",MemTab[num_of_log_part].BankStart, mem_border[num_of_log_part]);
#endif
	num_of_log_part++;
	return SUCCSES;
}
static void PrintOchName(uint32_t number)
{
	switch(number)
	{
	case 0:
		debug_printf("\tМА\t");		break;
	case 1:
		debug_printf("\tММ\t");		break;
	case 2:
		debug_printf("\tОД\t");		break;
	case 3:
		debug_printf("\tReg\t");	break;
	case 4:
		debug_printf("\tRScom(4)\t");	break;
	case 5:
		debug_printf("\tрез.\t");	break;
	case 6:
		debug_printf("\tФЛЕШ\t");	break;
	case 7:
		debug_printf("\tКИА\t");	break;
	case 8:
		debug_printf("\tHz(8)\t");	break;
	case 9:
		debug_printf("\tUART\t");	break;
	case 10:
		debug_printf("\tTest\t");	break;
	case 11:
		debug_printf("\tТЕСТ ТУ\t");break;
	case 12:
		debug_printf("\tUART_1\t"); break;
	case 13:
		debug_printf("\tDbg_proto(13)\t"); break;
	case 14:
		debug_printf("\tUART_2\t"); break;
	case 15:
		debug_printf("\tБаланс\t");	break;
	}
}

static void PrintEventName(Event_t e)
{
	switch(e)
	{
	case 0:
		debug_printf("ОЧЕРЕДИ");	break;
	case 1:
		debug_printf("ТЕСТ0");		break;
	case 2:
		debug_printf("ТЕСТ0+ВРЕМЯ");break;
	case 3:
		debug_printf("ТЕСТ1");		break;
	case 4:
		debug_printf("ТЕСТ1+ВРЕМЯ");break;
	case 5:
		debug_printf("ТЕСТ2");		break;
	case 6:
		debug_printf("ТЕСТ2+ВРЕМЯ");break;
	case 7:
		debug_printf("ТЕСТ3");		break;
	case 8:
		debug_printf("ТЕСТ3+ВРЕМЯ");break;
	case 9:
		debug_printf("");			break;
	case 10:
		debug_printf("");			break;
	case 11:
		debug_printf("");			break;
	case 12:
		debug_printf("");			break;
	case 13:
		debug_printf("");			break;
	case 14:
		debug_printf("");			break;
	case 13371337:
		debug_printf("ВСЕ ЛОГИ");	break;
	}
}

/*************************************************************************************************************************************************
* Имя функции	 :	LogPoint_Type
* Описание	 :	Функция для документирования определенного события с(без) временем(ни).
* Аргументы      :	Event_t e		- документируемое событие,
* 			bool_t TimeOption 	- лог с учетом времени,
* Возвращает     :	SUCCSES 		- документирование прошло успешно,
* 			MALLOC_FAILED 		- пул не выделил память,
*			BAD_PARAM 		- ошибка входных параметров.
*************************************************************************************************************************************************/
LOG_RES LogPoint_Type(Event_t e,  bool_t TimeOption)
{
	if(e < 0  ||  e > 65535) 					return BAD_PARAM;
	if(num_of_log_part == 0)	if(MemAdd()) 	return MALLOC_FAILED;	//эта ветка работает только при самом первом пуске.
	if(TimeOption == TIME)
	{
		if((uint8_t*)curr_mem_ptr+sizeof(LogTT_t)  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogTT_t logTT = {0};
		logTT.type = e;
		logTT.time = getTimer(TIMER_1)->milliseconds;
		memcpy(curr_mem_ptr, &logTT, sizeof(LogTT_t));
#if OTL_PRINT_LOG
debug_printf("\nДокументируем по адресу %#x", curr_mem_ptr);
#endif
		curr_mem_ptr += sizeof(LogTT_t)/4;
		MemTab[num_of_log_part-1].BankEnd = curr_mem_ptr;
		return SUCCSES;
	}
	else if(TimeOption == NOTIME)
	{
		if((uint8_t*)curr_mem_ptr+sizeof(LogT_t)  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogT_t logT = {0};
		logT.type = e;
#if OTL_PRINT_LOG
debug_printf("\nДокументируем по адресу %#x", curr_mem_ptr);
#endif
		memcpy(curr_mem_ptr, &logT, sizeof(LogT_t));
		curr_mem_ptr += sizeof(LogT_t)/4;
		MemTab[num_of_log_part-1].BankEnd = curr_mem_ptr;
		return SUCCSES;
	}
	else return BAD_PARAM;
}





/*************************************************************************************************************************************************
* Имя функции	 :	LogPoint_TypeArg
* Описание	 :	Функция для документирования определенного события, времени его наступления и аргумента.
* Аргументы      :	Event_t e		- документируемое событие,
* 			bool_t TimeOption 	- лог с учетом времени,
* 			uint32_t arg 		- аргумент.
* Возвращает     :	SUCCSES 		- документирование прошло успешно,
* 			MALLOC_FAILED 		- пул не выделил память,
*			BAD_PARAM 		- ошибка входных параметров.
*************************************************************************************************************************************************/
LOG_RES LogPoint_TypeArg(Event_t e, bool_t TimeOption, uint32_t arg)
{
	if(e < 0  ||  e > 65535) 				return BAD_PARAM;
	if(num_of_log_part == 0) if(MemAdd()) 	return MALLOC_FAILED;							//эта ветка работает только при самом первом пуске.
	if(TimeOption == TIME)
	{
		if((uint8_t*)curr_mem_ptr+sizeof(LogTTA_t)  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogTTA_t logTTA;
		logTTA.type = e;
		logTTA.time = getTimer(TIMER_1)->milliseconds;
		logTTA.arg = arg;
		memcpy(curr_mem_ptr, &logTTA, sizeof(LogTTA_t));
#if OTL_PRINT_LOG
debug_printf("\nЗадокументировали аргумент - %u и время - %u по адресу %#x", ((LogTTA_t*)curr_mem_ptr)->arg, ((LogTTA_t*)curr_mem_ptr)->time, curr_mem_ptr);
#endif
		curr_mem_ptr += sizeof(LogTTA_t)/4;
		MemTab[num_of_log_part-1].BankEnd = curr_mem_ptr;
		return SUCCSES;
	}
	else if(TimeOption == NOTIME)
	{
		if((uint8_t*)curr_mem_ptr+sizeof(LogTA_t)  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogTA_t logTA;
		logTA.type = e;
		logTA.arg = arg;
		memcpy(curr_mem_ptr, &logTA, sizeof(LogTA_t));
#if OTL_PRINT_LOG
debug_printf("\nЗадокументировали аргумент - %u по адресу %#x", ((LogTTA_t*)curr_mem_ptr)->arg, curr_mem_ptr);
#endif
		curr_mem_ptr += sizeof(LogTA_t)/4;
		MemTab[num_of_log_part-1].BankEnd = curr_mem_ptr;
		return SUCCSES;
	}
	else return BAD_PARAM;
}




/*************************************************************************************************************************************************
* Имя функции	 :	LogPoint_TypeArgS
* Описание	 :	Функция для документирования определенного события, времени его наступления и аргументов(более 1).
* Аргументы      :	Event_t e		- документируемое событие,
* 			bool_t TimeOption 	- лог с учетом времени,
* 			uint32_t arg_count 	- количество передаваемых аргументов.
* Возвращает     :	SUCCSES 		- документирование прошло успешно,
* 			MALLOC_FAILED 		- пул не выделил память,
*			BAD_PARAM 		- ошибка входных параметров.
*************************************************************************************************************************************************/
LOG_RES LogPoint_TypeArgS(Event_t e, bool_t TimeOption, uint32_t arg_count, ...)
{
	uint32_t ArgArr[arg_count];
	va_list arg_ptr;
	if(e < 0  ||  e > 65535 || arg_count <= 0) 	return BAD_PARAM;
	va_start (arg_ptr, arg_count);
	for(uint32_t i = 0; i<arg_count; i++)	ArgArr[i] = va_arg(arg_ptr, uint32_t);
	va_end (arg_ptr);
	if(num_of_log_part == 0)	if(MemAdd()) 	return MALLOC_FAILED;						//эта ветка работает только при самом первом пуске.
	if(TimeOption == TIME)
	{
		if((uint8_t*)curr_mem_ptr + sizeof(LogTTAs_t) + arg_count*4  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogTTAs_t logTTAs;
		logTTAs.arg_count = arg_count;
		logTTAs.time = getTimer(TIMER_1)->milliseconds;
		logTTAs.type = e;
		memcpy(curr_mem_ptr, &logTTAs, sizeof(logTTAs));
		curr_mem_ptr += sizeof(LogTTAs_t)/4;
	}
	else if(TimeOption == NOTIME)
	{
		if((uint8_t*)curr_mem_ptr + sizeof(LogTAs_t) + arg_count*4  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogTAs_t logTAs;
		logTAs.arg_count = arg_count;
		logTAs.type = e;
		memcpy(curr_mem_ptr, &logTAs, sizeof(logTAs));
		curr_mem_ptr += sizeof(LogTAs_t)/4;
	}
	else return BAD_PARAM;
	memcpy(curr_mem_ptr, &ArgArr, sizeof(ArgArr));
	curr_mem_ptr += sizeof(ArgArr)/4;
	MemTab[num_of_log_part-1].BankEnd = curr_mem_ptr;
	return SUCCSES;
}



/*************************************************************************************************************************************************
* Имя функции	 :	LogPoint_TypeTimeData
* Описание	 :	Функция для документирования определенного события, времени его наступления и массива данных определенной длины.
* Аргументы      :	Event_t e		- документируемое событие,
* 	 		bool_t TimeOption 	- лог с учетом времени,
* 			void * ptr_to_data	- указатель на данные,
* 			uint32_t size		- размер данных (в байтах, aligned 4!).
* Возвращает     :	SUCCSES 		- документирование прошло успешно,
* 			MALLOC_FAILED 		- пул не выделил память,
*			BAD_PARAM 		- ошибка входных параметров.
*************************************************************************************************************************************************/
LOG_RES LogPoint_TypeData(Event_t e, bool_t TimeOption, void * ptr_to_data, uint32_t size)
{
	if(e < 0  ||  e > 65535 || size <= 0) 		return BAD_PARAM;
	if(num_of_log_part == 0)	if(MemAdd()) 	return MALLOC_FAILED;						//эта ветка работает только при самом первом пуске.
	if(TimeOption == TIME)
	{
		if((uint8_t*)curr_mem_ptr+sizeof(LogTTD_t)+size  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogTTD_t logTTD;
		logTTD.type = e;
		logTTD.time = getTimer(TIMER_1)->milliseconds;
		logTTD.size = size;
		memcpy(curr_mem_ptr, &logTTD, sizeof(LogTTD_t));
		curr_mem_ptr += sizeof(LogTTD_t)/4;
	}
	else if(TimeOption == NOTIME)
	{
		if((uint8_t*)curr_mem_ptr+sizeof(LogTD_t)+size  >  (uint8_t*)mem_border[num_of_log_part-1])
		{
#if OTL_PRINT_LOG
		debug_printf("\nНе хватает памяти в банке №%u", num_of_log_part-1);
#endif
			if(MemAdd()) return MALLOC_FAILED;
		}
		LogTD_t logTD;
		logTD.type = e;
		logTD.size = size;
		memcpy(curr_mem_ptr, &logTD, sizeof(LogTD_t));
		curr_mem_ptr += sizeof(LogTD_t)/4;
	}
	else return BAD_PARAM;
	memcpy(curr_mem_ptr, ptr_to_data, size);
	curr_mem_ptr += size/4;
	MemTab[num_of_log_part-1].BankEnd = curr_mem_ptr;
	return SUCCSES;
}






/*************************************************************************************************************************************************
* Имя функции	 :	LogSend
* Описание	 :	Функция отправки данных из всех банков.
* Аргументы      :	NULL
* Возвращает     :
*************************************************************************************************************************************************/
LOG_RES LogSend( LOG_RES(*SendData)(uint32_t size, uint32_t * data))
{
	if(num_of_log_part == 0) return NOTHING2SEND;
	for(uint32_t i = 0; i<num_of_log_part; i++)
	{
		COLORPRNT_YELLOW();	debug_printf("\nВыгрузка банка %u/%u", i+1,num_of_log_part);	COLORPRNT_RESET();
		SendData((uint32_t)MemTab[i].BankEnd - (uint32_t)MemTab[i].BankStart, MemTab[i].BankStart);
		//mem_free(MemTab[i].BankStart);	//spi сам чистит
		MemTab[i].BankStart = MemTab[i].BankEnd = mem_border[i] =  0;
	}
	COLORPRNT_GREEN();debug_printf("\nГОТОВО!\n");
	curr_mem_ptr = 0;
	num_of_log_part = 0;
	return SUCCSES;
}



/*************************************************************************************************************************************************
* Имя функции	 :	LogPrintRam
* Описание	 :	Функция печати задокументированных данных из банков динамики.
* Аргументы      :	Event_t e - для выборочной печати определенного события.
* Возвращает     :	NULL.
*************************************************************************************************************************************************/
void LogPrintRam(uint32_t log_count, ...)
{
	/*Инструмент работы с переменным количеством аргументов*/
	va_list arg_ptr;
	uint32_t va_arg_var = 0;
	va_start (arg_ptr, log_count);


	uint32_t log_counter = 0;			//счетчик строк
	uint32_t * read_ptr = 0;			//указатель чтения
	uint32_t variable = 0;				//запасная переменная
	uint32_t FLAG_VA = 0;				//флаг успешного поиска аргумента
	COLORPRNT_GREEN();debug_printf("\nПечать логов по событиям Event_t: ")	;COLORPRNT_RESET();
	for(uint32_t i = 0; i<log_count; i++)
	{
		COLORPRNT_SKY(); PrintEventName(va_arg(arg_ptr, Event_t)); COLORPRNT_RESET();
		debug_printf(", ");
	}
	va_start (arg_ptr, log_count);
	for(uint32_t i = 0; i < num_of_log_part; i++)		//чтение происходит по банкам
	{
#if OTL_PRINT_LOG
		debug_printf("\nЛоги %u банка", i);
#endif
		read_ptr = MemTab[i].BankStart;
		while(read_ptr < MemTab[i].BankEnd)
		{
#if OTL_PRINT_LOG
			debug_printf("\nЧитаем по адресу %#x", read_ptr);
			debug_printf("\nСВИЧ - ((LogT_t*)read_ptr)->type - %u", ((LogT_t*)read_ptr)->type);
#endif
			switch(  ((LogT_t*)read_ptr)->type  )
			{

			case TEST0:		//LogT_t
				COLORPRNT_WHITE();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST0 || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС ТИП СОБЫТИЯ - %u", ++log_counter, ((LogT_t*)read_ptr)->type);
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogT_t)/4;
				COLORPRNT_RESET();
				break;


			case TEST0_TI:	//LogTT_t
				COLORPRNT_BLUE();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST0_TI || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС+ВРЕМЯ, ТИП СОБЫТИЯ - %u, ВРЕМЯ - %u", ++log_counter, ((LogTT_t*)read_ptr)->type, ((LogTT_t*)read_ptr)->time);
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogTT_t)/4;
				COLORPRNT_RESET();
				break;


			case TEST1:		//LogTA_t
				COLORPRNT_GREEN();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST1 || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС+АРГУМЕНТ ТИП СОБЫТИЯ - %u,  АРГУМЕНТ - %u",++log_counter, ((LogTA_t*)read_ptr)->type, ((LogTA_t*)read_ptr)->arg);
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogTA_t)/4;
				COLORPRNT_RESET();
				break;


			case TEST1_TI:	//LogTTA_t
				COLORPRNT_GREEN();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST1_TI || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС+ВРЕМЯ+АРГУМЕНТ ТИП СОБЫТИЯ - %u, АРГУМЕНТ - %u, ВРЕМЯ - %u",++log_counter, ((LogTTA_t*)read_ptr)->type, ((LogTTA_t*)read_ptr)->arg, ((LogTTA_t*)read_ptr)->time);
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogTTA_t)/4;
				COLORPRNT_RESET();
				break;


			case TEST2:		//LogTAs_t
				variable = ((LogTAs_t*)read_ptr)->arg_count;
				COLORPRNT_SKY();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST2 || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС+ВРЕМЯ+АРГУМЕНТЫ ТИП СОБЫТИЯ - %u", ++log_counter, ((LogTAs_t*)read_ptr)->type);
						debug_printf("\nКоличество аргументов - %u", ((LogTAs_t*)read_ptr)->arg_count);
						debug_printf("\nАргументы: ");
						FLAG_VA = 1;
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogTAs_t)/4;
				for(uint32_t i = 0; i<variable; i++)
				{
					if(FLAG_VA)	 debug_printf("%u ", *read_ptr);
					read_ptr++;
				}
				COLORPRNT_RESET();
				FLAG_VA = 0;
				break;


			case TEST2_TI:	//LogTTAs_t
				variable = ((LogTTAs_t*)read_ptr)->arg_count;
				COLORPRNT_SKY();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST2_TI || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС+ВРЕМЯ+АРГУМЕНТЫ ТИП СОБЫТИЯ - %u ВРЕМЯ - %u.",++log_counter, ((LogTTAs_t*)read_ptr)->type, ((LogTTAs_t*)read_ptr)->time);
						debug_printf("\nКоличество аргументов - %u", ((LogTTAs_t*)read_ptr)->arg_count);
						debug_printf("\nАргументы: ");
						FLAG_VA = 1;
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogTTAs_t)/4;
				for(uint32_t i = 0; i<variable; i++)
				{
					if(FLAG_VA)  debug_printf("%u ", *read_ptr);
					read_ptr++;
				}
				COLORPRNT_RESET();
				FLAG_VA = 0;
				break;


			case TEST3:	//LogTD_t
				variable = ((LogTD_t*)read_ptr)->size;
				COLORPRNT_RED();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST3 || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС+ДАННЫЕ ТИП СОБЫТИЯ - %u",++log_counter, ((LogTD_t*)read_ptr)->type);
						debug_printf("\nРазмер данных - %u", ((LogTD_t*)read_ptr)->size);
						debug_printf("\nДанные: ");
						FLAG_VA = 1;
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogTD_t)/4;
				/*КАЖДЫЙ РАЗРАБОТЧИК САМ ДЛЯ СЕБЯ ОПИСЫВАЕТ ПРАВИЛА ЧТЕНИЯ ДАННЫХ КОТОРЫЕ ОН ЗАДОКУМЕНТИРОВАЛ*/
				for(uint32_t i = 0; i<variable/sizeof(uint32_t); i++)
				{
					if(FLAG_VA) debug_printf("%u ", *read_ptr);
					read_ptr++;
				}
				COLORPRNT_RESET();
				FLAG_VA = 0;
				break;


			case TEST3_TI:	//LogTTD_t
				variable = ((LogTTD_t*)read_ptr)->size;
				COLORPRNT_RED();
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == TEST3_TI || va_arg_var == ALL )
					{
						debug_printf("\n%u. ЛОГ ТЕСТ ФС+ВРЕМЯ+ДАННЫЕ ТИП СОБЫТИЯ - %u ВРЕМЯ - %u",++log_counter, ((LogTTD_t*)read_ptr)->type, ((LogTTD_t*)read_ptr)->time);
						debug_printf("\nРазмер данных - %u", ((LogTTD_t*)read_ptr)->size);
						debug_printf("\nДанные:");
						FLAG_VA = 1;
						break;
					}
				}
				va_start (arg_ptr, log_count);
				read_ptr += sizeof(LogTTD_t)/4;
				/*КАЖДЫЙ РАЗРАБОТЧИК САМ ДЛЯ СЕБЯ ОПИСЫВАЕТ ПРАВИЛА ЧТЕНИЯ ДАННЫХ КОТОРЫЕ ОН ЗАДОКУМЕНТИРОВАЛ*/
				for(uint32_t i = 0; i<variable/sizeof(uint32_t); i++)
				{
					if(FLAG_VA) debug_printf("%u ", *read_ptr);
					read_ptr++;
				}
				COLORPRNT_RESET();
				FLAG_VA = 0;
				break;


			case OCH_ADD:	//Лог OchAdd
				for(uint32_t i = 0; i<log_count; i++)	//поиск аргумента для печати
				{
					va_arg_var = va_arg(arg_ptr, Event_t);
					if( va_arg_var == OCH_ADD || va_arg_var == ALL )
					{
						debug_printf("\n%u.", ++log_counter);
						debug_printf("\tОчередь\tОбмен\tРазмер\t\tВРЕМЯ\n");
						FLAG_VA = 1;
						break;
					}
				}
				va_start (arg_ptr, log_count);
				variable = ((LogTTAs_t*)read_ptr)->arg_count;
				read_ptr += sizeof(LogTTAs_t)/4;
				if(FLAG_VA)
				{
					COLORPRNT_RED();	PrintOchName(*read_ptr);												COLORPRNT_RESET();
	//									debug_printf("Обмен #");
					COLORPRNT_YELLOW();	debug_printf("%u\t",*(read_ptr+1));										COLORPRNT_RESET();
	//									debug_printf("размером ");
					COLORPRNT_GREEN();	debug_printf("%u.\t\t",*(read_ptr+2));									COLORPRNT_RESET();
	//									debug_printf("ВРЕМЯ: ");
					COLORPRNT_SKY();	debug_printf("%u.", ((LogTTAs_t*)read_ptr-sizeof(LogTTAs_t)/4)->time);	COLORPRNT_RESET();
				}
				read_ptr +=variable;
				FLAG_VA = 0;
				break;
			}
		}
	}
	COLORPRNT_RED();
	if(num_of_log_part == 0) debug_printf("\nЛогов для печати нет!");
	COLORPRNT_RESET();
	log_counter=0;
	va_end(arg_ptr);
}



/*************************************************************************************************************************************************
* Имя функции	 :	LogPrintState
* Описание	 :	Функция состояния памяти системы документирования.
* Аргументы      :	NULL.
* Возвращает     :	NULL.
*************************************************************************************************************************************************/
void LogPrintState(void)
{
	debug_printf("\nПечать состояния системы документирования:");
	if(num_of_log_part)
	{
		debug_printf("\nЗадействовано банков: ");
		COLORPRNT_YELLOW(); debug_printf("%u",num_of_log_part); COLORPRNT_RESET(); debug_printf("/");
		COLORPRNT_RED();debug_printf("%u\n", LOG_BANK_COUNT);COLORPRNT_RESET();
		for(uint32_t i = 0; i<num_of_log_part; i++)
		{
			if(MemTab[i].BankEnd)
			{
				debug_printf("\nБанк №");
				COLORPRNT_YELLOW(); debug_printf("%u", i); COLORPRNT_RESET(); debug_printf(":\t");
				COLORPRNT_GREEN();	debug_printf("%u",(uint32_t)MemTab[i].BankEnd - (uint32_t)MemTab[i].BankStart);COLORPRNT_RESET();debug_printf("/");
				COLORPRNT_RED();	debug_printf("%u", LOG_BANK_SIZE); COLORPRNT_RESET();debug_printf("байт.");
				debug_printf("\nНачальный адрес: ");
				COLORPRNT_GREEN();	debug_printf("%#x", MemTab[i].BankStart); COLORPRNT_RESET();
				debug_printf("\nКонечный  адрес: ");
				COLORPRNT_RED();	debug_printf("%#x\n", MemTab[i].BankEnd); COLORPRNT_RESET();
			}
		}
	}
	else	{ COLORPRNT_RED();debug_printf("\nСистема не была задействована!");COLORPRNT_RESET(); }
}

void LogHelp()
{
	COLORPRNT_GREEN(); debug_printf("\nКоманды системы документирования: "); COLORPRNT_RESET();
	debug_printf("\n\t\"log state\"\t	- печать состояния система документирования");
	debug_printf("\n\t\"log test\"\t	- тестирование системы документирования");
	debug_printf("\n\t\"log sendMD\"\t	- отправка данных из всех задействованных банков в МД");
	debug_printf("\n\t\"log help\"\t	- печать доступных команд системы документирования");
	debug_printf("\n\t\"log print\"\t	- печать всей задокументированной информации");
	debug_printf("\n\t\"log printoch\"\t	- печать задокументированной информации по событию OCH_ADD");
	debug_printf("\n\t\"log printtestt\"\t	- печать задокументированной информации по тестовым событиям с временем");
	debug_printf("\n\t\"log printtest\"\t	- печать задокументированной информации по тестовым событиям без времени");
	debug_printf("\n\t\"log printtest01\"\t - печать задокументированной информации по тестовым событиям TEST0, TEST1");
}



void TEST_LOG_SYSTEM(void)
{
	uint32_t flag = 0;
	uint32_t flag_sum = 0;
	uint32_t testArr[3] = {1488, 1488, 1488};
	for(uint32_t i = 0 ;i<1000;i++)
	{
		/*TEST0*/
		COLORPRNT_RED();
		flag = LogPoint_Type(TEST0, NOTIME);
		if(flag) { debug_printf("\nTEST0 ERROR# %u", flag); flag_sum +=flag; 	}  	//4б
		flag =	LogPoint_Type(TEST0_TI, TIME);
		if(flag) { debug_printf("\nTEST0_TI ERROR# %u", flag); flag_sum +=flag; }	//8б
		/*TEST1*/
		flag = LogPoint_TypeArg(TEST1, NOTIME ,13);
		if(flag) { debug_printf("\nTEST1 ERROR# %u", flag); flag_sum +=flag; 	}	//8б
		flag = LogPoint_TypeArg(TEST1_TI, TIME ,14);
		if(flag) { debug_printf("\nTEST1_TI ERROR# %u", flag); flag_sum +=flag; }	//12б
		/*TEST2*/
		flag = LogPoint_TypeArgS(TEST2, NOTIME ,3, 1337, 1337, 1337);
		if(flag) { debug_printf("\nTEST2 ERROR# %u", flag); flag_sum +=flag; 	}	//8б + 12б 	= 20б
		flag = LogPoint_TypeArgS(TEST2_TI, TIME ,3, 1338, 1338, 1338);
		if(flag) { debug_printf("\nTEST2_TI ERROR# %u", flag); flag_sum +=flag; }	//12б + 12б = 24б
		/*TEST3*/
		flag = LogPoint_TypeData(TEST3, NOTIME, &testArr, sizeof(testArr));
		if(flag) { debug_printf("\nTEST3 ERROR# %u", flag); flag_sum +=flag; 	}	//8б + 12 	= 20б
		flag = LogPoint_TypeData(TEST3_TI, TIME, &testArr, sizeof(testArr));
		if(flag) { debug_printf("\nTEST3_TI ERROR# %u", flag); flag_sum +=flag; }	//12б + 12 	= 24б
	}
	if(flag_sum) 	debug_printf("\nTEST FAIL!");
	else
	{
		COLORPRNT_GREEN();
		debug_printf("\nTEST SUCCSES!");
	}
	COLORPRNT_RESET();
}

