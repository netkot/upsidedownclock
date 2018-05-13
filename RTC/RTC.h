//	Библиотека работы arduino с модулями часов реального времени, версия 2015-10-01
//	Библиотека написана для сайта http://iarduino.ru/ автор Панькин Павел sajaem@narod.ru
//	P.S.: автор выражает свою благодарность интернет-магазину iarduino за предоставленные модули для написания библиотеки!
//	
//	Модули RTC с шинами I2C и SPI, подключаются к аппаратным выводам arduino
//	Модули RTC с трехпроводной шиной, подключаются к любым выводам arduino, но их нужно будет указать в функции begin
//	
//	Для работы с модулями, в библиотеке реализованы 4 функции:
//	    указать модуль       begin  (название модуля [, вывод SS/RST [, вывод CLK [, вывод DAT]]]);
//	    указать время        settime(секунды [, минуты [, часы [, день [, месяц [, год [, день недели]]]]]]);
//	    получить время       gettime("строка с параметрами");
//	    разгрузить шину      period (минуты);
//	
//	Функция begin(название модуля [, вывод SS/RST [, вывод CLK, вывод DAT]]):
//	    если модуль работает на шине I2C или SPI, то достаточно указать 1 параметр, например: begin(RTC_DS3231);
//	    если модуль работает на шине SPI, а аппаратный вывод SS занят, то номер назначенного вывода SS для модуля указывается вторым параметром, например: begin(RTC_DS1305,22);
//	    если модуль работает на трехпроводной шине, то указываются номера всех выводов, например: begin(RTC_DS1302, 1, 2, 3); // RST, CLK, DAT
//	
//	Функция settime(секунды [, минуты [, часы [, день [, месяц [, год [, день недели]]]]]]):
//	    записывает время в модуль
//	    год указывается без учёта века, в формате 0-99
//	    часы указываются в 24-часовом формате, от 0 до 23
//	    день недели указывается в виде числа от 0-воскресенье до 6-суббота
//	    если предыдущий параметр надо оставить без изменений, то можно указать отрицательное или заведомо большее значение
//	    пример: settime(-1, 10); установит 10 минут, а секунды, часы и дату, оставит без изменений
//	    пример: settime(0, 5, 13); установит 13 часов, 5 минут, 0 секунд, а дату оставит без изменений
//	    пример: settime(-1, -1, -1, 1, 10, 15); установит дату 01.10.2015 , а время и день недели оставит без изменений
//	
//	Функция period(минуты):
//	    устанавливает минимальный период обращения к модулю в минутах (от 0 до 255)
//	    функция может быть полезна, если шина сильно нагружена, на ней имеются несколько устройств
//	    period(10); период 10 минут, означает что каждые 10 минут к модулю может быть отправлен только 1 запрос на получение времени
//	    ответом на все остальные запросы будет результат последнего полученного от модуля времени + время прошедшее с этого запроса
//	
//	Функция gettime("строка с параметрами"):
//	    функция получает и выводит строку заменяя описанные ниже символы на текущее время
//	    пример: gettime("d-m-Y, H:i:s, D"); ответит строкой "01-10-2015, 14:00:05, Thu"
//	    пример: gettime("s");               ответит строкой "05"
//	    указанные символы идентичны символам для функции date() в PHP
//	s   секунды                       от      00    до       59  (два знака)
//	i   минуты                        от      00    до       59  (два знака)
//	h   часы в 12-часовом формате     от      01    до       12  (два знака)
//	H   часы в 24-часовом формате     от      00    до       23  (два знака)
//	d   день месяца                   от      01    до       31  (два знака)
//	w   день недели                   от       0    до        6  (один знак: 0-воскресенье, 6-суббота)
//	D   день недели наименование      от     Mon    до      Sun  (три знака: Mon Tue Wed Thu Fri Sat Sun)
//	m   месяц                         от      01    до       12  (два знака)
//	M   месяц наименование            от     Jan    до      Dec  (три знака: Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec)
//	Y   год                           от    2000    до     2099  (четыре знака)
//	y   год                           от      00    до       99  (два знака)
//	a   полдень                               am   или       pm  (два знака, в нижнем регистре)
//	A   полдень                               AM   или       PM  (два знака, в верхнем регистре)
//	    строка не должна превышать 50 символов
//	
//	если требуется получить время в виде цифр, то можно вызвать функцию gettime() без параметра, после чего получить время из переменных
//	    seconds  секунды     0-59
//	    minutes  минуты      0-59
//	    hours    часы        1-12
//	    Hours    часы        0-23
//	    midday   полдень     0-1 (0-am, 1-pm)
//	    day      день месяца 1-31
//	    weekday  день недели 0-6 (0-воскресенье, 6-суббота)
//	    month    месяц       1-12
//	    year     год         0-99


