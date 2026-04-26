/*
 * LibraVault — Frontend Logic
 * Communicates with the C++ HTTP server on port 8080
 * Includes embedded sample data for offline/demo usage
 */

const API = 'http://localhost:8080/api';

// ===== Embedded Sample Data =====
// Mirrors the C++ backend's initSampleData() so pages render even without the server

const SAMPLE_ITEMS = [
    { id:1,  title:"The C++ Programming Language", type:"Book", available:true, yearPublished:2013, author:"Bjarne Stroustrup", isbn:"978-0321563842", genre:"Programming" },
    { id:2,  title:"Clean Code", type:"Book", available:true, yearPublished:2008, author:"Robert C. Martin", isbn:"978-0132350884", genre:"Software Engineering" },
    { id:3,  title:"Design Patterns", type:"Book", available:true, yearPublished:1994, author:"Gang of Four", isbn:"978-0201633610", genre:"Computer Science" },
    { id:4,  title:"Introduction to Algorithms", type:"Book", available:true, yearPublished:2009, author:"Thomas H. Cormen", isbn:"978-0262033848", genre:"Algorithms" },
    { id:5,  title:"The Pragmatic Programmer", type:"Book", available:true, yearPublished:2019, author:"David Thomas", isbn:"978-0135957059", genre:"Software Development" },
    { id:6,  title:"Effective Modern C++", type:"Book", available:true, yearPublished:2014, author:"Scott Meyers", isbn:"978-1491903995", genre:"Programming" },
    { id:7,  title:"Data Structures and Algorithm Analysis", type:"Book", available:true, yearPublished:2011, author:"Mark Allen Weiss", isbn:"978-0132847377", genre:"Data Structures" },
    { id:8,  title:"Artificial Intelligence: A Modern Approach", type:"Book", available:true, yearPublished:2020, author:"Stuart Russell", isbn:"978-0134610993", genre:"Artificial Intelligence" },
    { id:9,  title:"Operating System Concepts", type:"Book", available:true, yearPublished:2018, author:"Abraham Silberschatz", isbn:"978-1119320913", genre:"Operating Systems" },
    { id:10, title:"Computer Networking: A Top-Down Approach", type:"Book", available:true, yearPublished:2016, author:"James Kurose", isbn:"978-0133594140", genre:"Networking" },
    { id:11, title:"Database System Concepts", type:"Book", available:true, yearPublished:2019, author:"Abraham Silberschatz", isbn:"978-0078022159", genre:"Databases" },
    { id:12, title:"Structure and Interpretation of Computer Programs", type:"Book", available:true, yearPublished:1996, author:"Harold Abelson", isbn:"978-0262510875", genre:"Computer Science" },
    { id:13, title:"The Art of Computer Programming", type:"Book", available:true, yearPublished:2011, author:"Donald Knuth", isbn:"978-0201896831", genre:"Algorithms" },
    { id:14, title:"Compilers: Principles, Techniques, and Tools", type:"Book", available:true, yearPublished:2006, author:"Alfred Aho", isbn:"978-0321486813", genre:"Compilers" },
    { id:15, title:"Discrete Mathematics and Its Applications", type:"Book", available:true, yearPublished:2018, author:"Kenneth Rosen", isbn:"978-1259676512", genre:"Mathematics" },
    { id:16, title:"Computer Organization and Design", type:"Book", available:true, yearPublished:2013, author:"David Patterson", isbn:"978-0124077263", genre:"Computer Architecture" },
    { id:17, title:"Software Engineering", type:"Book", available:true, yearPublished:2015, author:"Ian Sommerville", isbn:"978-0133943030", genre:"Software Engineering" },
    { id:18, title:"Machine Learning", type:"Book", available:true, yearPublished:2022, author:"Tom Mitchell", isbn:"978-1259096952", genre:"Machine Learning" },
    { id:19, title:"IEEE Spectrum", type:"Magazine", available:true, yearPublished:2024, issueNumber:42, publisher:"IEEE", category:"Technology" },
    { id:20, title:"Nature", type:"Magazine", available:true, yearPublished:2024, issueNumber:615, publisher:"Springer Nature", category:"Science" },
    { id:21, title:"ACM Computing Surveys", type:"Magazine", available:true, yearPublished:2024, issueNumber:88, publisher:"ACM", category:"Computer Science" },
    { id:22, title:"Scientific American", type:"Magazine", available:true, yearPublished:2024, issueNumber:330, publisher:"Springer Nature", category:"Science" }
];

