# 📚 LibraVault: C++ OOP Architecture & Workflow

This document provides a technical overview of the Object-Oriented Programming (OOP) principles and system architecture behind the LibraVault Library Management System.

---

## 🏗️ Core OOP Principles

The C++ backend is designed as a university-level demonstration of the four pillars of OOP:

### 1. Abstraction
The `LibraryItem` class is an **Abstract Base Class (ABC)**. It defines the "contract" for any item in the library without specifying how they work.
- **Pure Virtual Methods**: `getDetails()`, `getType()`, and `calculateFine()` have no implementation in the base class, forcing subclasses to provide their own logic.

### 2. Inheritance
A hierarchical structure is used to reuse code and represent relationships:
- **`Book`** and **`Magazine`** inherit from `LibraryItem`.
- **`PremiumMember`** inherits from `Member`, extending its capabilities (higher borrow limits and fine discounts).

### 3. Encapsulation
All classes use private data members to protect the internal state.
- Access is controlled via **Getters** and **Setters**.
- Setters include **Validation logic** (e.g., preventing empty titles or negative years) to maintain data integrity.

### 4. Polymorphism
The system heavily utilizes **Dynamic Dispatch** (Late Binding):
- **Runtime Polymorphism**: The `Library` class manages a `vector<LibraryItem*>`. When `item->calculateFine()` is called, C++ automatically executes the correct version (Book vs. Magazine) based on the actual object type at runtime.

---

## 🚀 Advanced C++ Features

| Feature | Implementation |
| :--- | :--- |
| **Templates** | The `Catalogue<T>` class is a generic template, allowing it to store any type of item (`Book*`, `Magazine*`, etc.) while maintaining type safety. |
| **Operator Overloading** | The `Library` class overloads `+=` to add items and `<<` to print the entire system status to the console. `LibraryItem` overloads `==` for ID comparison. |
| **Exception Handling** | Custom exceptions like `ItemNotFoundException` and `MemberLimitExceededException` are used to handle errors gracefully instead of crashing. |
| **STL Containers** | Extensive use of `std::vector` for lists, `std::map` for borrow tracking, and `std::string` for text processing. |

---

## 📊 System Workflow Diagram

The workflow illustrates how the JavaScript frontend communicates with the C++ backend and how data flows through the OOP structures.

### Workflow Sequence:
1. **Frontend (JS)**: Sends an HTTP Request (e.g., `GET /api/stats`).
2. **Backend (Server)**: Winsock2 listener receives the request and identifies the endpoint.
3. **Logic (Library)**: Calls the corresponding method on the `Library` class.
4. **Data (Catalogue)**: Interacts with the `Catalogue<T>` template to fetch/modify `LibraryItem` objects.
5. **Transformation**: The object uses its `toJSON()` method (polymorphic) to create a JSON string.
6. **Response**: The server wraps the JSON in an HTTP 200 OK header and sends it back to the browser.

---

## 🗂️ Class Relationship Diagram

- **Library**: The main controller. Has one `Catalogue` and many `Members`.
- **Catalogue**: A template container. Holds many `LibraryItem` pointers.
- **LibraryItem**: Abstract base. Inherited by `Book` and `Magazine`.
- **Member**: Base class for users. Inherited by `PremiumMember`.

---

## 🛠️ Integrated Web API

The `server.cpp` file acts as a bridge. It listens for HTTP requests, parses the URL/Body, and calls the appropriate methods on the global `Library` object (`gLibrary`). 

1. **Request**: `POST /api/borrow`
2. **Parsing**: Server extracts `memberId` and `itemId` from the request.
3. **Execution**: Calls `gLibrary->borrowItem(memberId, itemId)`.
4. **Exception Guard**: The call is wrapped in a `try-catch` block. If a C++ exception is thrown, it's converted to a JSON error response.
5. **Response**: Returns `{"success": true}` or an error message to the frontend.
