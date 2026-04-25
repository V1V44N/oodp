/*
 * ============================================================
 *  Library Management System - Web Server
 *  A lightweight HTTP server using Winsock2 that serves the
 *  web frontend and exposes a JSON REST API.
 *
 *  Compile (Windows): g++ -std=c++17 -o server server.cpp -lws2_32
 *  Run: server.exe
 *  Open: http://localhost:8080
 * ============================================================
 */

#ifdef _WIN32
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET socket_t;
  #define CLOSE_SOCKET closesocket
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  typedef int socket_t;
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  #define CLOSE_SOCKET   close
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

using namespace std;

// ============================================================
//  CUSTOM EXCEPTION CLASSES (Exception Handling)
// ============================================================

class ItemNotFoundException : public runtime_error {
public:
    explicit ItemNotFoundException(int id)
        : runtime_error("Item with ID " + to_string(id) + " not found.") {}
    explicit ItemNotFoundException(const string& msg)
        : runtime_error(msg) {}
};

class MemberLimitExceededException : public runtime_error {
public:
    explicit MemberLimitExceededException(const string& name, int limit)
        : runtime_error("Member '" + name + "' reached borrow limit of "
                             + to_string(limit) + " items.") {}
};

class ItemNotAvailableException : public runtime_error {
public:
    explicit ItemNotAvailableException(int id)
        : runtime_error("Item ID " + to_string(id) + " is not available.") {}
};

// ============================================================
//  ABSTRACT BASE CLASS: LibraryItem
//  Demonstrates: Abstraction, Encapsulation
// ============================================================

class LibraryItem {
private:
    int id;                 // Unique identifier
    string title;      // Title of the item
    bool available;         // Availability status
    int yearPublished;      // Year of publication

public:
    // Constructor
    LibraryItem(int id, const string& title, int year)
        : id(id), title(title), available(true), yearPublished(year) {}

    // Virtual destructor for proper polymorphic cleanup
    virtual ~LibraryItem() = default;

    // ---- Pure Virtual Methods (Abstraction) ----
    virtual string getDetails() const = 0;
    virtual string getType() const = 0;
    virtual double calculateFine(int daysOverdue) const = 0;

    // ---- Getters (Encapsulation) ----
    int getId() const { return id; }
    string getTitle() const { return title; }
    bool isAvailable() const { return available; }
    int getYearPublished() const { return yearPublished; }

    // ---- Setters with validation (Encapsulation) ----
    void setTitle(const string& t) {
        if (t.empty()) throw invalid_argument("Title cannot be empty.");
        title = t;
    }
    void setAvailable(bool s) { available = s; }
    void setYearPublished(int y) {
        if (y < 0 || y > 2030)
            throw invalid_argument("Invalid publication year.");
        yearPublished = y;
    }

    // ---- Operator Overloading: == compares by ID ----
    bool operator==(const LibraryItem& other) const {
        return id == other.id;
    }

    // ---- Operator Overloading: << for printing ----
    friend ostream& operator<<(ostream& os, const LibraryItem& item) {
        os << item.getDetails();
        return os;
    }

    // JSON representation (virtual so subclasses extend it)
    virtual string toJSON() const {
        ostringstream o;
        o << "{\"id\":" << id
          << ",\"title\":\"" << esc(title) << "\""
          << ",\"type\":\"" << getType() << "\""
          << ",\"available\":" << (available ? "true" : "false")
          << ",\"yearPublished\":" << yearPublished;
        return o.str(); // Note: subclasses close the brace
    }

protected:
    // Utility: escape a string for JSON output
    static string esc(const string& s) {
        string r;
        for (char c : s) {
            if (c == '"') r += "\\\"";
            else if (c == '\\') r += "\\\\";
            else r += c;
        }
        return r;
    }
};

// ============================================================
//  BOOK CLASS
//  Demonstrates: Inheritance, Polymorphism
// ============================================================

class Book : public LibraryItem {
private:
    string author;
    string isbn;
    string genre;

public:
    Book(int id, const string& title, int year,
         const string& author, const string& isbn,
         const string& genre)
        : LibraryItem(id, title, year),
          author(author), isbn(isbn), genre(genre) {}