#ifndef RTC_h
#define RTC_h

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define	RTC_UNDEFINED	0	//	Модуль часов реального времени не определён
#define	RTC_DS3231		1	//	Модуль часов реального времени с протоколом передачи данных I2C, памятью 019x8, температурной компенсацией, двумя будильниками и встроенным кварцевым резонатором
#define	RTC_DS1302		2	//	Модуль часов реального времени с протоколом передачи данных SI3, памятью 040x8 (31 байт которой доступны для хранения данных пользователя)
#define	RTC_DS1307		3	//	Модуль часов реального времени с протоколом передачи данных I2C, памятью 064x8 (56 байт которой доступны для хранения данных пользователя)

#define	RTC_I2C			1	//	Интерфейс передачи данных по двухпроводной шине
#define	RTC_SPI			2	//	Интерфейс передачи данных по четырехпроводной шине
#define	RTC_SI3			3	//	Интерфейс передачи данных по трехпроводной шине (недо SPI пере I2C)


class RTC{
	public:
	/**	пользовательские функции **/
		void  begin  (uint8_t,uint8_t=SS,uint8_t=SCK,uint8_t=MOSI);	//	инициализация модуля (название [, вывод SS/RST [, вывод CLK [, вывод DAT]]])
		void  period (uint8_t);										//	минимальный период обращения к модулю (мин)
		char* gettime(char*);										//	получить строку данных (строка с параметрами)
		void  gettime();											//	получить данные из переменных
		void  settime(int,int=-1,int=-1,int=-1,int=-1,int=-1,int=-1);//	указать время (сек,мин,час,день,мес,год,день недели)

	/**	переменные доступные для пользователя **/
		uint8_t	seconds					=	0;						//	секунды			0-59
		uint8_t	minutes					=	0;						//	минуты			0-59
		uint8_t	hours					=	12;						//	часы			1-12
		uint8_t	Hours					=	0;						//	часы			0-23
		uint8_t	midday					=	0;						//	полдень			0-1		(0-am, 1-pm)
		uint8_t	day						=	1;						//	день месяца		1-31
		uint8_t	weekday					=	0;						//	день недели		0-6		(0-воскресенье, 6-суббота)
		uint8_t	month					=	1;						//	месяц			1-12
		uint8_t	year					=	15;						//	год				0-99	(без учёта века)
		
