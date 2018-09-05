#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <math.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;
void D_processing();
void XY_processing();
void Percent_string();

string String_read_from_file; // строка, прочитанная из файла
string apertures[100]; //Массив апертур. Пока сто апертур должно хватить
static int Current_position_in_substring = 0; //текущая позиция в строке для считывания.
bool trace_on = 0; //Флаг, индицирующий состояние лазера: остаётся ли линия при перемещении от точки к точке, или нет
/*----------------------------------------*/
bool trace_on_line = 0;					//
bool trace_on_line_prev = 0;			// флаги, необходимые для построения отрезков в JSON
/*----------------------------------------*/
bool flag_out = 0;	// флаг, помогающий выводить данные в файл
bool Zeroes_at_the_end_deleted = 0; //Флаг "Удалены нули в конце"
bool Zeroes_at_the_start_deleted = 1; //Флаг "Удалены нули в начале", по умолчанию взведен
float X_start = 0, X_end = 0, Y_start = 0, Y_end = 0; //Начальные и конечные координаты отрезков,
float units_divider = 1; //Величина, на которую необходимо делить, по умолчанию координаты задаются в миллиметрах, так что длитель будет равен 1.
double X_digits = 0,    //количество цифр в группе 0Х
X_divider = 1,   //делитель координаты ОХ
Y_digits = 0,    //количество цифр в группе 0Y
Y_divider = 1;   //делитель координаты ОY
string Parcer_error_code;
bool Increment_coordinates = 0; //Флаг "Система координат - инкрементная"
bool Absolute_coordinates = 1; //Флаг "Система координат - абсолютная", по умолчанию взведен
unsigned int position_in_string = 5; //текущая позиция в строке для считывания. Используется при пропуске символов при считывании. По умолчанию пропуска нет.

int main() {
	setlocale(LC_ALL, "Russian");

	ifstream Gerber("PCB1.GTL"); // файловый поток, связываемый с разбираемым файлом
	ofstream File_out;			 // файловый поток, связываемый с выходным файлом
	File_out.open("FILEOUT.txt");	// открытие (создание) выходного файла

	if (Gerber.is_open())         //Если получилось открыть файл, то
	{
		while (!Gerber.eof())       // Пока не доберемся до конца файла
		{
			getline(Gerber, String_read_from_file);		// Читаем файл построчно
			//cout << String_read_from_file << "\n";	// И выводим его содержимое

			while (char(String_read_from_file[Current_position_in_substring]) != '\0')
			{
				//cout << String_read_from_file[Current_position_in_substring] << " ";
				switch (char(String_read_from_file[Current_position_in_substring]))
				{
				case 'D': D_processing(); if (flag_out == 1) {
					File_out << "(" << X_start << ";" << Y_start << ")(" << X_end << ";" << Y_end << ")" << endl;
				}
						  break;
				case 'X': XY_processing(); break;
				case 'Y': XY_processing(); break;
				case '%': Percent_string(); break;  //начало select case строчек, начинающихся с процента
				}// <- конец большого select case
				Current_position_in_substring++;
			}// <- конец вывода строки посимвольно
			Current_position_in_substring = 0;
			/*
			{

			/*case 'Y': Y_processing(); break;
			case 'M': M_processing(); break;
			case '*': { if (String_read_from_file[Current_position_in_substring+1]=='\0') cout << "End of line \n";
			else cout << "No asterisk after block end \n" ;
			Current_position_in_substring = 0;
			break;}

			}*/

		}
		Gerber.close();                                                             // потом закрываем файловый поток
		File_out.close();															//
	}
	else cout << "Где этот грёбаный гербер?"; // Не удалось найти файл

	return 0;

}


