# Слой входа

## POST /api/register

Регистрация нового пользователя.

**Принимает (JSON):**
`{
    "username": "string",
    "password": "string",
    "passwordDouble": "string"
}`

**Возвращает 200:**
`{ "ok": true }`

**Возвращает 400:**
`{ "ok": false, "message": "Username must be from 3 to 20 characters" }`


## POST /api/login

Вход в аккаунт.

**Принимает (JSON):**
`{
    "username": "string",
    "password": "string"
}`

**Возвращает 200:**
`Set-Cookie: sessionId=abc123; Path=/; HttpOnly`
`{ "ok": true }`

**Возвращает 400:**
`{ "ok": false, "message": "Username must be from 3 to 20 characters" }`

# Слой 'мои объявления'