	private:
	/**	параметры модулей **/
		uint8_t	RTC_uint_MODULE			=	0;						//	чип модуля RTC (по умолчанию = RTC_UNDEFINED)
		uint8_t	RTC_array_BUS[4][8]		=	{						//	указание типа шины и режимов её работы
			/*	RTC_UNDEFINED	0	*/		{0},					//	шина данных, частота шины в кГц, адрес модуля (0-7F) или активное состояние линии выбора модуля (RES/SS), режим работы при чтении (mode=0...3), режим работы при записи (mode=0...3), флаг переворачивания данных младшим битом вперед при чтении, флаг переворачивания данных младшим битом вперед при записи, слагаемое к адресу регистра для записи
			/*	RTC_DS3231		1	*/		{RTC_I2C,  100, 0x68, 0, 0, 0, 0, 0},
			/*	RTC_DS1302		2	*/		{RTC_SI3,   10,    1, 1, 0, 1, 1,-1},
			/*	RTC_DS1307		3	*/		{RTC_I2C,  100, 0x68, 0, 0, 0, 0, 0},
											};
	   uint16_t	RTC_array_STR[4][11]	=	{						//	установка битов регистров модулей при старте (биты регистра будут перезаписаны, только если они отличаются от указанных)
			/*	RTC_UNDEFINED	0	*/		{0},					//	количество регистров, № регистра, проверяемые биты, ... , № регистра, проверяемые биты (на каждый проверяемый бит отводятся два бита в массиве: "0X" - оставить бит без изменений, "10" - установить бит в "0", "11" - установить бит в "1")
			/*	RTC_DS3231		1	*/		{3,0x02,0b0010000000000000,0x0E,0b1010001010101010,0x0F,0b1000000011001010},
			/*	RTC_DS1302		2	*/		{3,0x8F,0b1000000000000000,0x81,0b1000000000000000,0x85,0b1000000000000000},
			/*	RTC_DS1307		3	*/		{3,0x00,0b1000000000000000,0x02,0b0010000000000000,0x07,0b0000001100001010},
											};
		uint8_t	RTC_array_REG[4][7]		=	{						//	адреса регистров даты и времени
			/*	RTC_UNDEFINED	0	*/		{0},					//	сек, мин, час, день, месяц, год, день недели
			/*	RTC_DS3231		1	*/		{0x00,0x01,0x02,0x04,0x05,0x06,0x03},
			/*	RTC_DS1302		2	*/		{0x81,0x83,0x85,0x87,0x89,0x8D,0x8B},
			/*	RTC_DS1307		3	*/		{0x00,0x01,0x02,0x04,0x05,0x06,0x03},
											};
		uint8_t	RTC_ignor_BIT[4][7]		=	{						//	маска для чтения/записи. совершить побитовое и (&) с данными регистров даты и времени
			/*	RTC_UNDEFINED	0	*/		{0},					//	сек, мин, час, день, месяц, год, день недели
			/*	RTC_DS3231		1	*/		{0x7F,0x7F,0x3F,0x3F,0x1F,0xFF,0x07},
			/*	RTC_DS1302		2	*/		{0x7F,0x7F,0x3F,0x3F,0x1F,0xFF,0x07},
			/*	RTC_DS1307		3	*/		{0x7F,0x7F,0x3F,0x3F,0x1F,0xFF,0x07},
											};

	/**	================================================================================================================================= **/

	/**	внутренние функции **/
		void	RTC_func_START();									//	установка флагов управления и состояния модуля
		void	RTC_func_READ_TIME();								//	чтение даты и времени из регистров модуля
		void	RTC_func_WRITE_TIME(int,int,int,int,int,int,int);	//	запись даты и времени в регистры модуля					(сек,мин,час,день,мес,год,день недели)
		uint8_t	RTC_func_READ_TIME_INDEX(uint8_t);					//	чтение одного из значений даты и времени из регистра	(индекс)
		void	RTC_func_WRITE_TIME_INDEX(uint8_t, uint8_t);		//	запись одного из значений даты и времени в регистр		(индекс, значение)
		uint8_t	RTC_func_READ_REG(uint8_t);							//	чтение регистра модуля									(адрес регистра)
		void	RTC_func_WRITE_REG(uint8_t, uint8_t);				//	запись в регистр модуля									(адрес регистра, байт данных)
		uint8_t	RTC_func_CONVER_CODE_NUM(uint8_t);					//	преобразование двоично-десятичного кода в число			(код)
		uint8_t	RTC_func_CONVER_NUM_CODE(uint8_t);					//	преобразование числа в двоично-десятичный код			(число)
		void	RTC_func_SET_MORE_TIME();							//	преобразование переменных не читаемых из модуля
		void	RTC_func_CALC_TIME();								//	увеличение переменных времени на разницу между последним и текущим запросами
		void	RTC_func_FILL_RESULTS(uint8_t,uint8_t);				//	заполнение строки вывода результата						(число: 0 год/1 число дня недели/2 две цифры/3 строка AM PM/4 строка месяц или день недели, данные)