const SAMPLE_MEMBERS = [
    { memberId:101, name:"Alice Johnson", email:"alice@uni.edu", phone:"555-0101", type:"Standard", borrowLimit:3, borrowedCount:0, borrowedItems:[] },
    { memberId:102, name:"Charlie Brown", email:"charlie@uni.edu", phone:"555-0103", type:"Standard", borrowLimit:3, borrowedCount:0, borrowedItems:[] },
    { memberId:103, name:"Bob Smith", email:"bob@uni.edu", phone:"555-0102", type:"Premium", borrowLimit:6, borrowedCount:0, borrowedItems:[] },
    { memberId:104, name:"Diana Ross", email:"diana@uni.edu", phone:"555-0104", type:"Standard", borrowLimit:3, borrowedCount:0, borrowedItems:[] },
    { memberId:105, name:"Eve Williams", email:"eve@uni.edu", phone:"555-0105", type:"Premium", borrowLimit:6, borrowedCount:0, borrowedItems:[] }
];

function getSampleStats() {
    const books = SAMPLE_ITEMS.filter(i => i.type === 'Book').length;
    const mags = SAMPLE_ITEMS.filter(i => i.type === 'Magazine').length;
    const borrowed = SAMPLE_ITEMS.filter(i => !i.available).length;
    return {
        totalItems: SAMPLE_ITEMS.length,
        totalBooks: books,
        totalMagazines: mags,
        available: SAMPLE_ITEMS.length - borrowed,
        borrowed: borrowed,
        totalMembers: SAMPLE_MEMBERS.length
    };
}

// Track whether server is online
let serverOnline = false;

// ===== Navigation =====
const navLinks = document.querySelectorAll('.nav-link');
const pages = document.querySelectorAll('.page');
const pageTitle = document.getElementById('pageTitle');
const titles = { dashboard:'Dashboard', catalogue:'Catalogue', members:'Members', transactions:'Borrow / Return', search:'Search' };

navLinks.forEach(link => {
    link.addEventListener('click', e => {
        e.preventDefault();
        const page = link.dataset.page;
        navLinks.forEach(l => l.classList.remove('active'));
        link.classList.add('active');
        pages.forEach(p => p.classList.remove('active'));
        document.getElementById('page-' + page).classList.add('active');
        pageTitle.textContent = titles[page] || page;
        if (page === 'dashboard') loadStats();
        if (page === 'catalogue') loadItems();
        if (page === 'members') loadMembers();
        if (page === 'transactions') loadTransactionData();
        // Close mobile sidebar
        document.getElementById('sidebar').classList.remove('open');
    });
});

document.getElementById('menuToggle').addEventListener('click', () => {
    document.getElementById('sidebar').classList.toggle('open');
});

// ===== API Helper =====
async function api(path, options = {}) {
    try {
        const res = await fetch(API + path, options);
        const data = await res.json();
        // Mark server as online
        serverOnline = true;
        document.querySelector('.status-badge').classList.remove('error');
        document.querySelector('.status-badge span:last-child').textContent = 'Server Connected';
        return data;
    } catch (err) {
        serverOnline = false;
        document.querySelector('.status-badge').classList.add('error');
        document.querySelector('.status-badge span:last-child').textContent = 'Offline — Demo Mode';
        throw err;
    }
}


// ===== Dashboard =====
function renderStats(s) {
    document.getElementById('statTotalItems').textContent = s.totalItems;
    document.getElementById('statBooks').textContent = s.totalBooks;
    document.getElementById('statMagazines').textContent = s.totalMagazines;
    document.getElementById('statAvailable').textContent = s.available;
    document.getElementById('statBorrowed').textContent = s.borrowed;
    document.getElementById('statMembers').textContent = s.totalMembers;
    // Animate numbers
    document.querySelectorAll('.stat-value').forEach(el => {
        el.style.animation = 'none';
        el.offsetHeight;
        el.style.animation = 'fadeIn 0.5s ease';
    });
}

