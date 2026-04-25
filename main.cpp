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

    // ----------------------------------------------------------
    // 1. Create the Library (Main System Class)
    // ----------------------------------------------------------
    Library lib("University Central Library");

    // ----------------------------------------------------------
    // 2. Create Book and Magazine objects, add via overloaded +=
    //    Demonstrates: Inheritance, Operator Overloading (+=)
    // ----------------------------------------------------------
    std::cout << "[+] Adding items using overloaded += operator...\n\n";

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
    std::cout << lib;

    // ----------------------------------------------------------
    // 4. Demonstrate POLYMORPHISM
    //    Loop through a vector<LibraryItem*> and call virtual methods
    // ----------------------------------------------------------
    std::cout << "\n--- Polymorphism Demo: Calling virtual methods ---\n";
    const auto& items = lib.getCatalogue().getItems();
    for (const auto& item : items) {
        std::cout << "  Type: " << std::left << std::setw(10) << item->getType()
                  << " | Fine (5 days overdue): $"
                  << std::fixed << std::setprecision(2)
                  << item->calculateFine(5) << "\n";
    }

    // ----------------------------------------------------------
    // 5. Create Members (Standard and Premium)
    //    Demonstrates: Inheritance (PremiumMember : Member)
    // ----------------------------------------------------------
    std::cout << "\n[+] Adding members...\n";
    lib.addMember(new Member(101, "Alice Johnson", "alice@uni.edu", "555-0101"));
    lib.addMember(new Member(102, "Charlie Brown", "charlie@uni.edu", "555-0103"));
    lib.addMember(new PremiumMember(103, "Bob Smith", "bob@uni.edu", "555-0102", 0.5));

    for (const auto& m : lib.getMembers()) {
        std::cout << "  " << m->getMemberType() << " Member: "
                  << m->getName() << " (Limit: " << m->getBorrowLimit() << ")\n";
    }

    // ----------------------------------------------------------
    // 6. Simulate Borrowing and Returning
    //    Demonstrates: Exception Handling, Polymorphic fine calc
    // ----------------------------------------------------------
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
              << " (Premium 50% discount!)\n";

    // ----------------------------------------------------------
    // 7. Demonstrate Template: search by ID using Catalogue<T>
    //    Demonstrates: Templates
    // ----------------------------------------------------------
    std::cout << "\n--- Template Search Demo ---\n";
    try {
        LibraryItem* found = lib.getCatalogue().findById(3);
        std::cout << "  Found: " << *found << "\n";
    } catch (const ItemNotFoundException& e) {
        std::cout << "  Error: " << e.what() << "\n";
    }

    // ----------------------------------------------------------
    // 8. Trigger and Catch Custom Exceptions
    //    Demonstrates: Exception Handling
    // ----------------------------------------------------------
    std::cout << "\n--- Exception Handling Demo ---\n";

    // a) ItemNotFoundException: search for non-existent ID
    try {
        lib.getCatalogue().findById(999);
    } catch (const ItemNotFoundException& e) {
        std::cout << "  Caught ItemNotFoundException: " << e.what() << "\n";
    }

    // b) MemberLimitExceededException: exceed borrow limit
    try {
        // Alice (Standard, limit=3) borrows 4 items
        lib.borrowItem(101, 2);
        lib.borrowItem(101, 3);
        lib.borrowItem(101, 4);
        lib.borrowItem(101, 5);  // This should throw
    } catch (const MemberLimitExceededException& e) {
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
    }

    // ----------------------------------------------------------
    // 9. Final state
    // ----------------------------------------------------------
    std::cout << "\n--- Final Library State ---\n";
    std::cout << lib;

    std::cout << "\n[✓] All OOP principles and C++ features demonstrated!\n";
    return 0;
}
