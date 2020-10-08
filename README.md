-------------------------------------------------------------
-------------------------------------------------------------
TCP Client-Server via ISO 8583 Protocol
-------------------------------------------------------------
-------------------------------------------------------------

2 приложения:

1. Клиент

    1.1 Собирает минимальное сообщение по протоколу ISO8583 (2, 6, 7, 11 поля) + bitmap
    
    1.2 Отправляет по сети сообщение на сервер
    
    1.3 Принимает ответ от сервера и печатает в stdout содержимое полей
 
2. Сервер

   1.1 Слушает порт, ожидает входящих соединений
   
   1.2 Принимает сообщение от клиента, разбирает сообщение
   
   1.3 Печатает на экран содержимое полей
   
   1.4 Генерируется ответ клиенту с теми же полями (+ 39 поле)
   
   1.5 Код ответа в 39 поле сделать настраиваемым
   
   1.6 Настройки мапирования кода ответа нужно считывать из БД oracle (можно считать при старте приложения в кэш и перечитывать по команде)
   
   1.7 Нужно поддержать в приложении возможность легкого добавления новых полей (по возможности без доработки)
   
   1.8 Мапирование 39 поля делать на основании входящих полей (способ мапирования на выбор)
 
 
Реквизиты подключений/слушающего порта выносить в настройки не надо, всё можно зашить в код.

Протокол ISO8583 базовый: https://ru.wikipedia.org/wiki/ISO_8583  

Реализация на C++  используя паттерны, если это возможно.

Приложение должно собираться под Linux (RedHat / CentOS).


-------------------------------------------------------------
SOLUTION
-------------------------------------------------------------

Решение реализовано на С++/STL/Boost. Использован стандарт C++17.

Для клиент-серверной реализации использована библиотека Boost Asio. Реализовано полностью асинхронное решение.

Для подключения к СУБД Oracle использован OCCI.

Для базовой сборки под Windows использован MSVC 2017 x64.

Для кроссплатформенной сборки использован CMake.

Решение поддерживает любые поля протокола ISO 8583 (Диапазон корректных номеров полей 2-128).

Клиент по умолчанию создает Сообщение ISO8583 с указанными в задании полями 2,6,7,11, но можно добавлять в сообщение любые поля через метод Iso8583::Message::setField(fieldNumber, data).

Клиент может читать данные из текстового файла. Формат:

MTI

FieldNumber1 Data1

FieldNumber2 Data2

FieldNumber2 Data2

...

Если файл не передан, не найден или имеет неправильный формат, то испольуется дефолтное сообщение (зашито в код).

Сообщение передается по протоколу TCP/IP в виде последовательности байт, на стороне получателя оно десериализуется из массива байт обратно в сообщение Iso8583::Message.

На сервере используется тестовый слой логики получения Response Code. Второй слой логики для Response Code согласно заданию накладывают настройки из БД. Настройки читаются из БД при соединении.

Сервер может принимать одновременно множество клиентских соединений.

!!! Ввиду наличия только установленной версии Oracle X64 работоспособность проверялась и гарантируется для сборки проекта под x64.

Т.к под Windows Oracle предоставляет OCCI тольк под MSVC, то тестировалось под Windows только под компилятор MSVC.

-------------------------------------------------------------
BUILD
-------------------------------------------------------------
Порядок сборки через CMake:

Минимальная версия CMake выставлена 3.0.

1) Подключение boost: 

Для гарантии работоспособности установлена минимальная версия 1.72.0, но вероятно решение будет корректно собираться и с более старыми версиями Boost. Минимально поддерживаемую версию можно поменять в CMakeLists для клиента и сервера.

!!! CMake пропишет стандартные пути к линкуемым библиотекам, но вероятно нужно будет в собранном проекте прописать конкретные пути, где фактически собраны нужные LIB для конкретного компилятора и платформы.

2) Подключение OCCI: 

Первым делом нужно установить переменную окружения OCCI_PATH. В нее прописать фактическое расположение include и lib для OCCI.

Для решения была использована СУБД Oracle Express Edition 18 x64. Соответственно OCI тоже версии 18 x64 (библиотека oraocci18/oraocci18d). Именно зависимость от  библиотек oraocci18/oraocci18d прописывается в проект. Должно работать и с другими версиями OCCI.

!!! Аналогично Boost CMake пропишет стандартные пути к линкуемым библиотекам (OCCI_PATH/lib), но вероятно нужно будет в собранном проекте прописать конкретные пути, где фактически собраны нужные LIB для конкретного компилятора и платформы.

-------------------------------------------------------------
Также для Quick-теста под Windows можно использовать собранное решение в директории /test. Там содержится набор батников, которые запускают уже собранные клиент и сервер в отдельных консолях. Тестовый файл входных данных находится в test/data/.

-------------------------------------------------------------
База данных
-------------------------------------------------------------

Для решения была использована СУБД Oracle Express Edition(XE) 18c x64.

Настройки мэппинга Response Code расположены в таблице response_code_settings.

Подключение к базе данных по дефолту через пользователя C##Scott. Пароль - tiger. Настройки подключения зашиты в код в  TcpServer::readResponseCodeSettings(). Там же можно их поменять.

В Server/response_code_settings.sql полный код создания соединения (пользователя, пароля а также всех прав) и заполненной таблицы. Выполняется например под sys as sysdba через клиентскую утилиту Oracle Sql*Plus.

-------------------------------------------------------------
Параметры запуска клиента (TcpIso8583Client)
-------------------------------------------------------------

TcpIso8583Client [/InputDataFile/] [/ClientName/]

Оба параметра необязательные. В случае отсутствия входного файла генерируется дефолтное сообщение. В случае отсутствия имени клиента генерируется дефотное имя.