    // Getters
    string getAuthor() const { return author; }
    string getISBN() const { return isbn; }
    string getGenre() const { return genre; }

    // Setters with validation
    void setAuthor(const string& a) {
        if (a.empty()) throw invalid_argument("Author cannot be empty.");
        author = a;
    }

    // ---- Polymorphism: override virtual methods ----
    string getDetails() const override {
        ostringstream o;
        o << "[Book] ID:" << getId()
          << " | " << getTitle()
          << " | by " << author
          << " | ISBN:" << isbn
          << " | " << genre
          << " | " << getYearPublished()
          << " | " << (isAvailable() ? "Available" : "Borrowed");
        return o.str();
    }

    string getType() const override { return "Book"; }

    // Books charge $0.50 per overdue day
    double calculateFine(int daysOverdue) const override {
        return (daysOverdue > 0) ? daysOverdue * 0.50 : 0.0;
    }

    string toJSON() const override {
        ostringstream o;
        o << LibraryItem::toJSON()
          << ",\"author\":\"" << esc(author) << "\""
          << ",\"isbn\":\"" << esc(isbn) << "\""
          << ",\"genre\":\"" << esc(genre) << "\""
          << "}";
        return o.str();
    }
};

// ============================================================
//  MAGAZINE CLASS
//  Demonstrates: Inheritance, Polymorphism
// ============================================================

class Magazine : public LibraryItem {
private:
    int issueNumber;
    string publisher;
    string category;

public:
    Magazine(int id, const string& title, int year,
             int issue, const string& publisher,
             const string& category)
        : LibraryItem(id, title, year),
          issueNumber(issue), publisher(publisher), category(category) {}

    // Getters
    int getIssueNumber() const { return issueNumber; }
    string getPublisher() const { return publisher; }
    string getCategory() const { return category; }

    // Setters
    void setIssueNumber(int n) {
        if (n < 0) throw invalid_argument("Issue number cannot be negative.");
        issueNumber = n;
    }

    // ---- Polymorphism: override virtual methods ----
    string getDetails() const override {
        ostringstream o;
        o << "[Magazine] ID:" << getId()
          << " | " << getTitle()
          << " | Issue #" << issueNumber
          << " | " << publisher
          << " | " << category
          << " | " << getYearPublished()
          << " | " << (isAvailable() ? "Available" : "Borrowed");
        return o.str();
    }

    string getType() const override { return "Magazine"; }

    // Magazines charge $1.00 per overdue day (higher rate)
    double calculateFine(int daysOverdue) const override {
        return (daysOverdue > 0) ? daysOverdue * 1.00 : 0.0;
    }

    string toJSON() const override {
        ostringstream o;
        o << LibraryItem::toJSON()
          << ",\"issueNumber\":" << issueNumber
          << ",\"publisher\":\"" << esc(publisher) << "\""
          << ",\"category\":\"" << esc(category) << "\""
          << "}";
        return o.str();
    }
};

// ============================================================
//  TEMPLATE CLASS: Catalogue<T>
//  Demonstrates: Templates, STL (vector)
// ============================================================

template <typename T>
class Catalogue {
private:
    vector<T> items;   // STL vector to store items

public:
    // Add an item to the catalogue
    void addItem(T item) {
        items.push_back(item);
    }

    // Remove an item by ID
    void removeItem(int id) {
        auto it = remove_if(items.begin(), items.end(),
            [id](T item) { return item->getId() == id; });
        if (it == items.end())
            throw ItemNotFoundException(id);
        items.erase(it, items.end());
    }

    // Template method: find an item by its ID
    T findById(int id) const {
        for (const auto& item : items) {
            if (item->getId() == id)
                return item;
        }
        throw ItemNotFoundException(id);
    }

    // Print all items using the overloaded << operator
    void printAll(ostream& os = cout) const {
        os << "\n========= CATALOGUE =========\n";
        for (const auto& item : items)
            os << *item << "\n";
        os << "=============================\n";
    }

    // Accessors
    const vector<T>& getItems() const { return items; }
    size_t size() const { return items.size(); }

    size_t countAvailable() const {
        return count_if(items.begin(), items.end(),
            [](T item) { return item->isAvailable(); });
    }
};

