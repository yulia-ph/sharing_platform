# Стиль кода

## Имена

|Что |	Стиль |	Пример|
|----|--------|-------|
Классы, структуры, enum-типы|	`PascalCase`| `BookingService`, `ItemRepository`, `BookingStatus`|
Методы и функции |	`camelCase`	|`findById`, `createBooking`, `isDateBusy`|
Переменные и параметры|	`camelCase`|	`userId`, `startDate`, `currentUser`|
Приватные поля класса|	`camelCase_` с подчёркиванием в конце	|`db_`, `items_`, `bookings_`|
Константы|	`kPascalCase`	|`kMaxTitleLength`, `kDefaultPort`|
Файлы|	`PascalCase.h/.cpp` для классов, `camelCase.h` для утилит|	`BookingService.h`, `dateUtils.h`|
Enum-значения	|`PascalCase`|	`BookingStatus::Reserved`|
Макросы (если правда нужны)	| `ALL_CAPS`|`#define APP_VERSION "1.0"`|

## Заголовки

- Каждый .cpp включает свой .h первым.
- Дальше — стандартная библиотека, потом сторонние, потом свои.
- В .h — #pragma once.
- В .h — минимум #include, максимум forward declaration.

```
// BookingService.cpp
#include "BookingService.h"

#include <string>
#include <vector>

#include "ItemRepository.h"
#include "BookingRepository.h"
```

## Форматтер

Всем поставить расширение clang-format в редакторе и включить «format on save».

## Git

- Ветки: `feature/booking-service`, `fix/date-validation`, `style/code-format`.
- Коммиты по-английски.
- Формат коммита: [область] краткое описание. Пример: `[booking] fix date overlap check`.