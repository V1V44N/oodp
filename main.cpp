/*
 * ============================================================
 *  Library Management System - Console Demonstration
 *  Demonstrates all OOP principles and advanced C++ features
 *  Compile: g++ -std=c++17 -o main main.cpp
 * ============================================================
 */

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

    virtual string toJSON() const { return "{}"; } // Simplified for console
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

    string getAuthor() const { return author; }

    string getDetails() const override {
        ostringstream o;
        o << "[Book] ID:" << getId() << " | " << getTitle() << " | by " << author;
        return o.str();
    }

    string getType() const override { return "Book"; }

    double calculateFine(int daysOverdue) const override {
        return (daysOverdue > 0) ? daysOverdue * 0.50 : 0.0;
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

    string getDetails() const override {
        ostringstream o;
        o << "[Magazine] ID:" << getId() << " | " << getTitle() << " | Issue #" << issueNumber;
        return o.str();
    }

    string getType() const override { return "Magazine"; }

    double calculateFine(int daysOverdue) const override {
        return (daysOverdue > 0) ? daysOverdue * 1.00 : 0.0;
    }
};

// ============================================================
//  TEMPLATE CLASS: Catalogue<T>
//  Demonstrates: Templates, STL (vector)
// ============================================================

template <typename T>
class Catalogue {
private:
    vector<T> items;

public:
    void addItem(T item) { items.push_back(item); }

    T findById(int id) const {
        for (const auto& item : items) {
            if (item->getId() == id) return item;
        }
        throw ItemNotFoundException(id);
    }

    void printAll() const {
        cout << "\n========= CATALOGUE =========\n";
        for (const auto& item : items) cout << *item << "\n";
        cout << "=============================\n";
    }

    const vector<T>& getItems() const { return items; }
    size_t size() const { return items.size(); }
    size_t countAvailable() const {
        return count_if(items.begin(), items.end(), [](T item) { return item->isAvailable(); });
    }
};

// ============================================================
//  MEMBER CLASS
// ============================================================

class Member {
private:
    int memberId;
    string name;
    vector<int> borrowedItemIds;
    int borrowLimit;

public:
    Member(int id, const string& name, string email="", string phone="", int limit = 3)
        : memberId(id), name(name), borrowLimit(limit) {}

    virtual ~Member() = default;

    int getMemberId() const { return memberId; }
    string getName() const { return name; }
    int getBorrowLimit() const { return borrowLimit; }

    virtual void borrowItem(int itemId) {
        if ((int)borrowedItemIds.size() >= borrowLimit)
            throw MemberLimitExceededException(name, borrowLimit);
        borrowedItemIds.push_back(itemId);
    }

    void returnItem(int itemId) {
        auto it = find(borrowedItemIds.begin(), borrowedItemIds.end(), itemId);
        if (it != borrowedItemIds.end()) borrowedItemIds.erase(it);
    }

    virtual string getMemberType() const { return "Standard"; }
    virtual double getFineMultiplier() const { return 1.0; }
};

class PremiumMember : public Member {
public:
    PremiumMember(int id, const string& name, string email="", string phone="")
        : Member(id, name, email, phone, 6) {}

    string getMemberType() const override { return "Premium"; }
    double getFineMultiplier() const override { return 0.5; }
};

// ============================================================
//  LIBRARY CLASS
// ============================================================

class Library {
private:
    string libraryName;
    Catalogue<LibraryItem*> catalogue;
    vector<Member*> members;

public:
    explicit Library(const string& name) : libraryName(name) {}

    ~Library() {
        for (auto item : catalogue.getItems()) delete item;
        for (auto m : members) delete m;
    }

    Library& operator+=(LibraryItem* item) {
        catalogue.addItem(item);
        return *this;
    }

    void addMember(Member* m) { members.push_back(m); }

    Member* findMember(int id) const {
        for (auto& m : members)
            if (m->getMemberId() == id) return m;
        throw runtime_error("Member not found.");
    }

    void borrowItem(int memberId, int itemId) {
        LibraryItem* item = catalogue.findById(itemId);
        if (!item->isAvailable()) throw ItemNotAvailableException(itemId);
        Member* member = findMember(memberId);
        member->borrowItem(itemId);
        item->setAvailable(false);
    }

    double returnItem(int memberId, int itemId, int daysOverdue = 0) {
        LibraryItem* item = catalogue.findById(itemId);
        Member* member = findMember(memberId);
        member->returnItem(itemId);
        item->setAvailable(true);
        return item->calculateFine(daysOverdue) * member->getFineMultiplier();
    }

    friend ostream& operator<<(ostream& os, const Library& lib) {
        os << "\n======== " << lib.libraryName << " ========\n";
        lib.catalogue.printAll();
        os << "Members: " << lib.members.size() << " | Items: " << lib.catalogue.size() << "\n";
        return os;
    }
};

// ============================================================
//  MAIN FUNCTION: Console Demo
// ============================================================

int main() {
    cout << "=============================================\n";
    cout << "   LIBRARY MANAGEMENT SYSTEM - CONSOLE DEMO  \n";
    cout << "=============================================\n\n";

    try {
        Library lib("University Central Library");

        cout << "[+] Adding items to catalogue...\n";
        lib += new Book(1, "The C++ Programming Language", 2013, "Bjarne Stroustrup", "ISBN-1", "Programming");
        lib += new Book(2, "Clean Code", 2008, "Robert Martin", "ISBN-2", "Software");
        lib += new Magazine(3, "National Geographic", 2024, 152, "NG Society", "Nature");

        cout << "[+] Adding members...\n";
        lib.addMember(new Member(101, "Alice Johnson"));
        lib.addMember(new PremiumMember(102, "Bob Smith"));

        cout << lib;

        cout << "\n[+] Testing Borrowing...\n";
        lib.borrowItem(101, 1);
        cout << "  Alice borrowed 'The C++ Programming Language'\n";
        
        lib.borrowItem(102, 3);
        cout << "  Bob borrowed 'National Geographic'\n";

        cout << "\n[+] Testing Polymorphic Fines...\n";
        double fineAlice = lib.returnItem(101, 1, 5); // 5 days late
        cout << "  Alice returned Book (ID:1) 5 days late. Fine: $" << fixed << setprecision(2) << fineAlice << "\n";

        double fineBob = lib.returnItem(102, 3, 5); // 5 days late
        cout << "  Bob returned Magazine (ID:3) 5 days late. Fine: $" << fineBob << " (Premium 50% discount!)\n";

        cout << "\n--- Exception Handling Demo ---\n";
        try {
            lib.borrowItem(101, 999); // Non-existent item
        } catch (const exception& e) {
            cout << "  Caught expected exception: " << e.what() << "\n";
        }

    } catch (const exception& e) {
        cerr << "FATAL ERROR: " << e.what() << endl;
    }

    cout << "\n[Done] All OOP principles demonstrated successfully!\n";
    return 0;
}
