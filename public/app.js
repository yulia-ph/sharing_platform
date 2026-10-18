// ============ Screen switching ============

function showLogin() {
    document.getElementById('login-screen').classList.add('active');
    document.getElementById('register-screen').classList.remove('active');
    clearMessage('login-message');
    clearMessage('register-message');
}

function showRegister() {
    document.getElementById('register-screen').classList.add('active');
    document.getElementById('login-screen').classList.remove('active');
    clearMessage('login-message');
    clearMessage('register-message');
}

// ============ Messages ============

function showError(elementId, text) {
    const el = document.getElementById(elementId);
    el.textContent = text;
    el.className = 'message error';
}

function showSuccess(elementId, text) {
    const el = document.getElementById(elementId);
    el.textContent = text;
    el.className = 'message success';
}

function clearMessage(elementId) {
    const el = document.getElementById(elementId);
    el.textContent = '';
    el.className = 'message';
}

// ============ Input validation ============

function validateUsername(username) {
    if (!username || username.length < 3 || username.length > 20) {
        return 'Username must be from 3 to 20 characters.';
    }

    return null;
}

function validatePassword(password) {
    if (!password || password.length < 6) {
        return 'Password must be at least 6 characters.';
    }
    return null;
}

function validatePasswordsMatch(p1, p2) {
    if (p1 !== p2) {
        return 'Typo in the second password.';
    }
    return null;
}

// ============ Server requests ============

async function submitLogin(event) {
    event.preventDefault();

    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;

    clearMessage('login-message');

    const err = validateUsername(username) || validatePassword(password);
    if (err) {
        showError('login-message', err);
        return;
    }

    const button = document.querySelector('#login-form button');
    button.disabled = true;

    try {
        const response = await fetch('/api/login', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({username, password})
        });

        const data = await response.json();

        if (data.ok) {
            showSuccess('login-message', 'Logged in successfully!');

        } else {
            showError('login-message', data.message || 'Login failed');
        }
    } catch (e) {
        showError('login-message', 'Server connection error');
    } finally {
        button.disabled = false;
    }
}

async function submitRegister(event) {
    event.preventDefault();

    const username = document.getElementById('register-username').value.trim();
    const password = document.getElementById('register-password').value;
    const passwordDouble = document.getElementById('register-password-double').value;

    clearMessage('register-message');

    const err = validateUsername(username) || validatePassword(password) ||
        validatePasswordsMatch(password, passwordDouble);

    if (err) {
        showError('register-message', err);
        return;
    }

    const button = document.querySelector('#register-form button');
    button.disabled = true;

    try {
        const response = await fetch('/api/register', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({username, password, passwordDouble})
        });

        const data = await response.json();

        if (data.ok) {
            showSuccess('register-message', 'Account created! Switching to log in...');
            setTimeout(() => {
                showLogin();
                document.getElementById('login-username').value = username;
            }, 1000);
        } else {
            showError('register-message', data.message || 'Registration failed');
        }
    } catch (e) {
        showError('register-message', 'Server connection error');
    } finally {
        button.disabled = false;
    }
}

// ============ Event listeners ============

document.getElementById('login-form').addEventListener('submit', submitLogin);
document.getElementById('register-form').addEventListener('submit', submitRegister);