	/**	внутренние переменные **/
		char	RTC_char_RETURN[50];								//	строка вывода результата
		char	RTC_uint_RETURN			=	0;						//	позиция для записи в строку вывода результата
		char	RTC_char_MIDDAY[4][3]	=	{"am","pm","AM","PM"};
		char	RTC_char_DayMon[19][4]	=	{"Sun","Mon","Tue","Wed","Thu","Fri","Sat","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
		uint8_t	RTC_buff_TIMEREG[7];								//	буфер данных прочитанных из регистров времени модуля
		uint8_t	RTC_uint_CENTURY		=	21;						//	текущий век
	   uint16_t	RTC_uint_PERIOD			=	0;						//	минимальный период опроса модуля (в минутах) от 00 до 255
	   uint32_t	RTC_uint_REQUEST		=	0;						//	время последнего чтения из модуля 

	/**	функции для работы с шинами I2C, SPI, SI3 **/
	//			выводы используемые шинами							//	I2C и SPI	используют аппаратные выводы.
		uint8_t	SI3_line_CLK			=	SCK;					//	№ вывода	для линии CLK шины SI3 (CLocK)
		uint8_t	SI3_line_DAT			=	MOSI;					//	№ вывода	для линии DAT шины SI3 (DATa)
		uint8_t	SI3_line_RES			=	SS;						//	№ вывода	для линии RES шины SI3 (RESet)				Контроль за выводом не выполняется функциями работы с шинами
		uint8_t	SPI_line_SS				=	SS;						//	№ вывода	для линии SS  шины SPI (SlaveSelect)		Контроль за выводом не выполняется функциями работы с шинами
	//			массивы режимов работы
	   uint16_t	SPI_mass_STATUS[0x06]	=	{4000,1000,1,0,1,0};	//  скорость	работы шины в кГц (макс F_CPU/2  ), ожидание сброса флага SPIF  в циклах, флаг успешного результата, режим работы SPI (0-3), флаг работы в режиме мастера, флаг переворачивания данных младшим битом вперед
	   uint16_t	I2C_mass_STATUS[0x04]	=	{ 100,1000,1,0x78 };	//	скорость	работы шины в кГц (макс F_CPU/2  ), ожидание сброса флага TWINT в циклах, флаг успешного результата, содержимое флагов TWS регистра состояния TWSR
	   uint16_t	SI3_mass_STATUS[0x06]	=	{  10,1000,1,0,1,1};	//  скорость	работы шины в кГц (макс F_CPU/354), ожидание импульса на CLK    в циклах, флаг успешного результата, режим работы SI3 (0-3), флаг работы в режиме мастера, флаг переворачивания данных младшим битом вперед
	//			инициализация шины
		void	I2C_func_begin			();							//	установка	скорости работы шины I2C					(без параметров)
		void	SPI_func_begin			();							//	установка	скорости работы шины SPI и режима её работы	(без параметров)
		void	SI3_func_begin			(uint8_t, uint8_t);			//	установка	скорости работы шины SI3 и режима её работы	(номер CLK, номер DAT)
	//			низкоуровневые команды
		void	I2C_func_START			();							//	выполнение	состояния START								(без параметров)
		void	I2C_func_RESTART		();							//	выполнение	состояния RESTART							(без параметров)
		void	I2C_func_STOP			();							//	выполнение	состояния STOP								(без параметров)
		void	I2C_func_SEND_ID		(uint8_t, bool);			//	передача	первого байта								(ID-адрес модуля, бит RW)
	//			работа с байтом
		void	I2C_func_WRITE_BYTE		(uint8_t);					//	передача	одного байта								(байт для передачи)
		uint8_t	I2C_func_READ_BYTE		(bool);						//	получение	одного байта								(бит подтверждения ACK/NACK)
		uint8_t	SPI_func_WRITEREAD_BYTE	(uint8_t);					//	переполучен	одного байта								(байт для передачи)
		void	SI3_func_WRITE_BYTE		(uint8_t);					//	передача	одного байта								(байт для передачи)
		uint8_t	SI3_func_READ_BYTE		(bool);						//	получение	одного байта								(флаг чтения предустановленного бита с линии DAT)
	//			работа с битами
		void	SI3_func_WRITE_BITS		(uint32_t, uint8_t);		//	передача	нескольких бит из 32-битного числа			(число, количество младших бит числа для передачи)
	   uint32_t	SI3_func_READ_BITS		(uint8_t, bool = false);	//	чтение		нескольких бит, но не более 32-ух			(количество принимаемых бит, флаг чтения предустановленного бита с линии DAT)
																	//	Если старший бит данных установлен тактом CLK предшествующим запуску функции SI3_func_READ_BITS,
																	//	то необходимо указать второй параметр функции как true, для чтения этого бита данных.
																	//	Чтение предустановленного бита может понадобиться если запуску функции предшествует смена режима работы шины на 1 или 3 (CPHA==1)
};

#endif