// ============================================================
//  MEMBER CLASS
//  Demonstrates: Encapsulation, STL (vector)
// ============================================================

class Member {
private:
    int memberId;
    string name;
    string email;
    string phone;
    vector<int> borrowedItemIds;   // Borrow history
    int borrowLimit;

public:
    Member(int id, const string& name, const string& email,
           const string& phone, int limit = 3)
        : memberId(id), name(name), email(email),
          phone(phone), borrowLimit(limit) {}

    virtual ~Member() = default;

    // ---- Getters ----
    int getMemberId() const { return memberId; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getPhone() const { return phone; }
    int getBorrowLimit() const { return borrowLimit; }
    const vector<int>& getBorrowedItems() const { return borrowedItemIds; }
    int getBorrowedCount() const { return (int)borrowedItemIds.size(); }

    // ---- Setters with validation ----
    void setName(const string& n) {
        if (n.empty()) throw invalid_argument("Name cannot be empty.");
        name = n;
    }
    void setEmail(const string& e) {
        if (e.empty() || e.find('@') == string::npos)
            throw invalid_argument("Invalid email address.");
        email = e;
    }
    void setPhone(const string& p) { phone = p; }

    // ---- Borrow / Return ----
    virtual void borrowItem(int itemId) {
        if ((int)borrowedItemIds.size() >= borrowLimit)
            throw MemberLimitExceededException(name, borrowLimit);
        borrowedItemIds.push_back(itemId);
    }

    void returnItem(int itemId) {
        auto it = find(borrowedItemIds.begin(), borrowedItemIds.end(), itemId);
        if (it != borrowedItemIds.end())
            borrowedItemIds.erase(it);
    }

    bool hasBorrowed(int itemId) const {
        return find(borrowedItemIds.begin(), borrowedItemIds.end(), itemId)
               != borrowedItemIds.end();
    }

    // Virtual: member type string
    virtual string getMemberType() const { return "Standard"; }
    // Virtual: fine multiplier (Premium members get discount)
    virtual double getFineMultiplier() const { return 1.0; }

    // JSON representation
    virtual string toJSON() const {
        ostringstream o;
        o << "{\"memberId\":" << memberId
          << ",\"name\":\"" << name << "\""
          << ",\"email\":\"" << email << "\""
          << ",\"phone\":\"" << phone << "\""
          << ",\"type\":\"" << getMemberType() << "\""
          << ",\"borrowLimit\":" << borrowLimit
          << ",\"borrowedCount\":" << borrowedItemIds.size()
          << ",\"borrowedItems\":[";
        for (size_t i = 0; i < borrowedItemIds.size(); i++) {
            if (i > 0) o << ",";
            o << borrowedItemIds[i];
        }
        o << "]}";
        return o.str();
    }
};

// ============================================================
//  PREMIUM MEMBER CLASS
//  Demonstrates: Multi-level Inheritance
// ============================================================

class PremiumMember : public Member {
private:
    double discountRate;    // Discount on fines (e.g. 0.5 = 50% off)

public:
    PremiumMember(int id, const string& name,
                  const string& email, const string& phone,
                  double discount = 0.5)
        : Member(id, name, email, phone, 6),   // Premium: 6-item limit
          discountRate(discount) {}

    double getDiscountRate() const { return discountRate; }

    string getMemberType() const override { return "Premium"; }

    // Premium members pay reduced fines
    double getFineMultiplier() const override {
        return 1.0 - discountRate;
    }
};

// ============================================================
//  LIBRARY CLASS (Main System)
//  Demonstrates: Operator Overloading (+= and <<), STL (map)
// ============================================================

class Library {
private:
    string libraryName;
    Catalogue<LibraryItem*> catalogue;
    vector<Member*> members;
    map<int, int> borrowMap;   // itemId -> memberId

public:
    explicit Library(const string& name) : libraryName(name) {}

    // Destructor: clean up heap-allocated objects
    ~Library() {
        for (auto item : catalogue.getItems()) delete item;
        for (auto m : members) delete m;
    }

    // ---- Operator Overloading: += adds an item ----
    Library& operator+=(LibraryItem* item) {
        catalogue.addItem(item);
        return *this;
    }