void Percent_string()
{
	//---------------------------------------------------------------*
	map <string, int> mapping;  // таблица для select case         |
								//---------------------------------------------------------------*
	mapping["FS"] = 1; //обработчик для format statement          |
	mapping["MO"] = 2; //обработчик для Data Block Format         |
	mapping["SF"] = 3; //обработчик для Data Block Format factor  |
	mapping["IP"] = 4; //обработчик для Data Block Format polarity|
	mapping["AD"] = 5; //обработчик для Aperture Definition       |
	mapping["LN"] = 6; //обработчик для Data Block Format string  |
	mapping["LP"] = 7; //обработчик для полярности слоя           |
	//---------------------------------------------------------------*

	ostringstream current_string;  // строчка для временного хранения данных, считанных с определённой позиции строки файла
	current_string << String_read_from_file.substr(1, 2); // считываем два символа, идущие после знака процента
	switch (mapping[current_string.str()]) //смотрим, какие два символа идут после знака процента
	{
	case 1 /*"FS"*/:
	{ cout << "Processing the format statement \n";
	switch (char(String_read_from_file[3]))
	{
	case 'T': {cout << "Zeroes at the end deleted \n"; Zeroes_at_the_end_deleted = 1; Zeroes_at_the_start_deleted = 0; break; }
	case 'L': {cout << "Zeroes at the start deleted  \n"; Zeroes_at_the_end_deleted = 0; Zeroes_at_the_start_deleted = 1; break; }
			  //Следующая буква — другая? Тогда конец программы. Код ошибки — F01
	default: {cout << "Parcer_error_code F01, by default zeroes at the start are deleted\n"; Parcer_error_code = "F01"; break; }
	}
	switch (char(String_read_from_file[4]))
	{
	case 'A': {cout << "Absolute coordinates \n"; Absolute_coordinates = 1; Increment_coordinates = 0; break; }
	case 'I': {cout << "Increment coordinates \n"; Absolute_coordinates = 0; Increment_coordinates = 1; break; }
			  //Следующая буква — другая? Тогда конец программы. Код ошибки — F010
	default: {cout << "Parcer error code F010, by default coordinate system is absolute\n"; Parcer_error_code = "F010"; break; }
	}
	switch (char(String_read_from_file[5])) //Пропуск G и N
	{
	case 'N': {cout << "N" << String_read_from_file[6] << "are skipped"; position_in_string = 7; break; }
	case 'G': {cout << "G" << String_read_from_file[6] << "are skipped"; position_in_string = 7; break; }
			  //Если нет ни G, ни N, то простo едем дальше
	default: {cout << "Nothing skipped \n"; Current_position_in_substring++; // Сместились ещё на символ
		break; }
	}
	if (char(String_read_from_file[position_in_string]) == 'X') //Если есть описание ОХ
	{
		cout << "0X axis: ";
		// сперва узнаём количество знаков до запятой, для этого объявляем переменную, куда запихиваем цифры, идущие за буквой "Х"
		stringstream current_character(String_read_from_file.substr(position_in_string + 1));
		//берем первый символ, преобразовываем его в целочисленное и записываем в переменную количества знаков
		//да, конечно, всё через жопу, но std::stoi() и std::atoi() какого-то хрена не работают, а поскольку патч на MinGW от 12 года,
		//то ломать с его помощью 4.9 версию, а потом всё переустанавливать по 10 раз я не хочу
		char before_and_after_the_dot = current_character.str()[0];
		X_digits = before_and_after_the_dot - '0'; //вычитаем символ нуля, соответственно, сдвигаемся по таблице, после чего получаем
												   //число знаков до запятой, записанное в переменную
		cout << "Digits before decimal dot " << X_digits << ",  ";
		before_and_after_the_dot = current_character.str()[1];
		X_divider = before_and_after_the_dot - '0'; //вычитаем символ нуля, соответственно, сдвигаемся по таблице, после чего получаем
													//число знаков до запятой, записанное в переменную
		cout << "Digits after decimal dot " << X_divider << ",  ";
		X_digits = X_digits + X_divider;
		cout << "Digits total " << X_digits << " \n";
		//Чему равна вторая цифра? Если 1, то делитель_Х = 10, если 2, то делитель_Х = 100, если 3, то делитель_Х = 1000,
		//если 4, то делитель_Х = 10`000, если 5, то делитель_Х = 100`000, если 6, то делитель_Х = 1000`000.
		X_divider = round(pow(10, X_divider));
		cout << "X_divider: " << X_divider << "\n";
	}
	else { cout << "Parcer error code F02, OX format missed"; Parcer_error_code = "F02"; }
	position_in_string = position_in_string + 3; //смещаемся в строке на букву Х и цифры, идущие за ней

	if (char(String_read_from_file[position_in_string]) == 'Y') //Если есть описание ОY
	{
		cout << "0Y axis: ";
		// сперва узнаём количество знаков до запятой, для этого объявляем переменную, куда запихиваем цифры, идущие за буквой "Y"
		stringstream current_character(String_read_from_file.substr(position_in_string + 1));
		//берем первый символ, преобразовываем его в целочисленное и записываем в переменную количества знаков
		//да, конечно, всё через жопу, но std::stoi() и std::atoi() какого-то хрена не работают, а поскольку патч на MinGW от 12 года,
		//то ломать с его помощью 4.9 версию, а потом всё переустанавливать по 10 раз я не хочу
		char before_and_after_the_dot = current_character.str()[0];
		Y_digits = before_and_after_the_dot - '0'; //вычитаем символ нуля, соответственно, сдвигаемся по таблице, после чего получаем
												   //число знаков до запятой, записанное в переменную
		cout << "Digits before decimal dot " << Y_digits << ",  ";
		before_and_after_the_dot = current_character.str()[1];
		Y_divider = before_and_after_the_dot - '0'; //вычитаем символ нуля, соответственно, сдвигаемся по таблице, после чего получаем
													//число знаков до запятой, записанное в переменную
		cout << "Digits after decimal dot " << Y_divider << ",  ";
		X_digits = Y_digits + Y_divider;
		cout << "Digits total " << Y_digits << " \n";
		//Чему равна вторая цифра? Если 1, то делитель_Y = 10, если 2, то делитель_Y = 100, если 3, то делитель_Y = 1000,
		//если 4, то делитель_Y = 10`000, если 5, то делитель_Y = 100`000, если 6, то делитель_Y = 1000`000.
		Y_divider = round(pow(10, Y_divider));
		cout << "Y_divider: " << Y_divider << "\n";
	}
	else { cout << "Parcer error code F03, OY format missed"; Parcer_error_code = "F03"; }
	position_in_string = position_in_string + 3; //смещаемся в строке на букву Y и цифры, идущие за ней
	if (char(String_read_from_file[position_in_string]) == '*' && char(String_read_from_file[position_in_string + 1]) == '%')
		cout << "FS successfully processed \n \n";
	else { cout << "Parcer error code F04, end of line of format statement is missed. \n"; Parcer_error_code = "F04"; }
	//cout << "String_read_from_file.length() " << int(String_read_from_file.length()) << "\n\n";
	}
	case 2 /*"MO"*/:
	{ cout << "Data Block Format: ";
	if (String_read_from_file.substr(3, 2) == "IN") { cout << "inches \n"; units_divider = 25.4; } //Если единицы - дюймы, то координаты нужно
	if (String_read_from_file.substr(3, 2) == "MM") cout << "millimeters \n"; // делить на 25,4, чтобы получить миллиметры. А если миллиметры, то нет.
	else cout << "Parcer error code M01, units are not specified, units set to millimeters. \n";
	if (char(String_read_from_file[5]) == '*' && char(String_read_from_file[6]) == '%') //Если строчка не заканчивается *%, то это - ошибка
		cout << "MO successfully processed \n \n";
	else { cout << "Parcer error code М02, end of line of format statement is missed. \n"; Parcer_error_code = "М02"; }
	break;
	}
	case 3 /*"SF"*/: cout << "Data Block Format factor \n"; break;
	case 4 /*"IP"*/: cout << "Data Block Format polarity \n"; break;
	case 5 /*"AD"*/:
	{ cout << "Aperture Definition \n";
	if (String_read_from_file.substr(3, 1) == "D")
	{
		cout << "Aperture found: ";
		//Считываем двузначное число, которое должно быть после буквы "D"
		stringstream current_aperture_number(String_read_from_file.substr(4));
		char buffer[2]; // для временного хранения номера апертуры
		buffer[0] = char(current_aperture_number.str()[0]); //записываем в буфер две цифры по порядку, это - первая,
		cout << "Aperture number: ";
		buffer[1] = char(current_aperture_number.str()[1]); // - это - вторая.
		cout << atoi(buffer) << "\n";                     // и преобразовываем их в целое число - номер апертуры в таблице
														  // теперь запишем эту строку в массив апертур
		int position_in_string = 6; // текущая позиция для считывания чимволов. Она такая, потому что мы уже сместились на "%ADDXX
		stringstream temporary_aperture_storage; // строка для временного хранения апертуры
		while (char(String_read_from_file[position_in_string]) != '\0') // Пока не дойдём до конца строки, пихаем во временную строку описание
		{
			temporary_aperture_storage << String_read_from_file[position_in_string]; // апертуры посимвольно
			if (char(String_read_from_file[position_in_string + 1]) == '*' && char(String_read_from_file[position_in_string + 2]) == '%')//если
																																		 //добрались до конца записи, то записываем апертуру в массив апертур
			{
				apertures[atoi(buffer)] = temporary_aperture_storage.str();
				cout << "Aperture Definition is successfully processed. Aperture number: " << atoi(buffer) << " aperture: "
					<< apertures[atoi(buffer)] << " \n";
			}
			else if (char(String_read_from_file[position_in_string]) == '*' && char(String_read_from_file[position_in_string + 1]) != '%')
			{
				cout << "Parcer error code A02, no end of aperture description \n"; Parcer_error_code = "A02";
			}
			position_in_string++; //Смещаемся на следующий символ
		}
	}
	else cout << "Parcer error code A01, no aperture number \n";
	break;
	}
	case 6 /*"LN"*/: cout << "Data Block Format string \n"; break;
	case 7 /*"LP"*/: cout << "Layer polarity \n"; break;

	default: cout << "Unknown statement \n";
		cout << "String length" << String_read_from_file.length();
	}
	Current_position_in_substring = String_read_from_file.length() - 1; // сместились на длину строки
}