async function loadStats() {
    try {
        const s = await api('/stats');
        renderStats(s);
    } catch (e) {
        // Fallback to sample data
        renderStats(getSampleStats());
    }
}

// ===== Catalogue =====
let allItems = [];
let currentCatalogueFilter = 'all';

async function loadItems() {
    try {
        allItems = await api('/items');
    } catch (e) {
        // Fallback to sample data
        allItems = JSON.parse(JSON.stringify(SAMPLE_ITEMS));
    }
    renderItems();
}

function renderItems() {
    const grid = document.getElementById('itemsGrid');
    const searchInput = document.getElementById('catalogueSearchInput').value.toLowerCase();
    const searchBy = document.getElementById('catalogueSearchBy').value;

    let filtered = currentCatalogueFilter === 'all' ? allItems : allItems.filter(i => i.type === currentCatalogueFilter);

    if (searchInput) {
        filtered = filtered.filter(item => {
            if (searchBy === 'title') {
                return item.title && item.title.toLowerCase().includes(searchInput);
            } else if (searchBy === 'author') {
                return item.author && item.author.toLowerCase().includes(searchInput);
            }
            return true;
        });
    }

    grid.innerHTML = filtered.map(item => {
        const isBook = item.type === 'Book';
        return `
        <div class="item-card">
            <span class="item-type ${isBook ? 'book' : 'magazine'}">${item.type}</span>
            <h3>${item.title}</h3>
            <div class="item-meta">
                ${isBook ? `<span><strong>Author:</strong> ${item.author}</span>
                <span><strong>ISBN:</strong> ${item.isbn}</span>
                <span><strong>Genre:</strong> ${item.genre}</span>` :
                `<span><strong>Issue #:</strong> ${item.issueNumber}</span>
                <span><strong>Publisher:</strong> ${item.publisher}</span>
                <span><strong>Category:</strong> ${item.category}</span>`}
                <span><strong>Year:</strong> ${item.yearPublished}</span>
                <span><strong>ID:</strong> ${item.id}</span>
            </div>
            <span class="item-status ${item.available ? 'available' : 'borrowed'}">
                ${item.available ? 'Available' : 'Borrowed'}
            </span>
        </div>`;
    }).join('');
}

// Search & Filter Listeners
document.getElementById('catalogueSearchInput').addEventListener('input', renderItems);
document.getElementById('catalogueSearchBy').addEventListener('change', renderItems);

document.querySelectorAll('.filter-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        currentCatalogueFilter = btn.dataset.filter;
        renderItems();
    });
});


// ===== Members =====
let allMembers = [];

function renderMemberCards(members) {
    const grid = document.getElementById('membersGrid');
    const colors = ['#6366f1','#ec4899','#10b981','#f59e0b','#06b6d4','#8b5cf6'];
    grid.innerHTML = members.map((m, i) => `
    <div class="member-card">
        <div class="member-avatar" style="background:linear-gradient(135deg,${colors[i%colors.length]},${colors[(i+2)%colors.length]})">
            ${m.name.charAt(0)}
        </div>
        <h3>${m.name}</h3>
        <span class="member-badge ${m.type.toLowerCase()}">${m.type}</span>
        <div class="member-info">
            <span>📧 ${m.email}</span>
            <span>📱 ${m.phone}</span>
            <span>📚 Borrowed: ${m.borrowedCount} / ${m.borrowLimit}</span>
            <span>🆔 ID: ${m.memberId}</span>
        </div>
    </div>`).join('');
}

async function loadMembers() {
    try {
        allMembers = await api('/members');
    } catch (e) {
        // Fallback to sample data
        allMembers = JSON.parse(JSON.stringify(SAMPLE_MEMBERS));
    }
    renderMemberCards(allMembers);
}

