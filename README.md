# Oптимизированная криптосистема с открытым ключом(RSA)
Консольная программа, позволяющая:
1. Генерировать ключи заданной длины.
2. Подписывать заданный файл.
3. Проверять подпись.
4. Шифровать заданный файл.
5. Расшифровывать файл.
    
Программа позволяет работать с файлами любой длины.
Программа является утилитой командной строки.
[имя_программы] --help – выводит как ей пользоваться

[имя_программы] genkey --size 1024 --pubkey pk.txt --secret secret.txt
Генерирует ключи. Открытый сохраняется в pk.txt, закрытый – в secret.txt. Размер может быть любой, кратный 256: 256, 512, 1024, 2048.

[имя_программы] sign --infile file.txt --secret secret.txt --sigfile signature.txt
Вычислят подпись файла, сохраняет ее в signature.txt.

[имя_программы] check --infile file.txt --pubkey pk.txt --sigfile signature.txt
Проверяет подпись файла, результат пишет в stdout.

[имя_программы]  --infile file.txt --pubkey pk.txt --outfile file_out.txt
Шифрует файл для получателя. Результат помещает в file_out.txt

[имя_программы] --infile file.txt.enc --secret secret.txt --outfile file.txt
Расшифровывает файл. Результат помещает в file.txt.

#Итог

Оценка оптимизаций на разных этапах производилась путем сравнение  свободных клетотаблиц времени к ключу,
затраченного на зашифаровку, расшифровку файла.
## Время работы алгоритма без оптимизации
![image](https://user-images.githubusercontent.com/60771708/213115475-275a2153-fa5a-4b1d-b201-bd233bb8b210.png)

На этом и последующих рисунках будет показываться размер ключа в битах, время в секундах, минутах и часах, в первом, втором, третьем и четвертом столбце соответсветнно. Кроме того, 
в последующих столбцах приводятся сравнения по затраченному времени относительно других результатов 
(времени без оптимизаций (эталон) или промежуточных результатов).
## Оптимизация на уровне алгоритма
Для данной оптимизации был выбран алгоритм перемножения многочленов методом быстрого преобразования Фурье 
Для перемножения чисел в исходной программе используется метод Карацубы, сложность алгоритма которого: O(n^(log_2(3)))

![image](https://user-images.githubusercontent.com/60771708/213115826-d85d1c1f-1539-4502-9dc0-370d65b2176f.png)

## Машинно-независимая оптимизация 
В машинно-независимых оптимизациях были свернуты некоторые циклы, развернут короткий цикл и заменен цикл for на внутреннию функцию языка Си (memcpy)

![image](https://user-images.githubusercontent.com/60771708/213116671-c953356e-c3e7-4f6c-b993-de891411b254.png)

## Ассемлерная вставка 
Время выполнения программы с aсселблерной вставкой незначительно уменьшилось.

![image](https://user-images.githubusercontent.com/60771708/213116854-45b1f804-e08c-4391-bc44-19faa44ff633.png)

## Машино-зависимая оптимизация

Время выполнения программы с машинно-зависимой оптимизацией незначительно увеличилось.

![image](https://user-images.githubusercontent.com/60771708/213117305-bdbbeeb9-370f-4e1d-a5da-6012656d05e0.png)

## Финальное время выполнения
![image](https://user-images.githubusercontent.com/60771708/213117514-932f3197-87ec-4e5e-ad84-4ce4ce9f487c.png)