    // ---- Member management ----
    void addMember(Member* m) { members.push_back(m); }

    Member* findMember(int id) const {
        for (auto& m : members)
            if (m->getMemberId() == id) return m;
        throw runtime_error("Member ID " + to_string(id) + " not found.");
    }

    // ---- Borrow an item ----
    void borrowItem(int memberId, int itemId) {
        LibraryItem* item = catalogue.findById(itemId);
        if (!item->isAvailable())
            throw ItemNotAvailableException(itemId);
        Member* member = findMember(memberId);
        member->borrowItem(itemId);       // may throw MemberLimitExceededException
        item->setAvailable(false);
        borrowMap[itemId] = memberId;
    }

    // ---- Return an item (calculates fine) ----
    double returnItem(int memberId, int itemId, int daysOverdue = 0) {
        LibraryItem* item = catalogue.findById(itemId);
        Member* member = findMember(memberId);
        member->returnItem(itemId);
        item->setAvailable(true);
        borrowMap.erase(itemId);
        // Polymorphic fine calculation
        return item->calculateFine(daysOverdue) * member->getFineMultiplier();
    }

    // ---- Accessors ----
    const string& getName() const { return libraryName; }
    const Catalogue<LibraryItem*>& getCatalogue() const { return catalogue; }
    Catalogue<LibraryItem*>& getCatalogue() { return catalogue; }
    const vector<Member*>& getMembers() const { return members; }
    const map<int, int>& getBorrowMap() const { return borrowMap; }

    // ---- Operator Overloading: << prints library info ----
    friend ostream& operator<<(ostream& os, const Library& lib) {
        os << "\n======== " << lib.libraryName << " ========\n";
        lib.catalogue.printAll(os);
        os << "Members : " << lib.members.size() << "\n";
        os << "Items   : " << lib.catalogue.size() << "\n";
        os << "Available: " << lib.catalogue.countAvailable() << "\n";
        os << "===================================\n";
        return os;
    }

    // ---- JSON helpers for the web API ----
    string getStatsJSON() const {
        int books = 0, mags = 0, borrowed = 0, premium = 0;
        for (auto i : catalogue.getItems()) {
            if (i->getType() == "Book") books++; else mags++;
            if (!i->isAvailable()) borrowed++;
        }
        for (auto m : members)
            if (m->getMemberType() == "Premium") premium++;
        ostringstream o;
        o << "{\"totalItems\":" << catalogue.size()
          << ",\"totalBooks\":" << books
          << ",\"totalMagazines\":" << mags
          << ",\"available\":" << catalogue.countAvailable()
          << ",\"borrowed\":" << borrowed
          << ",\"totalMembers\":" << members.size()
          << ",\"premiumMembers\":" << premium << "}";
        return o.str();
    }

    string getItemsJSON() const {
        ostringstream o;
        o << "[";
        auto& items = catalogue.getItems();
        for (size_t i = 0; i < items.size(); i++) {
            if (i) o << ",";
            o << items[i]->toJSON();
        }
        o << "]";
        return o.str();
    }

    string getMembersJSON() const {
        ostringstream o;
        o << "[";
        for (size_t i = 0; i < members.size(); i++) {
            if (i) o << ",";
            o << members[i]->toJSON();
        }
        o << "]";
        return o.str();
    }

    string searchItemJSON(int id) const {
        try {
            return catalogue.findById(id)->toJSON();
        } catch (...) {
            return "{\"error\":\"Item not found\"}";
        }
    }

    // Get next available IDs
    int getNextItemId() const {
        int maxId = 0;
        for (auto i : catalogue.getItems())
            if (i->getId() > maxId) maxId = i->getId();
        return maxId + 1;
    }

