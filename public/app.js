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

function showPage(name) {
    document.querySelectorAll('.page').forEach((page) => {
        page.classList.toggle('active', page.id === 'page-' + name);
    });
    document.querySelectorAll('.nav-btn[data-page]').forEach((button) => {
        button.classList.toggle('active', button.dataset.page === name);
    });
    if (name === 'items') showItemsList();
}

function showApp(username) {
    document.getElementById('hello-name').textContent = username;
    document.getElementById('auth-shell').classList.add('is-hidden');
    document.getElementById('app-screen').classList.add('is-visible');
    showPage('home');
}

function showAuth() {
    document.getElementById('app-screen').classList.remove('is-visible');
    document.getElementById('auth-shell').classList.remove('is-hidden');
    showLogin();
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
            showApp(username);
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

// ============ My items ============

let itemEditingId = null;
let itemDetailId = null;
let currentItem = null;
let categoriesLoaded = false;
let itemsRequest = 0;

function element(tag, className, text) {
    const node = document.createElement(tag);
    if (className) node.className = className;
    if (text != null) node.textContent = text;
    return node;
}

function showItemPanel(name) {
    document.getElementById('items-list-view').hidden = name !== 'list';
    document.getElementById('item-form').hidden = name !== 'form';
    document.getElementById('items-detail-view').hidden = name !== 'detail';
}

async function requestJson(url, options) {
    let response;
    try {
        response = await fetch(url, options);
    } catch (e) {
        throw new Error('Server connection error');
    }

    let data = null;
    try {
        data = await response.json();
    } catch (e) {
        throw new Error('Server connection error');
    }

    if (response.status === 401) {
        showAuth();
        const error = new Error(data.message || 'Not logged in');
        error.status = 401;
        throw error;
    }
    if (!response.ok) {
        const error = new Error((data && data.message) || 'Request failed');
        error.status = response.status;
        throw error;
    }
    return data;
}

function statusLabel(status) {
    return status === 'busy' ? 'Busy' : 'Free';
}

async function ensureCategories() {
    if (categoriesLoaded) return;
    const names = await requestJson('/api/categories');
    const select = document.getElementById('item-category');
    select.replaceChildren();
    names.forEach((name) => {
        const option = document.createElement('option');
        option.value = name;
        option.textContent = name;
        select.appendChild(option);
    });
    categoriesLoaded = true;
}

function renderItemList(items) {
    const list = document.getElementById('item-list');
    list.replaceChildren();
    if (!items.length) {
        list.appendChild(element('p', 'empty-note', 'You have not published anything yet.'));
        return;
    }

    items.forEach((item) => {
        const card = element('button', 'item-card');
        card.type = 'button';
        const text = element('span');
        text.appendChild(element('span', 'item-card-title', item.title));
        text.appendChild(element('span', 'item-card-city', item.city));
        card.appendChild(text);
        card.appendChild(element(
            'span', 'pill pill-' + (item.status === 'busy' ? 'busy' : 'free'),
            statusLabel(item.status)));
        card.addEventListener('click', () => openItem(item.id));
        list.appendChild(card);
    });
}

async function showItemsList(notice) {
    const requestId = ++itemsRequest;
    itemEditingId = null;
    showItemPanel('list');
    clearMessage('items-message');
    if (notice) showSuccess('items-message', notice);

    try {
        const items = await requestJson('/api/my-items');
        if (requestId !== itemsRequest) return;
        renderItemList(items);
    } catch (e) {
        if (e.status === 401 || requestId !== itemsRequest) return;
        showError('items-message', e.message || 'Server connection error');
    }
}

function openDeleteDialog() {
    document.getElementById('item-delete-name').textContent = currentItem ? currentItem.title : '';
    clearMessage('item-delete-message');
    document.getElementById('item-delete-dialog').hidden = false;
}

function closeDeleteDialog() {
    document.getElementById('item-delete-dialog').hidden = true;
}

async function openItemForm(item) {
    clearMessage('item-form-message');
    closeDeleteDialog();

    try {
        await ensureCategories();
    } catch (e) {
        if (e.status === 401) return;
        showItemPanel('form');
        showError('item-form-message', e.message || 'Server connection error');
        return;
    }

    document.getElementById('item-form').reset();
    if (item) {
        itemEditingId = item.id;
        document.getElementById('item-form-heading').textContent = 'Edit item';
        document.getElementById('item-title').value = item.title || '';
        document.getElementById('item-description').value = item.description || '';
        document.getElementById('item-category').value = item.category || '';
        document.getElementById('item-city').value = item.city || '';
    } else {
        itemEditingId = null;
        document.getElementById('item-form-heading').textContent = 'New item';
    }
    showItemPanel('form');
}

function closeItemForm() {
    const id = itemEditingId;
    if (id)
        openItem(id);
    else
        showItemsList();
}

function renderBookings(bookings) {
    const box = document.getElementById('item-detail-bookings');
    box.replaceChildren();
    if (!bookings.length) {
        box.appendChild(element('p', 'empty-note', 'No bookings yet.'));
        return;
    }

    bookings.forEach((booking) => {
        const row = element('div', 'booking-row');
        row.appendChild(element('span', 'booking-user', booking.borrowerUsername));
        row.appendChild(
            element('span', 'booking-dates', booking.startDate + ' to ' + booking.endDate));
        row.appendChild(element('span', 'booking-status', booking.status));
        box.appendChild(row);
    });
}

async function openItem(id) {
    itemDetailId = id;
    currentItem = null;
    closeDeleteDialog();
    clearMessage('item-detail-message');
    showItemPanel('detail');
    document.getElementById('item-detail-title').textContent = '';
    document.getElementById('item-detail-city').textContent = '';
    document.getElementById('item-detail-category').textContent = '';
    document.getElementById('item-detail-description').textContent = '';
    document.getElementById('item-detail-status').textContent = '';
    document.getElementById('item-detail-bookings').replaceChildren();

    try {
        const item = await requestJson('/api/items?id=' + encodeURIComponent(id));
        const bookings = await requestJson('/api/bookings?itemId=' + encodeURIComponent(id));
        if (itemDetailId !== id) return;
        currentItem = item;
        document.getElementById('item-detail-title').textContent = item.title;
        document.getElementById('item-detail-city').textContent = item.city;
        document.getElementById('item-detail-category').textContent = item.category;
        document.getElementById('item-detail-description').textContent =
            item.description || 'No description.';
        const status = document.getElementById('item-detail-status');
        status.textContent = statusLabel(item.status);
        status.className = 'pill pill-' + (item.status === 'busy' ? 'busy' : 'free');
        renderBookings(bookings);
    } catch (e) {
        if (e.status === 401 || itemDetailId !== id) return;
        showError('item-detail-message', e.message || 'Server connection error');
    }
}

async function submitItem(event) {
    event.preventDefault();

    const title = document.getElementById('item-title').value.trim();
    const description = document.getElementById('item-description').value.trim();
    const category = document.getElementById('item-category').value;
    const city = document.getElementById('item-city').value.trim();
    clearMessage('item-form-message');

    if (!title) {
        showError('item-form-message', 'Title cannot be empty');
        return;
    }
    if (title.length > 100) {
        showError('item-form-message', 'Title is too long (max 100 characters)');
        return;
    }
    if (!category) {
        showError('item-form-message', 'Invalid category');
        return;
    }
    if (!city) {
        showError('item-form-message', 'City cannot be empty');
        return;
    }

    const button = document.querySelector('#item-form button[type="submit"]');
    button.disabled = true;
    const editingId = itemEditingId;
    const body = {title, description, category, city};
    if (editingId) body.id = editingId;

    try {
        await requestJson('/api/items', {
            method: editingId ? 'PUT' : 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(body)
        });
        await showItemsList(editingId ? 'Item updated.' : 'Item published.');
    } catch (e) {
        if (e.status === 401) return;
        showError('item-form-message', e.message || 'Server connection error');
    } finally {
        button.disabled = false;
    }
}

async function deleteCurrentItem() {
    const button = document.getElementById('item-delete-yes');
    button.disabled = true;
    clearMessage('item-delete-message');
    try {
        await requestJson('/api/items?id=' + encodeURIComponent(itemDetailId), {method: 'DELETE'});
        closeDeleteDialog();
        await showItemsList('Item deleted.');
    } catch (e) {
        if (e.status === 401) return;
        showError('item-delete-message', e.message || 'Server connection error');
    } finally {
        button.disabled = false;
    }
}

// ============ Event listeners ============

document.getElementById('login-form').addEventListener('submit', submitLogin);
document.getElementById('register-form').addEventListener('submit', submitRegister);
document.getElementById('new-item-button').addEventListener('click', () => openItemForm(null));
document.getElementById('item-form').addEventListener('submit', submitItem);
document.getElementById('item-form-back').addEventListener('click', closeItemForm);
document.getElementById('item-form-cancel').addEventListener('click', closeItemForm);
document.getElementById('item-back').addEventListener('click', () => showItemsList());
document.getElementById('item-edit').addEventListener('click', () => {
    if (currentItem) openItemForm(currentItem);
});
document.getElementById('item-delete').addEventListener('click', openDeleteDialog);
document.getElementById('item-delete-no').addEventListener('click', closeDeleteDialog);
document.getElementById('item-delete-yes').addEventListener('click', deleteCurrentItem);
document.getElementById('item-delete-dialog').addEventListener('click', (event) => {
    if (event.target.id === 'item-delete-dialog') closeDeleteDialog();
});
document.addEventListener('keydown', (event) => {
    if (event.key === 'Escape') closeDeleteDialog();
});

document.getElementById('app-screen').addEventListener('click', (event) => {
    const target = event.target.closest('[data-page]');
    if (!target) return;
    showPage(target.dataset.page);
});

document.getElementById('logout-button').addEventListener('click', async () => {
    try {
        await fetch('/api/logout', {method: 'POST'});
    } catch (e) {
        // The screen still returns to log in if the server is unreachable.
    }
    document.getElementById('login-password').value = '';
    showAuth();
});