void D_processing()
{
	trace_on_line_prev = trace_on_line;
	flag_out = 0;
	char buffer[3] = { '0','0', '\0' };    //Это, конечно, весело. Смысл в том, что буфер символов любит собирать мусор до тех пор, пока ему не запишут
	buffer[0] = char(String_read_from_file[Current_position_in_substring + 1]); //нуль-терминатор. Так что для дальнейших преобразований нужно записать нуль-терминатор, иначе в
	buffer[1] = char(String_read_from_file[Current_position_in_substring + 2]); //буфере будет сидеть куча мусора. Ну а так здесь забирают первую и вторую цифру после буквы "D", затем
	if (atoi(buffer) >= 10)                 //преобразовывают это число в целое, если оно больше, либо равно 10, то это номер апертуры
											//выводим апертуру из таблицы по этому номеру
		cout << "Aperture enabled: #" << atoi(buffer) << " " << apertures[atoi(buffer)] << endl;
	else if ((atoi(buffer) == 1) || (atoi(buffer) == 3)) //если этот номер - состояние лазера "вкл", то
		trace_on = 1;                                //записываем такое состояние лазера
	else if (atoi(buffer) == 2)                        //если этот номер - состояние лазера "выкл", то
		trace_on = 0;                                //записываем такое состояние лазера
	else cout << "Parcer error code D01, unknown command " << '"' << "D" << atoi(buffer) << "\n";

	if (String_read_from_file.length() > 4 && (atoi(buffer) == 1 || atoi(buffer) == 3)) {			// если строка содержит координаты XY и состояние лазера "вкл",
		trace_on_line = 1;																			// то меняем состояние флага на "true"
	}
	else {
		trace_on_line = 0;
	}
	if ((trace_on_line == 1 && trace_on_line_prev == 0) || (trace_on_line == 1 && trace_on_line_prev == 1)) {	// если предыдущая строка имела координаты и считываемая в этот момент строка
		flag_out = 1;																							// имеет флаг включенного лазера
		//cout << trace_on_line << endl;
		//cout << trace_on_line_prev << endl;
	}
	//cout << "Trace switched to :" << trace_on << "\n\n";		// для отладки
	Current_position_in_substring = Current_position_in_substring + 2;  //смешаемся в строке на последнюю позицию, но не на следующую, там в конце
																	    // переменная "Current_position_in_substring" будет инкрементирован
	cout << endl;
}