    int getNextMemberId() const {
        int maxId = 100;
        for (auto m : members)
            if (m->getMemberId() > maxId) maxId = m->getMemberId();
        return maxId + 1;
    }
};


// ---- Global Library instance with sample data ----
Library* gLibrary = nullptr;

void initSampleData() {
    gLibrary = new Library("University Central Library");

    // Books (18 total)
    *gLibrary += new Book(1, "The C++ Programming Language", 2013,
                          "Bjarne Stroustrup", "978-0321563842", "Programming");
    *gLibrary += new Book(2, "Clean Code", 2008,
                          "Robert C. Martin", "978-0132350884", "Software Engineering");
    *gLibrary += new Book(3, "Design Patterns", 1994,
                          "Gang of Four", "978-0201633610", "Computer Science");
    *gLibrary += new Book(4, "Introduction to Algorithms", 2009,
                          "Thomas H. Cormen", "978-0262033848", "Algorithms");
    *gLibrary += new Book(5, "The Pragmatic Programmer", 2019,
                          "David Thomas", "978-0135957059", "Software Development");
    *gLibrary += new Book(6, "Effective Modern C++", 2014,
                          "Scott Meyers", "978-1491903995", "Programming");
    *gLibrary += new Book(7, "Data Structures and Algorithm Analysis", 2011,
                          "Mark Allen Weiss", "978-0132847377", "Data Structures");
    *gLibrary += new Book(8, "Artificial Intelligence: A Modern Approach", 2020,
                          "Stuart Russell", "978-0134610993", "Artificial Intelligence");
    *gLibrary += new Book(9, "Operating System Concepts", 2018,
                          "Abraham Silberschatz", "978-1119320913", "Operating Systems");
    *gLibrary += new Book(10, "Computer Networking: A Top-Down Approach", 2016,
                           "James Kurose", "978-0133594140", "Networking");
    *gLibrary += new Book(11, "Database System Concepts", 2019,
                           "Abraham Silberschatz", "978-0078022159", "Databases");
    *gLibrary += new Book(12, "Structure and Interpretation of Computer Programs", 1996,
                           "Harold Abelson", "978-0262510875", "Computer Science");
    *gLibrary += new Book(13, "The Art of Computer Programming", 2011,
                           "Donald Knuth", "978-0201896831", "Algorithms");
    *gLibrary += new Book(14, "Compilers: Principles, Techniques, and Tools", 2006,
                           "Alfred Aho", "978-0321486813", "Compilers");
    *gLibrary += new Book(15, "Discrete Mathematics and Its Applications", 2018,
                           "Kenneth Rosen", "978-1259676512", "Mathematics");
    *gLibrary += new Book(16, "Computer Organization and Design", 2013,
                           "David Patterson", "978-0124077263", "Computer Architecture");
    *gLibrary += new Book(17, "Software Engineering", 2015,
                           "Ian Sommerville", "978-0133943030", "Software Engineering");
    *gLibrary += new Book(18, "Machine Learning", 2022,
                           "Tom Mitchell", "978-1259096952", "Machine Learning");

    // Magazines
    *gLibrary += new Magazine(19, "IEEE Spectrum", 2024, 42,
                              "IEEE", "Technology");
    *gLibrary += new Magazine(20, "Nature", 2024, 615,
                              "Springer Nature", "Science");
    *gLibrary += new Magazine(21, "ACM Computing Surveys", 2024, 88,
                              "ACM", "Computer Science");
    *gLibrary += new Magazine(22, "Scientific American", 2024, 330,
                               "Springer Nature", "Science");

    // Members
    gLibrary->addMember(new Member(101, "Alice Johnson", "alice@uni.edu", "555-0101"));
    gLibrary->addMember(new Member(102, "Charlie Brown", "charlie@uni.edu", "555-0103"));
    gLibrary->addMember(new PremiumMember(103, "Bob Smith", "bob@uni.edu", "555-0102"));
    gLibrary->addMember(new Member(104, "Diana Ross", "diana@uni.edu", "555-0104"));
    gLibrary->addMember(new PremiumMember(105, "Eve Williams", "eve@uni.edu", "555-0105"));
}

// ---- Utility functions ----

string getMimeType(const string& path) {
    if (path.find(".html") != string::npos) return "text/html";
    if (path.find(".css")  != string::npos) return "text/css";
    if (path.find(".js")   != string::npos) return "application/javascript";
    if (path.find(".json") != string::npos) return "application/json";
    if (path.find(".png")  != string::npos) return "image/png";
    if (path.find(".ico")  != string::npos) return "image/x-icon";
    return "text/plain";
}

string readFile(const string& path) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) return "";
    ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

