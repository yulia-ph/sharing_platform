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
| Реализовать страницу входа| daradoroncova1-debug|✅ Готово|
| Прописать js для страницы входа| daradoroncova1-debug|✅ Готово|
| Определить интерфейсы для ItemService| arin-aa, yulia-ph | ✅ Готово|
| Определить интерфейсы для ItemRepository| arin-aa, yulia-ph | ✅ Готово|
| Реализовать первоначальную проверку ввода| daradoroncova1-debug|✅ Готово|
| Реализовать ItemService| yulia-ph | ✅ Готово|
| Согласовать эндпоинты| yulia-ph, daradoroncova1-debug | ✅ Готово|
| Реализовать функционал сервера для объявлений| yulia-ph | ✅ Готово|
| Реализовать ItemRepository| arin-aa| ✅ Готово|
| Реализовать первичный BookingRepository| arin-aa| ✅ Готово|
| Реализовать страницу объявлений| daradoroncova1-debug| -|
| Определить интерфейсы для BookingService для броней| arin-aa, yulia-ph | -|
| Определить интерфейсы для BookingRepository для броней| arin-aa, yulia-ph | -|
| Реализовать BookingService для броней| yulia-ph | -|
| Реализовать страницу броней| daradoroncova1-debug| -|
| Соглавовать работу над поиском| arin-aa, yulia-ph, daradoroncova1-debug | -|
| Соглавовать функционал главной страницы| arin-aa, yulia-ph, daradoroncova1-debug | -|
| | |