// ===== Transactions =====
function populateTransactionDropdowns(items, members) {
    // Borrow: show available items
    const borrowItem = document.getElementById('borrowItemId');
    borrowItem.innerHTML = items.filter(i => i.available).map(i =>
        `<option value="${i.id}">[${i.type}] ${i.title} (ID:${i.id})</option>`
    ).join('') || '<option disabled>No items available</option>';

    // Members dropdown for borrow
    const borrowMember = document.getElementById('borrowMemberId');
    borrowMember.innerHTML = members.map(m =>
        `<option value="${m.memberId}">${m.name} [${m.type}] (ID:${m.memberId})</option>`
    ).join('');

    // Return: show members who have borrowed items
    const returnMember = document.getElementById('returnMemberId');
    const borrowers = members.filter(m => m.borrowedCount > 0);
    returnMember.innerHTML = borrowers.map(m =>
        `<option value="${m.memberId}">${m.name} (${m.borrowedCount} items)</option>`
    ).join('') || '<option disabled>No active borrows</option>';

    // Update return items when member changes
    updateReturnItems(items, members);
    returnMember.addEventListener('change', () => updateReturnItems(items, members));
}

async function loadTransactionData() {
    try {
        const [items, members] = await Promise.all([api('/items'), api('/members')]);
        populateTransactionDropdowns(items, members);
    } catch (e) {
        // Fallback to sample data
        populateTransactionDropdowns(
            JSON.parse(JSON.stringify(SAMPLE_ITEMS)),
            JSON.parse(JSON.stringify(SAMPLE_MEMBERS))
        );
    }
}

function updateReturnItems(items, members) {
    const memberId = parseInt(document.getElementById('returnMemberId').value);
    const member = members.find(m => m.memberId === memberId);
    const returnItem = document.getElementById('returnItemId');
    if (member && member.borrowedItems && member.borrowedItems.length > 0) {
        returnItem.innerHTML = member.borrowedItems.map(id => {
            const item = items.find(i => i.id === id);
            return item ? `<option value="${id}">[${item.type}] ${item.title} (ID:${id})</option>` :
                          `<option value="${id}">Item ID: ${id}</option>`;
        }).join('');
    } else {
        returnItem.innerHTML = '<option disabled>No borrowed items</option>';
    }
}

// Borrow form
document.getElementById('borrowForm').addEventListener('submit', async e => {
    e.preventDefault();
    const memberId = document.getElementById('borrowMemberId').value;
    const itemId = document.getElementById('borrowItemId').value;
    const resultBox = document.getElementById('transactionResult');
    try {
        const data = await api('/borrow', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: `memberId=${memberId}&itemId=${itemId}`
        });
        resultBox.style.display = 'block';
        resultBox.className = 'result-box ' + (data.success ? 'success' : 'error');
        resultBox.textContent = data.message;
        if (data.success) {
            loadTransactionData();
        }
    } catch (err) {
        resultBox.style.display = 'block';
        resultBox.className = 'result-box error';
        resultBox.textContent = 'Server error. Is the C++ server running?';
    }
});

// Return form
document.getElementById('returnForm').addEventListener('submit', async e => {
    e.preventDefault();
    const memberId = document.getElementById('returnMemberId').value;
    const itemId = document.getElementById('returnItemId').value;
    const days = document.getElementById('daysOverdue').value;
    const resultBox = document.getElementById('transactionResult');
    try {
        const data = await api('/return', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: `memberId=${memberId}&itemId=${itemId}&daysOverdue=${days}`
        });
        resultBox.style.display = 'block';
        resultBox.className = 'result-box ' + (data.success ? 'success' : 'error');
        resultBox.textContent = data.message;
        if (data.success) {
            loadTransactionData();
        }
    } catch (err) {
        resultBox.style.display = 'block';
        resultBox.className = 'result-box error';
        resultBox.textContent = 'Server error.';
    }
});

