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

**Возвращает 200:** `[ { "id": 1, "title": "Молоток", "status": "free" }, { "id": 2, "title": "Книга", "status": "busy" } ]`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/items?id=5
Получить полную информацию о конкретном предмете.

**Принимает (Query):** `?id=5` (Берет userId из куки)

**Возвращает 200:** `{ "id": 5, "title": "Книга", "description": "Интересная", "category": "books", "status": "free" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }` (если ?id= не передан или не число)

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 404:** `{ "ok": false, "message": "Item not found" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`

## POST /api/items
Добавить новое объявление.

**Принимает (JSON):** `{ "title": "string", "description": "string", "category": "string" }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true, "id": 42 }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }` (если JSON сломан)

**Возвращает 400:** `{ "ok": false, "message": "Title cannot be empty" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## PUT /api/items
Обновить существующее объявление.

**Принимает (JSON):** `{ "id": 42, "title": "string", "description": "string", "category": "string" }` (Берет userId из куки)

**Возвращает 200:** `{ "ok": true }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid JSON" }`

**Возвращает 400:** `{ "ok": false, "message": "Invalid category" }`

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

**Возвращает 200:** `[ { "id": 1, "borrowerUsername": "anna", "startDate": "2026-10-20", "endDate": "2026-10-22", "status": "active" } ]`

**Возвращает 400:** `{ "ok": false, "message": "Invalid item id" }`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`


## GET /api/categories
Получить список доступных категорий для выпадающего списка.

**Принимает (JSON):** `{}`

**Возвращает 200:** `[ "electronics", "toys", "books", "tools", "other" ]`

**Возвращает 401:** `{ "ok": false, "message": "Not logged in" }`

**Возвращает 500:** `{ "ok": false, "message": "Internal server error" }`