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

**Возвращает 200:** `{ "id": 5, "title": "Книга", "description": "Интересная", "category": "books", "city": "Казань", "pricePerDay": 100, "status": "free" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }` (если ?id= не передан или не число)

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 404:** `{ "ok": false, "message": "Item not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`

## POST /api/items
Добавить новое объявление.

**Принимает (JSON):** `{ "title": "string", "description": "string", "category": "string", "city": "string", "pricePerDay": 100.5 }` (Берет userId из куки). `pricePerDay` — положительное число в BYN.

**Возвращает 200:** `{ "ok": true, "id": 42 }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }` (если JSON сломан)

**Возвращает 400:** `{ "ok": false, "message": "Title cannot be empty" }`

**Возвращает 400:** `{ "ok": false, "message": "Title is too long (max 100 characters)" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

**Возвращает 400:** `{ "ok": false, "message": "City cannot be empty" }`

**Возвращает 400:** `{ "ok": false, "message": "Price per day must be a positive number" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## PUT /api/items
Обновить существующее объявление.

**Принимает (JSON):** `{ "id": 42, "title": "string", "description": "string", "category": "string", "city": "string", "pricePerDay": 100.5 }` (Берет userId из куки). `pricePerDay` — положительное число в BYN.

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }`

**Возвращает 400:** `{ "ok": false, "message": "Title cannot be empty" }`

**Возвращает 400:** `{ "ok": false, "message": "Title is too long (max 100 characters)" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

**Возвращает 400:** `{ "ok": false, "message": "City cannot be empty" }`

**Возвращает 400:** `{ "ok": false, "message": "Price per day must be a positive number" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this item" }`

**Возвращает 404:** `{ "ok": false, "message": "Item not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## DELETE /api/items?id=42
Удалить объявление (мягкое удаление). Не проходит, если есть бронь `active` или `overdue`: вещь ещё у заёмщика, ответ 500 с текстом `Internal server error`. Брони `reserved` при удалении становятся `cancelled`.

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

`username` — заёмщик. Перед ответом список обновляет статусы этого предмета и может сдвинуть или бесплатно отменить ближайшую `reserved` бронь, если предыдущая ещё не возвращена.

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/categories
Получить список доступных категорий для выпадающего списка.

**Принимает (JSON):** `{}`

**Возвращает 200:** `[ "electronics", "toys", "for kitchen", "sports", "books", "leisure", "tools", "services", "other" ]`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


# Слой поиска и моих броней

Статус `active` сам не выставляется. Его ставит заёмщик кнопкой «Забрал». Статус `returned` ставит заёмщик кнопкой «Вернул» и подтверждением оплаты: сервер сам считает сумму и помечает её оплаченной. Статус `cancelled` ставит заёмщик или владелец кнопкой «Отменить», пока срок не кончился. Такая отмена счёта не создаёт.

Перед ответом `GET /api/my-bookings`, `GET /api/incoming-bookings`, `GET /api/bookings` и `GET /api/items`, а также при `POST /api/bookings`, `POST /api/bookings/pickup` и `POST /api/bookings/cancel` сервер обновляет брони предмета. `active` с прошедшим `endDate` становится `overdue`. Если просрочки нет, `reserved` с прошедшим `endDate` становится `cancelled`: счёт за дни от `startDate` до `endDate` включительно по 0.25 и дальше не растёт. Если просрочка есть, двигается только ближайшая `reserved`, у которой начало уже наступило: `startDate` становится завтрашним днём, `endDate` тот же. Когда завтра уже позже конца, эта бронь отменяется без счёта, и в том же проходе так же разбирается следующая, чей срок уже наступил. Брони с началом в будущем не двигаются. Заёмщику в бронь пишется `notice`. В ограничении таблицы `bookings.status` должны быть разрешены значения `reserved`, `active`, `overdue`, `returned` и `cancelled`.

Дневная цена берётся с предмета и в ответе не умножается на коэффициент. `amount` — текущая сумма, если бронь ещё открыта, или уже записанный счёт. Пока дата начала не наступила, сумма 0. После начала `reserved` считается по 0.25 за каждый прошедший день. У `active` и `overdue` дни с `startDate` по `endDate`, но не позже сегодня, считаются по 1, а дни после `endDate` по сегодня — по 2. Дни, которые ещё не наступили, в сумму не входят.

Кнопка «Забрал» есть у заёмщика, пока статус `reserved`, сегодня не раньше `startDate` и не позже `endDate`. Кнопка «Вернул» есть у заёмщика, пока статус `active` или `overdue`. Кнопка «Отменить» есть у заёмщика и у владельца, пока статус `reserved` и срок ещё не кончился. Кнопка «Оплатить» есть у заёмщика, когда несобранная бронь уже отменена и счёт ещё не оплачен. У владельца нет кнопок «Забрал», «Вернул» и «Оплатить».

## GET /api/cities
Города для выпадающего списка в поиске. Берутся из неудалённых предметов, без учёта регистра каждый город один раз.

**Принимает:** userId из куки

**Возвращает 200:** `[ "Казань", "Москва" ]`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/search?city=Казань&category=tools
Поиск вещей других людей. Город, категория и строка названия необязательны. Пустой запрос возвращает все неудалённые чужие предметы. Свои предметы в выдачу не входят. Категория и строка — два разных входа: вместе их передавать нельзя. В карточке только `id`, `title` и `city`, без `status`. Подробности и занятые даты открываются в предмете.

**Принимает (Query):** `city`, `category`, `q` (Берет userId из куки). `q` ищет по названию.

**Возвращает 200:** `[ { "id": 5, "title": "Молоток", "city": "Казань" } ]`

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

**Возвращает 400:** `{ "ok": false, "message": "Choose a category or a title" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## POST /api/bookings
Забронировать чужой предмет. Статус новой брони всегда `reserved`.

**Принимает (JSON):** `{ "itemId": 5, "startDate": "2026-10-23", "endDate": "2026-10-25" }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true, "id": 7 }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid date" }` (дата не в формате `YYYY-MM-DD`, `startDate` позже `endDate` или `startDate` раньше сегодня)

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
    "pricePerDay": 100,
    "amount": 0,
    "paid": false,
    "canCancel": true,
    "canPay": false
  }
]
```

`pricePerDay` и `amount` те же, что в `GET /api/my-bookings`. `canCancel` и `canPay` сервер считает сам. У владельца `canPay` всегда `false`. Поля `notice` здесь нет: предупреждение о сдвиге видит заёмщик.

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
    "pricePerDay": 100,
    "amount": 0,
    "paid": false,
    "canPickup": true,
    "canReturn": false,
    "canCancel": true,
    "canPay": false,
    "notice": ""
  }
]
```