// ===== Search =====
document.getElementById('searchForm').addEventListener('submit', async e => {
    e.preventDefault();
    const id = parseInt(document.getElementById('searchId').value);
    const resultDiv = document.getElementById('searchResult');

    // Try server first, fallback to local data
    let item = null;
    try {
        item = await api('/items/search?id=' + id);
    } catch (err) {
        // Fallback: search in local sample data
        item = SAMPLE_ITEMS.find(i => i.id === id) || { error: "Item not found" };
    }

    if (item.error) {
        resultDiv.innerHTML = `<div class="result-box error">❌ ItemNotFoundException: Item with ID ${id} not found in catalogue.</div>`;
    } else {
        const isBook = item.type === 'Book';
        resultDiv.innerHTML = `
        <div class="item-card">
            <span class="item-type ${isBook ? 'book' : 'magazine'}">${item.type}</span>
            <h3>${item.title}</h3>
            <div class="item-meta">
                ${isBook ? `<span><strong>Author:</strong> ${item.author}</span>
                <span><strong>ISBN:</strong> ${item.isbn}</span>
                <span><strong>Genre:</strong> ${item.genre}</span>` :
                `<span><strong>Issue #:</strong> ${item.issueNumber}</span>
                <span><strong>Publisher:</strong> ${item.publisher}</span>
                <span><strong>Category:</strong> ${item.category}</span>`}
                <span><strong>Year:</strong> ${item.yearPublished}</span>
                <span><strong>Fine (5 days):</strong> $${isBook ? '2.50' : '5.00'}</span>
            </div>
            <span class="item-status ${item.available ? 'available' : 'borrowed'}">
                ${item.available ? 'Available' : 'Borrowed'}
            </span>
        </div>`;
    }
});

// ===== Add Item Modal =====
const addItemModal = document.getElementById('addItemModal');
document.getElementById('addItemBtn').addEventListener('click', () => addItemModal.classList.add('active'));
document.getElementById('closeItemModal').addEventListener('click', () => addItemModal.classList.remove('active'));
addItemModal.addEventListener('click', e => { if (e.target === addItemModal) addItemModal.classList.remove('active'); });

document.getElementById('itemType').addEventListener('change', e => {
    document.getElementById('bookFields').style.display = e.target.value === 'Book' ? 'block' : 'none';
    document.getElementById('magazineFields').style.display = e.target.value === 'Magazine' ? 'block' : 'none';
});

document.getElementById('addItemForm').addEventListener('submit', async e => {
    e.preventDefault();
    const type = document.getElementById('itemType').value;
    let body = `type=${type}&title=${encodeURIComponent(document.getElementById('itemTitle').value)}&year=${document.getElementById('itemYear').value}`;
    if (type === 'Book') {
        body += `&author=${encodeURIComponent(document.getElementById('itemAuthor').value)}&isbn=${encodeURIComponent(document.getElementById('itemISBN').value)}&genre=${encodeURIComponent(document.getElementById('itemGenre').value)}`;
    } else {
        body += `&issueNumber=${document.getElementById('itemIssue').value}&publisher=${encodeURIComponent(document.getElementById('itemPublisher').value)}&category=${encodeURIComponent(document.getElementById('itemCategory').value)}`;
    }
    try {
        const data = await api('/items/add', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body
        });
        if (data.success) {
            addItemModal.classList.remove('active');
            document.getElementById('addItemForm').reset();
            loadItems();
        }
    } catch (err) { }
});

// ===== Add Member Modal =====
const addMemberModal = document.getElementById('addMemberModal');
document.getElementById('addMemberBtn').addEventListener('click', () => addMemberModal.classList.add('active'));
document.getElementById('closeMemberModal').addEventListener('click', () => addMemberModal.classList.remove('active'));
addMemberModal.addEventListener('click', e => { if (e.target === addMemberModal) addMemberModal.classList.remove('active'); });

document.getElementById('addMemberForm').addEventListener('submit', async e => {
    e.preventDefault();
    const body = `name=${encodeURIComponent(document.getElementById('memberName').value)}&email=${encodeURIComponent(document.getElementById('memberEmail').value)}&phone=${encodeURIComponent(document.getElementById('memberPhone').value)}&type=${document.getElementById('memberType').value}`;
    try {
        const data = await api('/members/add', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body
        });
        if (data.success) {
            addMemberModal.classList.remove('active');
            document.getElementById('addMemberForm').reset();
            loadMembers();
        }
    } catch (err) { }
});

// ===== Init: Load ALL pages on startup =====
async function initApp() {
    // Load all pages so every section is populated
    await Promise.allSettled([
        loadStats(),
        loadItems(),
        loadMembers(),
        loadTransactionData()
    ]);
}

initApp();
