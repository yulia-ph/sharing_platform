# Слой входа и авторизации

## POST /api/register
Регистрация нового пользователя.

**Принимает (JSON):**
`{ "username": "string", "password": "string", "passwordDouble": "string" }`

**Возвращает 200:**
`{ "ok": true }`

**Возвращает 400 (Ошибки валидации):**

`{ "ok": false, "message": "Invalid JSON" }` (если JSON сломан)

`{ "ok": false, "message": "Username must be from 3 to 20 characters" }`

`{ "ok": false, "message": "Username must be all letters, numbers and _" }`

`{ "ok": false, "message": "Typo in the second password" }`

`{ "ok": false, "message": "Password must be at least 6 characters" }`

`{ "ok": false, "message": "This username is already taken" }`

**Возвращает 500:**
`{ "ok": false, "message": "Internal server error" }`

## POST /api/login
Вход в аккаунт.

**Принимает (JSON):**
`{ "username": "string", "password": "string" }`

**Возвращает 200:**
`Set-Cookie: sessionId=abc123; Path=/; HttpOnly`
`{ "ok": true }`

**Возвращает 400:**
`{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 401 (Ошибка авторизации):**
`{ "ok": false, "message": "Wrong username or password" }` 
*(Мы не говорим, что именно не так — имя или пароль, в целях безопасности)*

**Возвращает 500:**
`{ "ok": false, "message": "Internal server error" }`


## POST /api/logout
Выход из аккаунта. Удаляет сессию на сервере и стирает куки в браузере.

**Принимает (JSON):** `{}` (Берет sessionId из куки)

**Возвращает 200:**
`Set-Cookie: sessionId=; Path=/; HttpOnly; Max-Age=0`
`{ "ok": true }`

**Возвращает 500:**
`{ "ok": false, "message": "Internal server error" }`


# Слой мои объявления

## GET /api/my-items
Получить список моих объявлений (сокращенная структура).

**Принимает (JSON):** `{}` (Берет userId из куки)

**Возвращает 200:** `[ { "id": 1, "title": "Молоток", "city": "Казань", "status": "free" }, { "id": 2, "title": "Книга", "city": "Москва", "status": "busy" } ]`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/items?id=5
Получить полную информацию о конкретном предмете.

**Принимает (Query):** `?id=5` (Берет userId из куки)

**Возвращает 200:** `{ "id": 5, "title": "Книга", "description": "Интересная", "category": "books", "city": "Казань", "status": "free" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }` (если ?id= не передан или не число)

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 404:** `{ "ok": false, "message": "Item not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`

## POST /api/items
Добавить новое объявление.

**Принимает (JSON):** `{ "title": "string", "description": "string", "category": "string", "city": "string" }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true, "id": 42 }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }` (если JSON сломан)

**Возвращает 400:** `{ "ok": false, "message": "Title cannot be empty" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

**Возвращает 400:** `{ "ok": false, "message": "City cannot be empty" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## PUT /api/items
Обновить существующее объявление.

**Принимает (JSON):** `{ "id": 42, "title": "string", "description": "string", "category": "string", "city": "string" }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

**Возвращает 400:** `{ "ok": false, "message": "City cannot be empty" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this item" }`

**Возвращает 404:** `{ "ok": false, "message": "Item not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## DELETE /api/items?id=42
Удалить объявление (мягкое удаление).

**Принимает (Query):** `?id=42` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this item" }`

**Возвращает 404:** `{ "ok": false, "message": "Item not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/bookings?itemId=5
Получить список всех броней для конкретного предмета (сокращенная структура).

**Принимает (Query):** `?itemId=5`

**Возвращает 200:** `[ { "id": 1, "username": "anna", "startDate": "2026-10-20", "endDate": "2026-10-22", "status": "active" } ]`

`username` — заёмщик. Этот список не переводит просроченные брони в `overdue`.

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/categories
Получить список доступных категорий для выпадающего списка.

**Принимает (JSON):** `{}`

**Возвращает 200:** `[ "electronics", "toys", "for kitchen", "sports", "books", "leisure", "tools", "services", "other" ]`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


# Слой главной, поиска и моих броней

Статус `active` сам не выставляется. Его ставит заёмщик кнопкой «Забрал». Статус `returned` ставит заёмщик кнопкой «Вернул». Статус `cancelled` ставит заёмщик или владелец кнопкой «Отменить». Автоматически сервер меняет только просрочку.

Перед ответом `GET /api/my-bookings` и `GET /api/incoming-bookings` сервер переводит в `overdue` брони, у которых `endDate` меньше сегодняшней даты и статус `reserved` или `active`. `GET /api/home` делает то же перед своим ответом. В ограничении таблицы `bookings.status` должны быть разрешены значения `reserved`, `active`, `overdue`, `returned` и `cancelled`.

Кнопка «Забрал» есть у заёмщика, пока статус `reserved`, сегодня не раньше `startDate` и не позже `endDate`. Кнопка «Вернул» есть у заёмщика, пока статус `active` или `overdue`. Кнопка «Отменить» есть у заёмщика и у владельца, пока статус `reserved` (без ограничения по датам). У владельца нет кнопок «Забрал» и «Вернул».

## GET /api/home
Главная после входа: числа по сайту и сегодняшние дела.

**Принимает:** userId из куки

**Возвращает 200:**
```
{
  "stats": { "users": 12, "items": 40, "bookings": 15 },
  "today": [
    { "bookingId": 1, "itemId": 5, "title": "Молоток", "city": "Казань", "kind": "pickup", "username": "anna", "canAct": true, "canCancel": true },
    { "bookingId": 2, "itemId": 6, "title": "Книга", "city": "Казань", "kind": "return", "username": "anna", "canAct": true, "canCancel": false },
    { "bookingId": 3, "itemId": 7, "title": "Палатка", "city": "Москва", "kind": "pickup", "username": "ivan", "canAct": false, "canCancel": false }
  ]
}
```

`stats.users` — все пользователи. `stats.items` — предметы с `is_deleted = 0`. `stats.bookings` — все брони.

`today` — брони, у которых сегодня `startDate` (`kind` = `pickup`, статус ещё `reserved`) или сегодня `endDate` (`kind` = `return`, статус `active`). `username` — второй участник. `canAct` истинно только у заёмщика: по этому полю главная показывает кнопку «Забрал» или «Вернул». `canCancel` истинно только у заёмщика при `kind` = `pickup` (статус `reserved`): по этому полю показывается рядом с «Забрал» кнопка «Отменить». У владельца оба флага всегда `false`; отмена ещё не начавшейся брони доступна только на странице «Мои брони».


Сюда же, даже если дата конца уже прошла, попадают просроченные брони заёмщика со статусом `overdue` и `kind` = `return`, чтобы с главной можно было нажать «Вернул».

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/cities
Города для выпадающего списка в поиске. Берутся из неудалённых предметов, без учёта регистра каждый город один раз.

**Принимает:** userId из куки

**Возвращает 200:** `[ "Казань", "Москва" ]`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/search?city=Казань&category=tools
Поиск вещей других людей. Оба параметра необязательны. Пустой запрос возвращает все неудалённые чужие предметы. Свои предметы в выдачу не входят.

**Принимает (Query):** `city`, `category` (Берет userId из куки)

**Возвращает 200:** `[ { "id": 5, "title": "Молоток", "category": "tools", "city": "Казань", "status": "free" } ]`

`status` = `busy`, если у предмета есть бронь со статусом `reserved`, `active` или `overdue`. Иначе `free`. Занятую вещь видно, но создать пересекающуюся бронь нельзя.

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## POST /api/bookings
Забронировать чужой предмет. Статус новой брони всегда `reserved`.

**Принимает (JSON):** `{ "itemId": 5, "startDate": "2026-10-23", "endDate": "2026-10-25" }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true, "id": 7 }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid date" }` (дата не в формате `YYYY-MM-DD` или `startDate` позже `endDate`)

**Возвращает 400:** `{ "ok": false, "message": "You cannot book your own item" }`

**Возвращает 400:** `{ "ok": false, "message": "These dates are busy" }` (пересечение с бронью `reserved`, `active` или `overdue`)

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 404:** `{ "ok": false, "message": "Item not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/incoming-bookings
Все брони, где текущий пользователь владелец. `username` — заёмщик. Отменить можно, пока статус `reserved`.

**Принимает:** userId из куки

**Возвращает 200:**
```
[
  {
    "bookingId": 1,
    "title": "Молоток",
    "city": "Казань",
    "username": "anna",
    "startDate": "2026-10-23",
    "endDate": "2026-10-25",
    "status": "reserved",
    "canCancel": true
  }
]
```

`canCancel` сервер считает сам. Клиент показывает кнопки только по этим полям.

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/my-bookings
Все брони, где текущий пользователь заёмщик. `username` — владелец предмета. Здесь те же кнопки, что на главной, но список не ограничен сегодняшним днём.


**Принимает:** userId из куки

**Возвращает 200:**
```
[
  {
    "bookingId": 1,
    "title": "Молоток",
    "city": "Казань",
    "username": "anna",
    "startDate": "2026-10-23",
    "endDate": "2026-10-25",
    "status": "reserved",
    "canPickup": true,
    "canReturn": false,
    "canCancel": true
  }
]
```

`canPickup`, `canCancel` и `canReturn` сервер считает сам. Клиент показывает кнопки только по этим полям.

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## POST /api/bookings/pickup
Заёмщик забрал вещь. Статус `reserved` становится `active`.

**Принимает (JSON):** `{ "bookingId": 1 }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid booking id" }`

**Возвращает 400:** `{ "ok": false, "message": "Booking cannot be picked up" }` (статус не `reserved`, сегодня раньше `startDate` или бронь уже просрочена)

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this booking" }`

**Возвращает 404:** `{ "ok": false, "message": "Booking not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## POST /api/bookings/return
Заёмщик вернул вещь. Статус `active` или `overdue` становится `returned`, в `returnedAt` пишется сегодняшняя дата.

**Принимает (JSON):** `{ "bookingId": 1 }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid booking id" }`

**Возвращает 400:** `{ "ok": false, "message": "Booking cannot be returned" }` (статус не `active` и не `overdue`)

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this booking" }`

**Возвращает 404:** `{ "ok": false, "message": "Booking not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## POST /api/bookings/cancel
Заёмщик или владелец отменил бронь. Статус `reserved` становится `cancelled`. Вещь освобождается для новых броней, если на неё нет других активных броней.

**Принимает (JSON):** `{ "bookingId": 1 }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid booking id" }`

**Возвращает 400:** `{ "ok": false, "message": "Booking cannot be cancelled" }` (статус не `reserved`) 

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this booking" }`

**Возвращает 404:** `{ "ok": false, "message": "Booking not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`