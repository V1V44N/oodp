<<<<<<< HEAD
/*
 * ============================================================
 *  Library Management System - Console Demonstration
 *  Demonstrates all OOP principles and advanced C++ features
 *  Compile: g++ -std=c++17 -o main main.cpp
 * ============================================================
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include "library_system.h"

int main() {
    std::cout << "=============================================\n";
    std::cout << "   LIBRARY MANAGEMENT SYSTEM - CONSOLE DEMO  \n";
    std::cout << "=============================================\n\n";
=======
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
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


int main() {
    cout << "=============================================\n";
    cout << "   LIBRARY MANAGEMENT SYSTEM - CONSOLE DEMO  \n";
    cout << "=============================================\n\n";
>>>>>>> dev

    // ----------------------------------------------------------
    // 1. Create the Library (Main System Class)
    // ----------------------------------------------------------
    Library lib("University Central Library");

    // ----------------------------------------------------------
    // 2. Create Book and Magazine objects, add via overloaded +=
    //    Demonstrates: Inheritance, Operator Overloading (+=)
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << "[+] Adding items using overloaded += operator...\n\n";
=======
    cout << "[+] Adding items using overloaded += operator...\n\n";
>>>>>>> dev

    lib += new Book(1, "The C++ Programming Language", 2013,
                    "Bjarne Stroustrup", "978-0321563842", "Programming");
    lib += new Book(2, "Clean Code", 2008,
                    "Robert C. Martin", "978-0132350884", "Software Engineering");
    lib += new Book(3, "Design Patterns", 1994,
                    "Gang of Four", "978-0201633610", "Computer Science");
    lib += new Book(4, "Introduction to Algorithms", 2009,
                    "Thomas H. Cormen", "978-0262033848", "Algorithms");
    lib += new Book(5, "The Pragmatic Programmer", 2019,
                    "David Thomas", "978-0135957059", "Software Development");

    lib += new Magazine(6, "IEEE Spectrum", 2024, 42,
                        "IEEE", "Technology");
    lib += new Magazine(7, "Nature", 2024, 615,
                        "Springer Nature", "Science");
    lib += new Magazine(8, "ACM Computing Surveys", 2024, 88,
                        "ACM", "Computer Science");

    // ----------------------------------------------------------
    // 3. Print all items using overloaded << operator
    //    Demonstrates: Operator Overloading (<<)
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << lib;
=======
    cout << lib;
>>>>>>> dev

    // ----------------------------------------------------------
    // 4. Demonstrate POLYMORPHISM
    //    Loop through a vector<LibraryItem*> and call virtual methods
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << "\n--- Polymorphism Demo: Calling virtual methods ---\n";
    const auto& items = lib.getCatalogue().getItems();
    for (const auto& item : items) {
        std::cout << "  Type: " << std::left << std::setw(10) << item->getType()
                  << " | Fine (5 days overdue): $"
                  << std::fixed << std::setprecision(2)
=======
    cout << "\n--- Polymorphism Demo: Calling virtual methods ---\n";
    const auto& items = lib.getCatalogue().getItems();
    for (const auto& item : items) {
        cout << "  Type: " << left << setw(10) << item->getType()
                  << " | Fine (5 days overdue): $"
                  << fixed << setprecision(2)
>>>>>>> dev
                  << item->calculateFine(5) << "\n";
    }

    // ----------------------------------------------------------
    // 5. Create Members (Standard and Premium)
    //    Demonstrates: Inheritance (PremiumMember : Member)
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << "\n[+] Adding members...\n";
=======
    cout << "\n[+] Adding members...\n";
>>>>>>> dev
    lib.addMember(new Member(101, "Alice Johnson", "alice@uni.edu", "555-0101"));
    lib.addMember(new Member(102, "Charlie Brown", "charlie@uni.edu", "555-0103"));
    lib.addMember(new PremiumMember(103, "Bob Smith", "bob@uni.edu", "555-0102", 0.5));

    for (const auto& m : lib.getMembers()) {
<<<<<<< HEAD
        std::cout << "  " << m->getMemberType() << " Member: "
=======
        cout << "  " << m->getMemberType() << " Member: "
>>>>>>> dev
                  << m->getName() << " (Limit: " << m->getBorrowLimit() << ")\n";
    }

    // ----------------------------------------------------------
    // 6. Simulate Borrowing and Returning
    //    Demonstrates: Exception Handling, Polymorphic fine calc
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << "\n--- Borrowing & Returning Demo ---\n";

    // Alice borrows "The C++ Programming Language" (Book, ID 1)
    lib.borrowItem(101, 1);
    std::cout << "  Alice borrowed item 1 (Book).\n";

    // Bob (Premium) borrows "Nature" (Magazine, ID 7)
    lib.borrowItem(103, 7);
    std::cout << "  Bob (Premium) borrowed item 7 (Magazine).\n";

    // Return with fines
    double fineAlice = lib.returnItem(101, 1, 5);
    std::cout << "  Alice returned item 1, 5 days late. Fine: $"
              << std::fixed << std::setprecision(2) << fineAlice << "\n";

    double fineBob = lib.returnItem(103, 7, 5);
    std::cout << "  Bob returned item 7, 5 days late. Fine: $"
              << std::fixed << std::setprecision(2) << fineBob
=======
    cout << "\n--- Borrowing & Returning Demo ---\n";

    // Alice borrows "The C++ Programming Language" (Book, ID 1)
    lib.borrowItem(101, 1);
    cout << "  Alice borrowed item 1 (Book).\n";

    // Bob (Premium) borrows "Nature" (Magazine, ID 7)
    lib.borrowItem(103, 7);
    cout << "  Bob (Premium) borrowed item 7 (Magazine).\n";

    // Return with fines
    double fineAlice = lib.returnItem(101, 1, 5);
    cout << "  Alice returned item 1, 5 days late. Fine: $"
              << fixed << setprecision(2) << fineAlice << "\n";

    double fineBob = lib.returnItem(103, 7, 5);
    cout << "  Bob returned item 7, 5 days late. Fine: $"
              << fixed << setprecision(2) << fineBob
>>>>>>> dev
              << " (Premium 50% discount!)\n";

    // ----------------------------------------------------------
    // 7. Demonstrate Template: search by ID using Catalogue<T>
    //    Demonstrates: Templates
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << "\n--- Template Search Demo ---\n";
    try {
        LibraryItem* found = lib.getCatalogue().findById(3);
        std::cout << "  Found: " << *found << "\n";
    } catch (const ItemNotFoundException& e) {
        std::cout << "  Error: " << e.what() << "\n";
=======
    cout << "\n--- Template Search Demo ---\n";
    try {
        LibraryItem* found = lib.getCatalogue().findById(3);
        cout << "  Found: " << *found << "\n";
    } catch (const ItemNotFoundException& e) {
        cout << "  Error: " << e.what() << "\n";
>>>>>>> dev
    }

    // ----------------------------------------------------------
    // 8. Trigger and Catch Custom Exceptions
    //    Demonstrates: Exception Handling
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << "\n--- Exception Handling Demo ---\n";
=======
    cout << "\n--- Exception Handling Demo ---\n";
>>>>>>> dev

    // a) ItemNotFoundException: search for non-existent ID
    try {
        lib.getCatalogue().findById(999);
    } catch (const ItemNotFoundException& e) {
<<<<<<< HEAD
        std::cout << "  Caught ItemNotFoundException: " << e.what() << "\n";
=======
        cout << "  Caught ItemNotFoundException: " << e.what() << "\n";
>>>>>>> dev
    }

    // b) MemberLimitExceededException: exceed borrow limit
    try {
        // Alice (Standard, limit=3) borrows 4 items
        lib.borrowItem(101, 2);
        lib.borrowItem(101, 3);
        lib.borrowItem(101, 4);
        lib.borrowItem(101, 5);  // This should throw
    } catch (const MemberLimitExceededException& e) {
<<<<<<< HEAD
        std::cout << "  Caught MemberLimitExceededException: " << e.what() << "\n";
    }

    // c) Operator== comparison
    std::cout << "\n--- Operator== Demo ---\n";
    const auto& allItems = lib.getCatalogue().getItems();
    if (*allItems[0] == *allItems[0]) {
        std::cout << "  Item 0 == Item 0: true (same ID)\n";
    }
    if (!(*allItems[0] == *allItems[1])) {
        std::cout << "  Item 0 == Item 1: false (different IDs)\n";
=======
        cout << "  Caught MemberLimitExceededException: " << e.what() << "\n";
    }

    // c) Operator== comparison
    cout << "\n--- Operator== Demo ---\n";
    const auto& allItems = lib.getCatalogue().getItems();
    if (*allItems[0] == *allItems[0]) {
        cout << "  Item 0 == Item 0: true (same ID)\n";
    }
    if (!(*allItems[0] == *allItems[1])) {
        cout << "  Item 0 == Item 1: false (different IDs)\n";
>>>>>>> dev
    }

    // ----------------------------------------------------------
    // 9. Final state
    // ----------------------------------------------------------
<<<<<<< HEAD
    std::cout << "\n--- Final Library State ---\n";
    std::cout << lib;

    std::cout << "\n[✓] All OOP principles and C++ features demonstrated!\n";
=======
    cout << "\n--- Final Library State ---\n";
    cout << lib;

    cout << "\n[✓] All OOP principles and C++ features demonstrated!\n";
>>>>>>> dev
    return 0;
}
