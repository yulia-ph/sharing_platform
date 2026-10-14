# Шэринговая платформа

Вэб приложение на C++ для бесплатного шеринга вещей.
Пользователи публикуют объявления, ищут вещи по категориям,
бронируют их на определённые даты.

## Команда

- **yulia-ph** — координация проекта, логика, утилиты дат, сервер
- **arin-aa** — база данных: схема, репозитории, миграции
- **daradoroncova1-debug** — интерфейс: html, js, css; хеширование паролей


## To-do
|Задача  | Ответственный | Статус |
|---|---|---|
| Создать структуру файлов | yulia-ph | ✅ Готово |
| Определить стиль кода | arin-aa, yulia-ph, daradoroncova1-debug | ✅ Готово |
| Создать docs/style.md., .clang-format в корне, gitignore. | yulia-ph | ✅ Готово |
| Определить структуры сущностей | arin-aa, yulia-ph, daradoroncova1-debug | ✅ Готово |
| Создать src/domain/Entities.h, Enums.h, Result.h | yulia-ph | ✅ Готово |
| Определить интерфейсы для входа| arin-aa, yulia-ph, daradoroncova1-debug | ✅ Готово |
| Создать docs/schema.sql | arin-aa | ✅ Готово |
| Создать базовые заголовочные файлы| yulia-ph|✅ Готово|
| Открытие/закрытие БД, exec, транзакции| arin-aa|✅ Готово|
| Создать все таблицы и индексы|arin-aa |✅ Готово|
| Реализовать UserRepository| arin-aa|✅ Готово|
| Реализовать AuthService| yulia-ph|✅ Готово|
| Утилиты даты |yulia-ph|✅ Готово|
| Хеширование пароля| daradoroncova1-debug|✅ Готово|
| Сервер для входа| yulia-ph|✅ Готово|
| Определить эндпоинты для входа| yulia-ph, daradoroncova1-debug|✅ Готово|
| Реализовать страницу входа| daradoroncova1-debug|-|
| Прописать js для страницы входа| daradoroncova1-debug|-|
| Определить интерфейсы для ItemService| arin-aa, yulia-ph | ✅ Готово|
| Реализовать ItemService| yulia-ph | -|
| Определить интерфейсы для ItemRepository| arin-aa, yulia-ph | ✅ Готово|
| Реализовать ItemRepository| arin-aa| -|
| Реализовать функции первоначальной проверки ввода| daradoroncova1-debug| -|
| | |