void XY_processing()
{
	//cout << "X found \n";
	X_start = X_end;
	if (char(String_read_from_file[Current_position_in_substring]) == 'X') {
		stringstream temporary_X_storage; // строка для временного хранения координаты
		Current_position_in_substring++; //Буква уже была считана, так что смещаемся на один символ
		while (isdigit(String_read_from_file[Current_position_in_substring])) // Пока не дойдём до следующей буквы, пихаем во временную строку значение
		{
			temporary_X_storage << String_read_from_file[Current_position_in_substring]; // координаты посимвольно
			Current_position_in_substring++;
		}
		temporary_X_storage << '\0';  //завершаем строку нуль-символом
		string str_X = temporary_X_storage.str(); //запихиваем переменную в строку
		const char *X_coordinate = str_X.c_str(); // делаем указатель на неё. важен тип указателя - символьный
		if (Zeroes_at_the_start_deleted) //Если удалены нули в начале, то
			X_end = (atof(X_coordinate) / X_divider)*units_divider;
		else cout << "Zeroes at the end deleted. This part is not written yet \n";
		//cout << "Х value: " << X_end << "\n";			// для откладки
	}

	Y_start = Y_end;
	if (char(String_read_from_file[Current_position_in_substring]) == 'Y') {
		stringstream temporary_Y_storage; // строка для временного хранения координаты
		Current_position_in_substring++; //Буква уже была считана, так что смещаемся на один символ
		while (isdigit(String_read_from_file[Current_position_in_substring])) // Пока не дойдём до следующей буквы, пихаем во временную строку значение
		{
			temporary_Y_storage << String_read_from_file[Current_position_in_substring]; // координаты посимвольно
			Current_position_in_substring++;
		}
		temporary_Y_storage << '\0';  //завершаем строку нуль-символом
		string str_Y = temporary_Y_storage.str(); //запихиваем переменную в строку
		const char *Y_coordinate = str_Y.c_str(); // делаем указатель на неё, важен тип указателя - символьный
		if (Zeroes_at_the_start_deleted) //Если удалены нули в начале, то
			Y_end = (atof(Y_coordinate) / Y_divider)*units_divider;
		else cout << "Zeroes at the end deleted. This part is not written yet \n";

		//cout << "Y value: " << Y_end << "\n";			// для откладки

		Current_position_in_substring--;	// уменьшаем индекс, чтобы значение переменной не конкурировало с алгоритмом в методе main
	}
	else {
		Current_position_in_substring--; // уменьшаем индекс, чтобы значение переменной не конкурировало с алгоритмом в методе main
	}
}



/*
Если этот символ - «Х» то считать следующие за ним символы в количестве Количество_считываемых_символов_после_Х_при_выполнении_работы_по_ОХ
и преобразовать их в число. Записать в переменную «X» значение (это число*Масштаб по ОХ/делитель_Х).

Если этот символ «Y”, то считать следующие за ним символы в количестве Количество_считываемых_символов_после_У_при_выполнении_работы_по_ОУ
и преобразовать их в число. Записать в переменную «У» значение (это число*Масштаб по ОY/делитель_У).

Если этот символ «D”, то считать следующие за ним 2 символа.
Преобразовать считанные 2 символа в число.
(Если это число = 01, то записать в переменную «Тип хода» значение «G00» Если это число = 01, то записать в переменную «Тип хода» значение «G01»)
Если этот символ «*”, то записать в файл с новой строки «Тип хода»X”переменнаяХ»У»переменнаяУ»

Если этот символ «М», то считать следующие за ним 2 символа.
Преобразовать эти 2 символа в число.
Если это число равно 2, то записать в файл с новой строки «М0»
иначе вывести сообщение «Неизвестная команда М0», число. Код ошибки М03.


*/