string buildResponse(int status, const string& contentType,
                           const string& body) {
    string statusText = (status == 200) ? "OK" :
                             (status == 404) ? "Not Found" :
                             (status == 400) ? "Bad Request" : "Internal Server Error";
    ostringstream r;
    r << "HTTP/1.1 " << status << " " << statusText << "\r\n"
      << "Content-Type: " << contentType << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Access-Control-Allow-Origin: *\r\n"
      << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
      << "Access-Control-Allow-Headers: Content-Type\r\n"
      << "Connection: close\r\n"
      << "\r\n"
      << body;
    return r.str();
}

// Parse simple key=value from a query string or POST body
map<string, string> parseParams(const string& data) {
    map<string, string> params;
    istringstream stream(data);
    string pair;
    while (getline(stream, pair, '&')) {
        auto eq = pair.find('=');
        if (eq != string::npos) {
            string key = pair.substr(0, eq);
            string val = pair.substr(eq + 1);
            // Simple URL decode for + and %20
            replace(val.begin(), val.end(), '+', ' ');
            params[key] = val;
        }
    }
    return params;
}

// URL-decode a string (basic)
string urlDecode(const string& s) {
    string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int val = 0;
            istringstream iss(s.substr(i + 1, 2));
            iss >> hex >> val;
            result += (char)val;
            i += 2;
        } else if (s[i] == '+') {
            result += ' ';
        } else {
            result += s[i];
        }
    }
    return result;
}

