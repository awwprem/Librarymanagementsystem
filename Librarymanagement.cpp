/*
    Library Management System
    --------------------------------
    A console-based Library Management System built with C++ classes.

    Features:
      1. Add / list books (with structured details)
      2. Add / list members
      3. Issue a book to a member
      4. Return a book
      5. Search books by title or author
      6. Data is persisted to text files (books.txt, members.txt, issues.txt)
         so records survive between program runs.

    Compile:
      g++ -std=c++17 -O2 -o library library_management_system.cpp

    Run:
      ./library
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>

using namespace std;

// ---------- Utility helpers ----------

// Reads a full line of input safely (clears leftover newline in the buffer).
static string readLine(const string &prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

static int readInt(const string &prompt) {
    while (true) {
        cout << prompt;
        string line;
        getline(cin, line);
        try {
            return stoi(line);
        } catch (...) {
            cout << "  Please enter a valid number.\n";
        }
    }
}

// Case-insensitive substring search, used for search-by-title/author.
static string toLower(const string &s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static bool containsIgnoreCase(const string &haystack, const string &needle) {
    return toLower(haystack).find(toLower(needle)) != string::npos;
}

// Escapes '|' in a field so our simple pipe-delimited file format stays valid.
static string sanitize(string s) {
    replace(s.begin(), s.end(), '|', '/');
    return s;
}

// ---------- Data structures (structs) ----------

struct Book {
    int id;
    string title;
    string author;
    string isbn;
    int totalCopies;
    int availableCopies;
};

struct Member {
    int id;
    string name;
    string contact;
};

struct IssueRecord {
    int bookId;
    int memberId;
    bool returned;
};

// ---------- Library class (core logic) ----------

class Library {
private:
    vector<Book> books;
    vector<Member> members;
    vector<IssueRecord> issues;

    int nextBookId = 1;
    int nextMemberId = 1;

    const string booksFile   = "books.txt";
    const string membersFile = "members.txt";
    const string issuesFile  = "issues.txt";

    Book* findBookById(int id) {
        for (auto &b : books) if (b.id == id) return &b;
        return nullptr;
    }

    Member* findMemberById(int id) {
        for (auto &m : members) if (m.id == id) return &m;
        return nullptr;
    }

public:
    // ---------- Persistence ----------

    void load() {
        // Books: id|title|author|isbn|total|available
        ifstream fb(booksFile);
        if (fb.is_open()) {
            string line;
            while (getline(fb, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string tok;
                Book b;
                getline(ss, tok, '|'); b.id = stoi(tok);
                getline(ss, b.title, '|');
                getline(ss, b.author, '|');
                getline(ss, b.isbn, '|');
                getline(ss, tok, '|'); b.totalCopies = stoi(tok);
                getline(ss, tok, '|'); b.availableCopies = stoi(tok);
                books.push_back(b);
                nextBookId = max(nextBookId, b.id + 1);
            }
        }

        // Members: id|name|contact
        ifstream fm(membersFile);
        if (fm.is_open()) {
            string line;
            while (getline(fm, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string tok;
                Member m;
                getline(ss, tok, '|'); m.id = stoi(tok);
                getline(ss, m.name, '|');
                getline(ss, m.contact, '|');
                members.push_back(m);
                nextMemberId = max(nextMemberId, m.id + 1);
            }
        }

        // Issues: bookId|memberId|returned(0/1)
        ifstream fi(issuesFile);
        if (fi.is_open()) {
            string line;
            while (getline(fi, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string tok;
                IssueRecord r;
                getline(ss, tok, '|'); r.bookId = stoi(tok);
                getline(ss, tok, '|'); r.memberId = stoi(tok);
                getline(ss, tok, '|'); r.returned = (stoi(tok) != 0);
                issues.push_back(r);
            }
        }
    }

    void save() {
        ofstream fb(booksFile, ios::trunc);
        for (auto &b : books) {
            fb << b.id << "|" << b.title << "|" << b.author << "|"
               << b.isbn << "|" << b.totalCopies << "|" << b.availableCopies << "\n";
        }

        ofstream fm(membersFile, ios::trunc);
        for (auto &m : members) {
            fm << m.id << "|" << m.name << "|" << m.contact << "\n";
        }

        ofstream fi(issuesFile, ios::trunc);
        for (auto &r : issues) {
            fi << r.bookId << "|" << r.memberId << "|" << (r.returned ? 1 : 0) << "\n";
        }
    }

    // ---------- Book management ----------

    void addBook() {
        Book b;
        b.id = nextBookId++;
        b.title  = sanitize(readLine("  Title: "));
        b.author = sanitize(readLine("  Author: "));
        b.isbn   = sanitize(readLine("  ISBN: "));
        b.totalCopies = readInt("  Number of copies: ");
        if (b.totalCopies < 0) b.totalCopies = 0;
        b.availableCopies = b.totalCopies;
        books.push_back(b);
        save();
        cout << "  Added \"" << b.title << "\" with Book ID " << b.id << ".\n";
    }

    void listBooks() {
        if (books.empty()) {
            cout << "  No books in the catalog yet.\n";
            return;
        }
        cout << "\n  ID   Title                          Author                 ISBN            Avail/Total\n";
        cout << "  ---------------------------------------------------------------------------------------\n";
        for (auto &b : books) {
            printf("  %-4d %-30s %-22s %-15s %d/%d\n",
                   b.id, b.title.c_str(), b.author.c_str(), b.isbn.c_str(),
                   b.availableCopies, b.totalCopies);
        }
    }

    void searchBooks() {
        string term = readLine("  Enter title or author keyword: ");
        bool found = false;
        cout << "\n  Search results:\n";
        for (auto &b : books) {
            if (containsIgnoreCase(b.title, term) || containsIgnoreCase(b.author, term)) {
                printf("  ID %-4d %-30s by %-22s (%d/%d available)\n",
                       b.id, b.title.c_str(), b.author.c_str(),
                       b.availableCopies, b.totalCopies);
                found = true;
            }
        }
        if (!found) cout << "  No matching books found.\n";
    }

    // ---------- Member management ----------

    void addMember() {
        Member m;
        m.id = nextMemberId++;
        m.name    = sanitize(readLine("  Member name: "));
        m.contact = sanitize(readLine("  Contact (phone/email): "));
        members.push_back(m);
        save();
        cout << "  Added member \"" << m.name << "\" with Member ID " << m.id << ".\n";
    }

    void listMembers() {
        if (members.empty()) {
            cout << "  No members registered yet.\n";
            return;
        }
        cout << "\n  ID   Name                     Contact\n";
        cout << "  ---------------------------------------------\n";
        for (auto &m : members) {
            printf("  %-4d %-24s %-20s\n", m.id, m.name.c_str(), m.contact.c_str());
        }
    }

    // ---------- Issue / Return ----------

    void issueBook() {
        listBooks();
        int bookId = readInt("  Enter Book ID to issue: ");
        Book *b = findBookById(bookId);
        if (!b) { cout << "  Book not found.\n"; return; }
        if (b->availableCopies <= 0) { cout << "  No copies available right now.\n"; return; }

        listMembers();
        int memberId = readInt("  Enter Member ID: ");
        Member *m = findMemberById(memberId);
        if (!m) { cout << "  Member not found.\n"; return; }

        b->availableCopies--;
        issues.push_back({bookId, memberId, false});
        save();
        cout << "  Issued \"" << b->title << "\" to " << m->name << ".\n";
    }

    void returnBook() {
        // Show this member's currently issued (not yet returned) books.
        int memberId = readInt("  Enter Member ID returning a book: ");
        Member *m = findMemberById(memberId);
        if (!m) { cout << "  Member not found.\n"; return; }

        vector<int> openIndices;
        cout << "\n  Books currently issued to " << m->name << ":\n";
        for (size_t i = 0; i < issues.size(); i++) {
            if (issues[i].memberId == memberId && !issues[i].returned) {
                Book *b = findBookById(issues[i].bookId);
                cout << "  [" << openIndices.size() << "] "
                     << (b ? b->title : "Unknown Book") << " (Book ID " << issues[i].bookId << ")\n";
                openIndices.push_back((int)i);
            }
        }
        if (openIndices.empty()) {
            cout << "  This member has no books currently issued.\n";
            return;
        }

        int choice = readInt("  Enter the [number] of the book being returned: ");
        if (choice < 0 || choice >= (int)openIndices.size()) {
            cout << "  Invalid selection.\n";
            return;
        }
        int idx = openIndices[choice];
        issues[idx].returned = true;
        Book *b = findBookById(issues[idx].bookId);
        if (b) b->availableCopies++;
        save();
        cout << "  Return recorded" << (b ? (" for \"" + b->title + "\".") : ".") << "\n";
    }

    // ---------- Menu ----------

    void run() {
        load();
        cout << "=========================================\n";
        cout << "     LIBRARY MANAGEMENT SYSTEM\n";
        cout << "=========================================\n";

        while (true) {
            cout << "\n--------------- MENU ---------------\n";
            cout << " 1. Add Book\n";
            cout << " 2. List Books\n";
            cout << " 3. Add Member\n";
            cout << " 4. List Members\n";
            cout << " 5. Issue Book\n";
            cout << " 6. Return Book\n";
            cout << " 7. Search Books (by title/author)\n";
            cout << " 8. Exit\n";
            int choice = readInt("Choose an option: ");

            switch (choice) {
                case 1: addBook(); break;
                case 2: listBooks(); break;
                case 3: addMember(); break;
                case 4: listMembers(); break;
                case 5: issueBook(); break;
                case 6: returnBook(); break;
                case 7: searchBooks(); break;
                case 8:
                    save();
                    cout << "Data saved. Goodbye!\n";
                    return;
                default:
                    cout << "  Invalid option, please choose 1-8.\n";
            }
        }
    }
};

int main() {
    Library library;
    library.run();
    return 0;
}