`pricePerDay` — дневная цена предмета в BYN, без коэффициента. Это число, дробная часть — копейки. `amount` — сколько выйдет в BYN, если закрыть бронь сегодня, либо уже замороженный счёт. `canPickup`, `canCancel`, `canReturn` и `canPay` сервер считает сам. Клиент показывает кнопки только по этим полям. `notice` — текст, если эту бронь сдвинули или бесплатно отменили из-за чужой просрочки. Пустая строка значит, что предупреждения нет. Сервер текст не стирает: он приходит в каждом ответе, пока строка лежит в базе.

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
Заёмщик вернул вещь и подтвердил оплату. Статус `active` или `overdue` становится `returned`, в `returnedAt` пишется сегодняшняя дата. Сумму считает сервер и сохраняет её как оплаченную. Если срок `active` уже кончился, дни после `endDate` считаются по 2, как у `overdue`, даже если список ещё не успел сменить статус. Дни внутри исходного срока остаются по 1.

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

**Возвращает 400:** `{ "ok": false, "message": "Booking cannot be cancelled" }` (статус уже не `reserved`. Если срок `reserved` вышел сам по себе, бронь становится `cancelled` со счётом по 0.25. Если срок съела чужая просрочка, отмена бесплатная. Ответ в обоих случаях один) 

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this booking" }`

**Возвращает 404:** `{ "ok": false, "message": "Booking not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## POST /api/bookings/pay
Заёмщик подтверждает оплату счёта за бронь, которую не забрали и срок которой уже кончился. Статус остаётся `cancelled`, счёт помечается оплаченным. Сумма уже записана и здесь заново не считается.

**Принимает (JSON):** `{ "bookingId": 1 }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid booking id" }`

**Возвращает 400:** `{ "ok": false, "message": "Nothing to pay" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 403:** `{ "ok": false, "message": "You don't have access to this booking" }`

**Возвращает 404:** `{ "ok": false, "message": "Booking not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`