// ---- Handle API requests ----
string handleAPI(const string& method, const string& path,
                      const string& body) {
    // GET /api/stats
    if (method == "GET" && path == "/api/stats") {
        return buildResponse(200, "application/json", gLibrary->getStatsJSON());
    }

    // GET /api/items
    if (method == "GET" && path == "/api/items") {
        return buildResponse(200, "application/json", gLibrary->getItemsJSON());
    }

    // GET /api/members
    if (method == "GET" && path == "/api/members") {
        return buildResponse(200, "application/json", gLibrary->getMembersJSON());
    }

    // GET /api/items/search?id=X
    if (method == "GET" && path.find("/api/items/search") == 0) {
        auto qPos = path.find("?");
        if (qPos != string::npos) {
            auto params = parseParams(path.substr(qPos + 1));
            if (params.count("id")) {
                int id = stoi(params["id"]);
                return buildResponse(200, "application/json",
                                     gLibrary->searchItemJSON(id));
            }
        }
        return buildResponse(400, "application/json", "{\"error\":\"Missing id parameter\"}");
    }

    // POST /api/borrow  (body: memberId=X&itemId=Y)
    if (method == "POST" && path == "/api/borrow") {
        auto params = parseParams(body);
        try {
            int memberId = stoi(params["memberId"]);
            int itemId = stoi(params["itemId"]);
            gLibrary->borrowItem(memberId, itemId);
            return buildResponse(200, "application/json",
                "{\"success\":true,\"message\":\"Item borrowed successfully.\"}");
        } catch (const exception& e) {
            return buildResponse(400, "application/json",
                string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    // POST /api/return  (body: memberId=X&itemId=Y&daysOverdue=Z)
    if (method == "POST" && path == "/api/return") {
        auto params = parseParams(body);
        try {
            int memberId = stoi(params["memberId"]);
            int itemId = stoi(params["itemId"]);
            int days = params.count("daysOverdue") ? stoi(params["daysOverdue"]) : 0;
            double fine = gLibrary->returnItem(memberId, itemId, days);
            ostringstream o;
            o << "{\"success\":true,\"fine\":" << fixed << setprecision(2) << fine
              << ",\"message\":\"Item returned. Fine: $" << fixed << setprecision(2) << fine << "\"}";
            return buildResponse(200, "application/json", o.str());
        } catch (const exception& e) {
            return buildResponse(400, "application/json",
                string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    // POST /api/items/add  (body: type=Book&title=...&year=...&...)
    if (method == "POST" && path == "/api/items/add") {
        auto params = parseParams(body);
        try {
            int id = gLibrary->getNextItemId();
            string type = params["type"];
            string title = urlDecode(params["title"]);
            int year = stoi(params["year"]);

            if (type == "Book") {
                string author = urlDecode(params["author"]);
                string isbn = urlDecode(params["isbn"]);
                string genre = urlDecode(params["genre"]);
                *gLibrary += new Book(id, title, year, author, isbn, genre);
            } else {
                int issue = stoi(params["issueNumber"]);
                string publisher = urlDecode(params["publisher"]);
                string category = urlDecode(params["category"]);
                *gLibrary += new Magazine(id, title, year, issue, publisher, category);
            }
            return buildResponse(200, "application/json",
                "{\"success\":true,\"message\":\"Item added with ID " + to_string(id) + "\"}");
        } catch (const exception& e) {
            return buildResponse(400, "application/json",
                string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    // POST /api/members/add
    if (method == "POST" && path == "/api/members/add") {
        auto params = parseParams(body);
        try {
            int id = gLibrary->getNextMemberId();
            string name = urlDecode(params["name"]);
            string email = urlDecode(params["email"]);
            string phone = urlDecode(params["phone"]);
            string type = params.count("type") ? params["type"] : "Standard";

            if (type == "Premium")
                gLibrary->addMember(new PremiumMember(id, name, email, phone));
            else
                gLibrary->addMember(new Member(id, name, email, phone));

            return buildResponse(200, "application/json",
                "{\"success\":true,\"message\":\"Member added with ID " + to_string(id) + "\"}");
        } catch (const exception& e) {
            return buildResponse(400, "application/json",
                string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    return buildResponse(404, "application/json", "{\"error\":\"API endpoint not found\"}");
}

// ---- Handle a single client connection ----
void handleClient(socket_t client) {
    char buffer[8192] = {0};
    int received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) { CLOSE_SOCKET(client); return; }

    string request(buffer, received);

    // Parse method and path
    string method, path;
    istringstream reqStream(request);
    reqStream >> method >> path;

    // Handle OPTIONS (CORS preflight)
    if (method == "OPTIONS") {
        string resp = buildResponse(200, "text/plain", "");
        send(client, resp.c_str(), (int)resp.size(), 0);
        CLOSE_SOCKET(client);
        return;
    }

    // Extract body for POST requests
    string body;
    auto bodyPos = request.find("\r\n\r\n");
    if (bodyPos != string::npos)
        body = request.substr(bodyPos + 4);

    string response;

    // Route: API endpoints
    if (path.find("/api/") == 0) {
        response = handleAPI(method, path, body);
    }
    // Route: serve static files
    else {
        string filePath = (path == "/") ? "index.html" : path.substr(1);

        // Security: prevent directory traversal
        if (filePath.find("..") != string::npos) {
            response = buildResponse(400, "text/plain", "Bad Request");
        } else {
            string content = readFile(filePath);
            if (!content.empty()) {
                response = buildResponse(200, getMimeType(filePath), content);
            } else {
                response = buildResponse(404, "text/plain", "File not found: " + filePath);
            }
        }
    }

    send(client, response.c_str(), (int)response.size(), 0);
    CLOSE_SOCKET(client);
}

// ---- Main: start HTTP server ----
int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!\n";
        return 1;
    }
    #endif

    initSampleData();
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════╗\n";
    cout << "  ║   Library Management System - Server     ║\n";
    cout << "  ║   http://localhost:8080                   ║\n";
    cout << "  ╚══════════════════════════════════════════╝\n";
    cout << "\n  [✓] Sample data loaded: "
              << gLibrary->getCatalogue().size() << " items, "
              << gLibrary->getMembers().size() << " members\n";
    cout << "  [✓] Server starting on port 8080...\n\n";

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket.\n";
        return 1;
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "Bind failed. Port 8080 may be in use.\n";
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "Listen failed.\n";
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    cout << "  [✓] Listening on http://localhost:8080\n";
    cout << "  [i] Press Ctrl+C to stop.\n\n";

    while (true) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);
        socket_t client = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (client == INVALID_SOCKET) continue;

        // Handle client synchronously (sufficient for demo)
        handleClient(client);
    }

    delete gLibrary;
    CLOSE_SOCKET(serverSocket);
    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}
