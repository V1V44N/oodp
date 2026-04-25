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
#include <map>
#include <cstring>
#include <algorithm>
#include "library_system.h"

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

std::string getMimeType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css")  != std::string::npos) return "text/css";
    if (path.find(".js")   != std::string::npos) return "application/javascript";
    if (path.find(".json") != std::string::npos) return "application/json";
    if (path.find(".png")  != std::string::npos) return "image/png";
    if (path.find(".ico")  != std::string::npos) return "image/x-icon";
    return "text/plain";
}

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string buildResponse(int status, const std::string& contentType,
                           const std::string& body) {
    std::string statusText = (status == 200) ? "OK" :
                             (status == 404) ? "Not Found" :
                             (status == 400) ? "Bad Request" : "Internal Server Error";
    std::ostringstream r;
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
std::map<std::string, std::string> parseParams(const std::string& data) {
    std::map<std::string, std::string> params;
    std::istringstream stream(data);
    std::string pair;
    while (std::getline(stream, pair, '&')) {
        auto eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string key = pair.substr(0, eq);
            std::string val = pair.substr(eq + 1);
            // Simple URL decode for + and %20
            std::replace(val.begin(), val.end(), '+', ' ');
            params[key] = val;
        }
    }
    return params;
}

// URL-decode a string (basic)
std::string urlDecode(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int val = 0;
            std::istringstream iss(s.substr(i + 1, 2));
            iss >> std::hex >> val;
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
std::string handleAPI(const std::string& method, const std::string& path,
                      const std::string& body) {
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
        if (qPos != std::string::npos) {
            auto params = parseParams(path.substr(qPos + 1));
            if (params.count("id")) {
                int id = std::stoi(params["id"]);
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
            int memberId = std::stoi(params["memberId"]);
            int itemId = std::stoi(params["itemId"]);
            gLibrary->borrowItem(memberId, itemId);
            return buildResponse(200, "application/json",
                "{\"success\":true,\"message\":\"Item borrowed successfully.\"}");
        } catch (const std::exception& e) {
            return buildResponse(400, "application/json",
                std::string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    // POST /api/return  (body: memberId=X&itemId=Y&daysOverdue=Z)
    if (method == "POST" && path == "/api/return") {
        auto params = parseParams(body);
        try {
            int memberId = std::stoi(params["memberId"]);
            int itemId = std::stoi(params["itemId"]);
            int days = params.count("daysOverdue") ? std::stoi(params["daysOverdue"]) : 0;
            double fine = gLibrary->returnItem(memberId, itemId, days);
            std::ostringstream o;
            o << "{\"success\":true,\"fine\":" << std::fixed << std::setprecision(2) << fine
              << ",\"message\":\"Item returned. Fine: $" << std::fixed << std::setprecision(2) << fine << "\"}";
            return buildResponse(200, "application/json", o.str());
        } catch (const std::exception& e) {
            return buildResponse(400, "application/json",
                std::string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    // POST /api/items/add  (body: type=Book&title=...&year=...&...)
    if (method == "POST" && path == "/api/items/add") {
        auto params = parseParams(body);
        try {
            int id = gLibrary->getNextItemId();
            std::string type = params["type"];
            std::string title = urlDecode(params["title"]);
            int year = std::stoi(params["year"]);

            if (type == "Book") {
                std::string author = urlDecode(params["author"]);
                std::string isbn = urlDecode(params["isbn"]);
                std::string genre = urlDecode(params["genre"]);
                *gLibrary += new Book(id, title, year, author, isbn, genre);
            } else {
                int issue = std::stoi(params["issueNumber"]);
                std::string publisher = urlDecode(params["publisher"]);
                std::string category = urlDecode(params["category"]);
                *gLibrary += new Magazine(id, title, year, issue, publisher, category);
            }
            return buildResponse(200, "application/json",
                "{\"success\":true,\"message\":\"Item added with ID " + std::to_string(id) + "\"}");
        } catch (const std::exception& e) {
            return buildResponse(400, "application/json",
                std::string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    // POST /api/members/add
    if (method == "POST" && path == "/api/members/add") {
        auto params = parseParams(body);
        try {
            int id = gLibrary->getNextMemberId();
            std::string name = urlDecode(params["name"]);
            std::string email = urlDecode(params["email"]);
            std::string phone = urlDecode(params["phone"]);
            std::string type = params.count("type") ? params["type"] : "Standard";

            if (type == "Premium")
                gLibrary->addMember(new PremiumMember(id, name, email, phone));
            else
                gLibrary->addMember(new Member(id, name, email, phone));

            return buildResponse(200, "application/json",
                "{\"success\":true,\"message\":\"Member added with ID " + std::to_string(id) + "\"}");
        } catch (const std::exception& e) {
            return buildResponse(400, "application/json",
                std::string("{\"success\":false,\"message\":\"") + e.what() + "\"}");
        }
    }

    return buildResponse(404, "application/json", "{\"error\":\"API endpoint not found\"}");
}

// ---- Handle a single client connection ----
void handleClient(socket_t client) {
    char buffer[8192] = {0};
    int received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) { CLOSE_SOCKET(client); return; }

    std::string request(buffer, received);

    // Parse method and path
    std::string method, path;
    std::istringstream reqStream(request);
    reqStream >> method >> path;

    // Handle OPTIONS (CORS preflight)
    if (method == "OPTIONS") {
        std::string resp = buildResponse(200, "text/plain", "");
        send(client, resp.c_str(), (int)resp.size(), 0);
        CLOSE_SOCKET(client);
        return;
    }

    // Extract body for POST requests
    std::string body;
    auto bodyPos = request.find("\r\n\r\n");
    if (bodyPos != std::string::npos)
        body = request.substr(bodyPos + 4);

    std::string response;

    // Route: API endpoints
    if (path.find("/api/") == 0) {
        response = handleAPI(method, path, body);
    }
    // Route: serve static files
    else {
        std::string filePath = (path == "/") ? "index.html" : path.substr(1);

        // Security: prevent directory traversal
        if (filePath.find("..") != std::string::npos) {
            response = buildResponse(400, "text/plain", "Bad Request");
        } else {
            std::string content = readFile(filePath);
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
        std::cerr << "WSAStartup failed!\n";
        return 1;
    }
    #endif

    initSampleData();
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════╗\n";
    std::cout << "  ║   Library Management System - Server     ║\n";
    std::cout << "  ║   http://localhost:8080                   ║\n";
    std::cout << "  ╚══════════════════════════════════════════╝\n";
    std::cout << "\n  [✓] Sample data loaded: "
              << gLibrary->getCatalogue().size() << " items, "
              << gLibrary->getMembers().size() << " members\n";
    std::cout << "  [✓] Server starting on port 8080...\n\n";

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket.\n";
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
        std::cerr << "Bind failed. Port 8080 may be in use.\n";
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    std::cout << "  [✓] Listening on http://localhost:8080\n";
    std::cout << "  [i] Press Ctrl+C to stop.\n\n";

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
