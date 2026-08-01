Library Management System (C++)
A console-based Library Management System built with C++ classes. It supports managing books and members, issuing and returning books, and searching the catalog — with all data persisted to text files so records survive between runs.
Features
Books — add new books (title, author, ISBN, copy count) and list the full catalog
Members — register new members (name, contact) and list all members
Issue / Return — issue an available book to a member, and record returns per member
Search — case-insensitive search across book titles and authors
Persistence — data is automatically saved to and loaded from plain text files:
`books.txt` — book records
`members.txt` — member records
`issues.txt` — issue/return history
Requirements
A C++ compiler that supports C++17 (e.g. `g++`)
Build
```bash
g++ -std=c++17 -O2 -o library library\_management\_system.cpp
```
Run
```bash
./library
```
On startup, the program automatically loads any existing `books.txt`, `members.txt`, and `issues.txt` files from the working directory. If none exist, it starts with an empty catalog.
Menu Options
```
1. Add Book
2. List Books
3. Add Member
4. List Members
5. Issue Book
6. Return Book
7. Search Books (by title/author)
8. Exit
```
Add Book — prompts for title, author, ISBN, and number of copies; assigns a new Book ID.
List Books — shows all books with available/total copy counts.
Add Member — prompts for name and contact info; assigns a new Member ID.
List Members — shows all registered members.
Issue Book — lists books and members, then issues a chosen book (if a copy is available) to a chosen member.
Return Book — shows the books currently issued to a given member and records the selected return.
Search Books — searches titles and authors for a keyword (case-insensitive).
Exit — saves all data and closes the program.
Data is also saved immediately after every add, issue, and return, so progress isn't lost even if the program exits unexpectedly.
Data File Format
Each file uses a simple pipe-delimited (`|`) format, one record per line:
File	Format
`books.txt`	`id|title|author|isbn|totalCopies|availableCopies`
`members.txt`	`id|name|contact`
`issues.txt`	`bookId|memberId|returned(0/1)`
Any `|` characters typed into a field are automatically replaced with `/` to keep the file format valid.
Project Structure
```
library\_management\_system.cpp   # Full application source (single file)
books.txt                       # Generated at runtime — book records
members.txt                     # Generated at runtime — member records
issues.txt                      # Generated at runtime — issue/return records
```
Key Components
Component	Purpose
`Book`, `Member`, `IssueRecord` (structs)	Core data records
`Library` (class)	Holds all in-memory data and implements every feature
`Library::load` / `Library::save`	Reads/writes the three text files
`Library::addBook` / `listBooks` / `searchBooks`	Book management
`Library::addMember` / `listMembers`	Member management
`Library::issueBook` / `returnBook`	Issue and return workflow
`Library::run`	Displays the menu and dispatches user choices
