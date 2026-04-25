/*
 * ============================================================
 *  Library Management System - Core Header
 *  Demonstrates: Abstraction, Inheritance, Encapsulation,
 *  Polymorphism, Operator Overloading, Templates,
 *  Exception Handling, STL Usage
 * ============================================================
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

// ============================================================
//  CUSTOM EXCEPTION CLASSES (Exception Handling)
// ============================================================

class ItemNotFoundException : public std::runtime_error {
public:
    explicit ItemNotFoundException(int id)
        : std::runtime_error("Item with ID " + std::to_string(id) + " not found.") {}
    explicit ItemNotFoundException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class MemberLimitExceededException : public std::runtime_error {
public:
    explicit MemberLimitExceededException(const std::string& name, int limit)
        : std::runtime_error("Member '" + name + "' reached borrow limit of "
                             + std::to_string(limit) + " items.") {}
};

class ItemNotAvailableException : public std::runtime_error {
public:
    explicit ItemNotAvailableException(int id)
        : std::runtime_error("Item ID " + std::to_string(id) + " is not available.") {}
};

// ============================================================
//  ABSTRACT BASE CLASS: LibraryItem
//  Demonstrates: Abstraction, Encapsulation
// ============================================================

class LibraryItem {
private:
    int id;                 // Unique identifier
    std::string title;      // Title of the item
    bool available;         // Availability status
    int yearPublished;      // Year of publication

public:
    // Constructor
    LibraryItem(int id, const std::string& title, int year)
        : id(id), title(title), available(true), yearPublished(year) {}

    // Virtual destructor for proper polymorphic cleanup
    virtual ~LibraryItem() = default;

    // ---- Pure Virtual Methods (Abstraction) ----
    virtual std::string getDetails() const = 0;
    virtual std::string getType() const = 0;
    virtual double calculateFine(int daysOverdue) const = 0;

    // ---- Getters (Encapsulation) ----
    int getId() const { return id; }
    std::string getTitle() const { return title; }
    bool isAvailable() const { return available; }
    int getYearPublished() const { return yearPublished; }

    // ---- Setters with validation (Encapsulation) ----
    void setTitle(const std::string& t) {
        if (t.empty()) throw std::invalid_argument("Title cannot be empty.");
        title = t;
    }
    void setAvailable(bool s) { available = s; }
    void setYearPublished(int y) {
        if (y < 0 || y > 2030)
            throw std::invalid_argument("Invalid publication year.");
        yearPublished = y;
    }

    // ---- Operator Overloading: == compares by ID ----
    bool operator==(const LibraryItem& other) const {
        return id == other.id;
    }

    // ---- Operator Overloading: << for printing ----
    friend std::ostream& operator<<(std::ostream& os, const LibraryItem& item) {
        os << item.getDetails();
        return os;
    }

    // JSON representation (virtual so subclasses extend it)
    virtual std::string toJSON() const {
        std::ostringstream o;
        o << "{\"id\":" << id
          << ",\"title\":\"" << esc(title) << "\""
          << ",\"type\":\"" << getType() << "\""
          << ",\"available\":" << (available ? "true" : "false")
          << ",\"yearPublished\":" << yearPublished;
        return o.str(); // Note: subclasses close the brace
    }

protected:
    // Utility: escape a string for JSON output
    static std::string esc(const std::string& s) {
        std::string r;
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
    std::string author;
    std::string isbn;
    std::string genre;

public:
    Book(int id, const std::string& title, int year,
         const std::string& author, const std::string& isbn,
         const std::string& genre)
        : LibraryItem(id, title, year),
          author(author), isbn(isbn), genre(genre) {}

    // Getters
    std::string getAuthor() const { return author; }
    std::string getISBN() const { return isbn; }
    std::string getGenre() const { return genre; }

    // Setters with validation
    void setAuthor(const std::string& a) {
        if (a.empty()) throw std::invalid_argument("Author cannot be empty.");
        author = a;
    }

    // ---- Polymorphism: override virtual methods ----
    std::string getDetails() const override {
        std::ostringstream o;
        o << "[Book] ID:" << getId()
          << " | " << getTitle()
          << " | by " << author
          << " | ISBN:" << isbn
          << " | " << genre
          << " | " << getYearPublished()
          << " | " << (isAvailable() ? "Available" : "Borrowed");
        return o.str();
    }

    std::string getType() const override { return "Book"; }

    // Books charge $0.50 per overdue day
    double calculateFine(int daysOverdue) const override {
        return (daysOverdue > 0) ? daysOverdue * 0.50 : 0.0;
    }

    std::string toJSON() const override {
        std::ostringstream o;
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
    std::string publisher;
    std::string category;

public:
    Magazine(int id, const std::string& title, int year,
             int issue, const std::string& publisher,
             const std::string& category)
        : LibraryItem(id, title, year),
          issueNumber(issue), publisher(publisher), category(category) {}

    // Getters
    int getIssueNumber() const { return issueNumber; }
    std::string getPublisher() const { return publisher; }
    std::string getCategory() const { return category; }

    // Setters
    void setIssueNumber(int n) {
        if (n < 0) throw std::invalid_argument("Issue number cannot be negative.");
        issueNumber = n;
    }

    // ---- Polymorphism: override virtual methods ----
    std::string getDetails() const override {
        std::ostringstream o;
        o << "[Magazine] ID:" << getId()
          << " | " << getTitle()
          << " | Issue #" << issueNumber
          << " | " << publisher
          << " | " << category
          << " | " << getYearPublished()
          << " | " << (isAvailable() ? "Available" : "Borrowed");
        return o.str();
    }

    std::string getType() const override { return "Magazine"; }

    // Magazines charge $1.00 per overdue day (higher rate)
    double calculateFine(int daysOverdue) const override {
        return (daysOverdue > 0) ? daysOverdue * 1.00 : 0.0;
    }

    std::string toJSON() const override {
        std::ostringstream o;
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
    std::vector<T> items;   // STL vector to store items

public:
    // Add an item to the catalogue
    void addItem(T item) {
        items.push_back(item);
    }

    // Remove an item by ID
    void removeItem(int id) {
        auto it = std::remove_if(items.begin(), items.end(),
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
    void printAll(std::ostream& os = std::cout) const {
        os << "\n========= CATALOGUE =========\n";
        for (const auto& item : items)
            os << *item << "\n";
        os << "=============================\n";
    }

    // Accessors
    const std::vector<T>& getItems() const { return items; }
    size_t size() const { return items.size(); }

    size_t countAvailable() const {
        return std::count_if(items.begin(), items.end(),
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
    std::string name;
    std::string email;
    std::string phone;
    std::vector<int> borrowedItemIds;   // Borrow history
    int borrowLimit;

public:
    Member(int id, const std::string& name, const std::string& email,
           const std::string& phone, int limit = 3)
        : memberId(id), name(name), email(email),
          phone(phone), borrowLimit(limit) {}

    virtual ~Member() = default;

    // ---- Getters ----
    int getMemberId() const { return memberId; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    std::string getPhone() const { return phone; }
    int getBorrowLimit() const { return borrowLimit; }
    const std::vector<int>& getBorrowedItems() const { return borrowedItemIds; }
    int getBorrowedCount() const { return (int)borrowedItemIds.size(); }

    // ---- Setters with validation ----
    void setName(const std::string& n) {
        if (n.empty()) throw std::invalid_argument("Name cannot be empty.");
        name = n;
    }
    void setEmail(const std::string& e) {
        if (e.empty() || e.find('@') == std::string::npos)
            throw std::invalid_argument("Invalid email address.");
        email = e;
    }
    void setPhone(const std::string& p) { phone = p; }

    // ---- Borrow / Return ----
    virtual void borrowItem(int itemId) {
        if ((int)borrowedItemIds.size() >= borrowLimit)
            throw MemberLimitExceededException(name, borrowLimit);
        borrowedItemIds.push_back(itemId);
    }

    void returnItem(int itemId) {
        auto it = std::find(borrowedItemIds.begin(), borrowedItemIds.end(), itemId);
        if (it != borrowedItemIds.end())
            borrowedItemIds.erase(it);
    }

    bool hasBorrowed(int itemId) const {
        return std::find(borrowedItemIds.begin(), borrowedItemIds.end(), itemId)
               != borrowedItemIds.end();
    }

    // Virtual: member type string
    virtual std::string getMemberType() const { return "Standard"; }
    // Virtual: fine multiplier (Premium members get discount)
    virtual double getFineMultiplier() const { return 1.0; }

    // JSON representation
    virtual std::string toJSON() const {
        std::ostringstream o;
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
    PremiumMember(int id, const std::string& name,
                  const std::string& email, const std::string& phone,
                  double discount = 0.5)
        : Member(id, name, email, phone, 6),   // Premium: 6-item limit
          discountRate(discount) {}

    double getDiscountRate() const { return discountRate; }

    std::string getMemberType() const override { return "Premium"; }

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
    std::string libraryName;
    Catalogue<LibraryItem*> catalogue;
    std::vector<Member*> members;
    std::map<int, int> borrowMap;   // itemId -> memberId

public:
    explicit Library(const std::string& name) : libraryName(name) {}

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
        throw std::runtime_error("Member ID " + std::to_string(id) + " not found.");
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
    const std::string& getName() const { return libraryName; }
    const Catalogue<LibraryItem*>& getCatalogue() const { return catalogue; }
    Catalogue<LibraryItem*>& getCatalogue() { return catalogue; }
    const std::vector<Member*>& getMembers() const { return members; }
    const std::map<int, int>& getBorrowMap() const { return borrowMap; }

    // ---- Operator Overloading: << prints library info ----
    friend std::ostream& operator<<(std::ostream& os, const Library& lib) {
        os << "\n======== " << lib.libraryName << " ========\n";
        lib.catalogue.printAll(os);
        os << "Members : " << lib.members.size() << "\n";
        os << "Items   : " << lib.catalogue.size() << "\n";
        os << "Available: " << lib.catalogue.countAvailable() << "\n";
        os << "===================================\n";
        return os;
    }

    // ---- JSON helpers for the web API ----
    std::string getStatsJSON() const {
        int books = 0, mags = 0, borrowed = 0, premium = 0;
        for (auto i : catalogue.getItems()) {
            if (i->getType() == "Book") books++; else mags++;
            if (!i->isAvailable()) borrowed++;
        }
        for (auto m : members)
            if (m->getMemberType() == "Premium") premium++;
        std::ostringstream o;
        o << "{\"totalItems\":" << catalogue.size()
          << ",\"totalBooks\":" << books
          << ",\"totalMagazines\":" << mags
          << ",\"available\":" << catalogue.countAvailable()
          << ",\"borrowed\":" << borrowed
          << ",\"totalMembers\":" << members.size()
          << ",\"premiumMembers\":" << premium << "}";
        return o.str();
    }

    std::string getItemsJSON() const {
        std::ostringstream o;
        o << "[";
        auto& items = catalogue.getItems();
        for (size_t i = 0; i < items.size(); i++) {
            if (i) o << ",";
            o << items[i]->toJSON();
        }
        o << "]";
        return o.str();
    }

    std::string getMembersJSON() const {
        std::ostringstream o;
        o << "[";
        for (size_t i = 0; i < members.size(); i++) {
            if (i) o << ",";
            o << members[i]->toJSON();
        }
        o << "]";
        return o.str();
    }

    std::string searchItemJSON(int id) const {
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
