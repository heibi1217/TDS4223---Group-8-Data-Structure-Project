#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
using namespace std;

// forward declarations so functions can call each other freely
void addBook();
void editBook();
void deleteBook();
void displayBooks();
void searchBook(string username, bool isAdmin);
void sortBooks();
void borrowBook(string currentCustomer);
void returnBook(string currentCustomer);
int  countBooksInFile();

struct Book {
    string bookID;
    string title;
    string author;
    string category;
    int year;
    int stock;
    string status;
};

struct BookRecord {
    string borrowID;
    string customerID;
    string bookID;
    string borrowDate;
    string returnDate;
    string borrowStatus;
};

// -------------------------------------------------------
// ADT Queue - used in borrow and return book
// Fixed: added 'string item' variable that was missing
// -------------------------------------------------------
class ADTqueue {
    private:
        string queue[10];
        int head, tail;

    public:
        ADTqueue() {
            tail = -1;
            head = 0;
        }

        int empty() {
            if (head == tail + 1) return 1;
            else return 0;
        }

        int full() {
            if (tail == 9) return 1;
            else return 0;
        }

        void append(string newItem) {
            if (!full()) {
                tail++;
                queue[tail] = newItem;
            } else {
                cout << "Queue is Full" << endl;
            }
        }

        string serve() {
            string item = "";   // fixed: item was missing before
            if (!empty()) {
                item = queue[head];
                head++;
                return item;
            } else {
                cout << "Queue is Empty" << endl;
                return "";
            }
        }
};

// -------------------------------------------------------
// Account base class and subclasses
// -------------------------------------------------------
class Account {
    protected:
        string username;
        string password;
        string role;

    public:
        Account() {
            username = "";
            password = "";
            role = "";
        }

        ~Account() {}

        friend void displayAccount(const Account& a);
        friend class Admin;

        string getUsername() { return username; }
        string getPassword() { return password; }
        string getRole()     { return role; }
};

class User : public Account {
    public:
        User() { role = "Customer"; }

        User(string u, string p) {
            username = u;
            password = p;
            role = "Customer";
        }

        ~User() {}
        friend void displayUser(const User& u);

        void setUsername(string u) { username = u; }
        void setPassword(string p) { password = p; }
};

class Admin : public Account {
    public:
        Admin() { role = "Admin"; }

        Admin(string u, string p) {
            username = u;
            password = p;
            role = "Admin";
        }

        ~Admin() {}
        friend void displayAdmin(const Admin& a);

        void setUsername(string u) { username = u; }
        void setPassword(string p) { password = p; }
};

// -------------------------------------------------------
// Register / Login / Logout
// -------------------------------------------------------
void registerUser() {
    string u, p;
    cout << "\n== Register New Customer ==" << endl;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;
    cout << endl;

    string fu, fp, frole;
    ifstream checkFile("Customer.txt");
    while (getline(checkFile, fu, '|') && getline(checkFile, fp, '|') && getline(checkFile, frole)) {
        if (u == fu) {
            cout << "Username already exists! Please choose another." << endl << endl;
            checkFile.close();
            return;
        }
    }
    checkFile.close();

    User* newUser = new User(u, p);
    ofstream file("Customer.txt", ios::app);
    file << newUser->getUsername() << "|" << newUser->getPassword() << "|Customer" << endl;
    file.close();
    delete newUser;

    cout << "Customer registered successfully!" << endl << endl;
}

string loginUser() {
    string u, p, fu, fp, frole;
    cout << "\n== Customer Login ==" << endl;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;
    cout << endl;

    ifstream file("Customer.txt");
    while (getline(file, fu, '|') && getline(file, fp, '|') && getline(file, frole)) {
        if (u == fu && p == fp) {
            file.close();
            cout << "Login successful! Welcome, " << u << "!" << endl;
            return u;
        }
    }
    file.close();
    cout << "Invalid username or password." << endl;
    return "";
}

string loginAdmin() {
    string u, p, fu, fp, frole;
    cout << "\n== Admin Login ==" << endl;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;
    cout << endl;

    ifstream file("Admin.txt");
    while (getline(file, fu, '|') && getline(file, fp, '|') && getline(file, frole)) {
        if (u == fu && p == fp) {
            file.close();
            cout << "Login successful! Welcome, Admin " << u << "!" << endl;
            return u;
        }
    }
    file.close();
    cout << "Invalid username or password." << endl;
    return "";
}

void logout() {
    cout << "\nLogging out... Goodbye!" << endl << endl;
}

// -------------------------------------------------------
// File helpers - count, load, save books
// Fixed: stoi crash when year = "Unknown"
// -------------------------------------------------------
int countBooksInFile() {
    ifstream file("Books.txt");
    if (!file.is_open()) return 0;
    int count = 0;
    string line;
    while (getline(file, line)) {
        if (!line.empty()) count++;
    }
    file.close();
    return count;
}

Book* loadBooks(int& size) {
    size = countBooksInFile();
    if (size == 0) return nullptr;

    Book* bookList = new Book[size];
    ifstream file("Books.txt");
    string tempYear, tempStock;

    for (int i = 0; i < size; i++) {
        getline(file, bookList[i].bookID, '|');
        getline(file, bookList[i].title, '|');
        getline(file, bookList[i].author, '|');
        getline(file, bookList[i].category, '|');
        getline(file, tempYear, '|');
        getline(file, tempStock, '|');
        getline(file, bookList[i].status);

        // fix: some books have "Unknown" year, stoi would crash on that
        try {
            if (!tempYear.empty() && tempYear != "Unknown")
                bookList[i].year = stoi(tempYear);
            else
                bookList[i].year = 0;
        } catch (...) {
            bookList[i].year = 0;
        }

        try {
            if (!tempStock.empty())
                bookList[i].stock = stoi(tempStock);
            else
                bookList[i].stock = 0;
        } catch (...) {
            bookList[i].stock = 0;
        }
    }
    file.close();
    return bookList;
}

void saveBooks(Book* bookList, int size) {
    ofstream file("Books.txt", ios::trunc);
    for (int i = 0; i < size; i++) {
        file << bookList[i].bookID   << "|"
             << bookList[i].title    << "|"
             << bookList[i].author   << "|"
             << bookList[i].category << "|"
             << bookList[i].year     << "|"
             << bookList[i].stock    << "|"
             << bookList[i].status   << endl;
    }
    file.close();
}

// -------------------------------------------------------
// Borrow Book (Wen Zhe's part - unchanged)
// -------------------------------------------------------
void borrowBook(string currentCustomer) {
    cin.ignore();
    cout << "\n== Borrow Book ==" << endl;

    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "No books available in the library." << endl;
        return;
    }

    string id;
    cout << "Enter Book ID to borrow: ";
    cin >> id;

    bool found = false;
    for (int i = 0; i < size; i++) {
        if (bookList[i].bookID == id) {
            found = true;

            if (bookList[i].stock > 0) {
                bookList[i].stock--;

                if (bookList[i].stock == 0)
                    bookList[i].status = "Borrowed";

                ADTqueue borrowQueue;
                borrowQueue.append(currentCustomer);
                string processingUser = borrowQueue.serve();

                ofstream recordFile("BorrowRecords.txt", ios::app);
                recordFile << "BR" << bookList[i].year << id << "|"
                           << processingUser << "|"
                           << bookList[i].bookID << "|"
                           << "2026-06-07|---|Borrowed" << endl;
                recordFile.close();

                cout << "Book '" << bookList[i].title
                     << "' borrowed successfully by " << processingUser << "!" << endl;
            } else {
                cout << "Sorry, this book is out of stock!" << endl;
            }
            break;
        }
    }

    if (found) {
        saveBooks(bookList, size);
    } else {
        cout << "Book ID not found." << endl;
    }

    delete[] bookList;
}

// -------------------------------------------------------
// Return Book (Wen Zhe's part - unchanged)
// -------------------------------------------------------
void returnBook(string currentCustomer) {
    cin.ignore();
    cout << "\n=== Return Book ===" << endl;

    string id;
    cout << "Enter Book ID to return: ";
    cin >> id;

    int size = 0;
    Book* bookList = loadBooks(size);
    bool bookFound = false;

    for (int i = 0; i < size; i++) {
        if (bookList[i].bookID == id) {
            bookList[i].stock++;
            bookList[i].status = "Available";
            bookFound = true;
            break;
        }
    }

    if (!bookFound) {
        cout << "This book was not found in the library system." << endl;
        if (bookList != nullptr) delete[] bookList;
        return;
    }

    ifstream readFile("BorrowRecords.txt");
    if (!readFile.is_open()) {
        cout << "No borrowing records found." << endl;
        delete[] bookList;
        return;
    }

    ofstream tempFile("TempRecords.txt");
    string bID, custID, bkID, bDate, rDate, bStatus;
    bool recordUpdated = false;

    ADTqueue returnQueue;

    while (getline(readFile, bID, '|') && getline(readFile, custID, '|') &&
           getline(readFile, bkID, '|') && getline(readFile, bDate, '|') &&
           getline(readFile, rDate, '|') && getline(readFile, bStatus)) {

        if (custID == currentCustomer && bkID == id && bStatus == "Borrowed" && !recordUpdated) {
            returnQueue.append(custID);
            string processingCust = returnQueue.serve();

            tempFile << bID << "|" << processingCust << "|" << bkID << "|"
                     << bDate << "|2026-06-14|Returned" << endl;
            recordUpdated = true;
        } else {
            tempFile << bID << "|" << custID << "|" << bkID << "|"
                     << bDate << "|" << rDate << "|" << bStatus << endl;
        }
    }

    readFile.close();
    tempFile.close();

    remove("BorrowRecords.txt");
    rename("TempRecords.txt", "BorrowRecords.txt");

    if (recordUpdated) {
        saveBooks(bookList, size);
        cout << "Book returned successfully! Thank you, " << currentCustomer << "!" << endl;
    } else {
        cout << "You have not borrowed this book or it was already returned." << endl;
        for (int i = 0; i < size; i++) {
            if (bookList[i].bookID == id) {
                bookList[i].stock--;
                if (bookList[i].stock == 0)
                    bookList[i].status = "Borrowed";
                break;
            }
        }
        saveBooks(bookList, size);
    }

    delete[] bookList;
}

// -------------------------------------------------------
// Customer and Admin menus
// -------------------------------------------------------
void customerMenu(string username) {
    int choice;
    do {
        cout << "\n=== Customer Menu ===" << endl;
        cout << "1. Borrow Book" << endl;
        cout << "2. Return Book" << endl;
        cout << "3. Search Book" << endl;
        cout << "4. Display Books" << endl;
        cout << "5. View Borrowing Summary" << endl;
        cout << "0. Logout" << endl << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: borrowBook(username); break;
            case 2: returnBook(username); break;
            case 3: searchBook(username, false); break;
            case 4: displayBooks(); break;
            case 5: break;  // Tsui Hern's part
            case 0: break;
            default: cout << "\nInvalid choice. Please try again." << endl;
        }
    } while (choice != 0);

    logout();
}

void adminMenu(string username) {
    int choice;
    do {
        cout << "\n=== Admin Menu ===" << endl;
        cout << "1. Add Book" << endl;
        cout << "2. Edit Book" << endl;
        cout << "3. Delete Book" << endl;
        cout << "4. Search Book" << endl;
        cout << "5. Sort Books" << endl;
        cout << "6. Display Books" << endl;
        cout << "7. Generate Reports" << endl;
        cout << "0. Logout" << endl << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: addBook(); break;
            case 2: editBook(); break;
            case 3: deleteBook(); break;
            case 4: searchBook(username, true); break;
            case 5: sortBooks(); break;
            case 6: displayBooks(); break;
            case 7: break;  // Tsui Hern's part
            case 0: break;
            default: cout << "\nInvalid choice. Please try again." << endl;
        }
    } while (choice != 0);

    logout();
}

// -------------------------------------------------------
// Main
// -------------------------------------------------------
int main() {
    int choice;
    do {
        cout << "=================================" << endl;
        cout << "=  Library Book Records System  =" << endl;
        cout << "=================================" << endl;
        cout << "1. Customer Login" << endl;
        cout << "2. Customer Register" << endl;
        cout << "3. Admin Login" << endl;
        cout << "0. Exit" << endl << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                try {
                    string loggedIn = loginUser();
                    if (loggedIn.empty()) throw "Login failed!";
                    customerMenu(loggedIn);
                } catch (const char* e) {
                    cout << "Error: " << e << endl << endl;
                }
                break;
            }
            case 2: {
                registerUser();
                break;
            }
            case 3: {
                try {
                    string loggedIn = loginAdmin();
                    if (loggedIn.empty()) throw "Login failed!";
                    adminMenu(loggedIn);
                } catch (const char* e) {
                    cout << "Error: " << e << endl << endl;
                }
                break;
            }
            case 0: {
                logout();
                break;
            }
            default: {
                cout << "Invalid choice. Please try again." << endl;
                break;
            }
        }
    } while (choice != 0);

    return 0;
}

// -------------------------------------------------------
// Add / Display / Edit / Delete Book (Yvonne's part)
// -------------------------------------------------------
void addBook() {
    cout << "\n== Add New Book ==" << endl;
    Book newBook;

    cout << "Enter Book ID: ";     cin >> newBook.bookID;
    cin.ignore();
    cout << "Enter Title: ";       getline(cin, newBook.title);
    cout << "Enter Author: ";      getline(cin, newBook.author);
    cout << "Enter Category: ";    getline(cin, newBook.category);
    cout << "Enter Publication Year: "; cin >> newBook.year;
    cout << "Enter Stock Quantity: ";   cin >> newBook.stock;

    newBook.status = (newBook.stock > 0) ? "Available" : "Borrowed";

    ofstream file("Books.txt", ios::app);
    file << newBook.bookID   << "|" << newBook.title  << "|"
         << newBook.author   << "|" << newBook.category << "|"
         << newBook.year     << "|" << newBook.stock   << "|"
         << newBook.status   << endl;
    file.close();

    cout << "Book successfully added!" << endl;
}

void displayBooks() {
    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books found in the library." << endl;
        return;
    }

    cout << "\n========================================= Book List =========================================" << endl;
    cout << left << setw(10) << "ID"
                 << setw(25) << "Title"
                 << setw(20) << "Author"
                 << setw(15) << "Category"
                 << setw(8)  << "Year"
                 << setw(8)  << "Stock"
                 << "Status" << endl;
    cout << "---------------------------------------------------------------------------------------------" << endl;

    for (int i = 0; i < size; i++) {
        cout << left << setw(10) << bookList[i].bookID
             << setw(25) << bookList[i].title.substr(0, 23)
             << setw(20) << bookList[i].author.substr(0, 18)
             << setw(15) << bookList[i].category.substr(0, 13)
             << setw(8)  << bookList[i].year
             << setw(8)  << bookList[i].stock
             << bookList[i].status << endl;
    }
    cout << "=============================================================================================" << endl;

    delete[] bookList;
}

void editBook() {
    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0) {
        cout << "No books available to edit." << endl;
        return;
    }

    string id;
    cout << "\nEnter Book ID to edit: ";
    cin >> id;

    bool found = false;
    for (int i = 0; i < size; i++) {
        if (bookList[i].bookID == id) {
            found = true;
            cout << "\nBook Found! Current Title: " << bookList[i].title << endl;
            cin.ignore();

            cout << "Enter New Title (or press Enter to keep current): ";
            string temp;
            getline(cin, temp);
            if (!temp.empty()) bookList[i].title = temp;

            cout << "Enter New Author (or press Enter to keep current): ";
            getline(cin, temp);
            if (!temp.empty()) bookList[i].author = temp;

            cout << "Enter New Category (or press Enter to keep current): ";
            getline(cin, temp);
            if (!temp.empty()) bookList[i].category = temp;

            cout << "Enter New Stock Quantity (Enter -1 to keep current): ";
            int newStock;
            cin >> newStock;
            if (newStock >= 0) {
                bookList[i].stock = newStock;
                bookList[i].status = (newStock > 0) ? "Available" : "Borrowed";
            }
            break;
        }
    }

    if (found) {
        saveBooks(bookList, size);
        cout << "Book details updated successfully!" << endl;
    } else {
        cout << "Book ID not found." << endl;
    }

    delete[] bookList;
}

void deleteBook() {
    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0) {
        cout << "No books available to delete." << endl;
        return;
    }

    string id;
    cout << "\nEnter Book ID to delete: ";
    cin >> id;

    int deleteIndex = -1;
    for (int i = 0; i < size; i++) {
        if (bookList[i].bookID == id) {
            deleteIndex = i;
            break;
        }
    }

    if (deleteIndex != -1) {
        Book* newBookList = new Book[size - 1];
        int j = 0;
        for (int i = 0; i < size; i++) {
            if (i == deleteIndex) continue;
            newBookList[j++] = bookList[i];
        }
        saveBooks(newBookList, size - 1);
        delete[] newBookList;
        cout << "Book deleted successfully!" << endl;
    } else {
        cout << "Book ID not found." << endl;
    }

    delete[] bookList;
}

// =======================================================
// PART D - Search and Sort  (Zhong Bao's part)
// =======================================================

// -------------------------------------------------------
// LINEAR SEARCH
// Chapter 6.2 - Sequential Search
// Goes through every book one by one
// Returns index if found, -1 if not found
// Used for: search by title keyword, search by author, search by category
// -------------------------------------------------------
int linearSearch(Book* bookList, int size, string keyword, int field) {
    // field 0 = title, 1 = author, 2 = category

    string kw = keyword;
    for (int i = 0; i < (int)kw.size(); i++)
        kw[i] = tolower(kw[i]);

    for (int i = 0; i < size; i++) {
        string target = "";

        if (field == 0)      target = bookList[i].title;
        else if (field == 1) target = bookList[i].author;
        else if (field == 2) target = bookList[i].category;

        // convert to lowercase before comparing
        for (int j = 0; j < (int)target.size(); j++)
            target[j] = tolower(target[j]);

        if (target.find(kw) != string::npos)
            return i;   // found first match
    }

    return -1;
}

// -------------------------------------------------------
// BINARY SEARCH
// Chapter 6.3 - Divide and Conquer
// List MUST be sorted by bookID before calling this
// Searches by exact Book ID
// -------------------------------------------------------
int binarySearch(Book* bookList, int size, string targetID) {

    int first = 0;
    int last  = size - 1;

    while (first <= last) {

        int mid = (first + last) / 2;   // find the middle

        if (bookList[mid].bookID == targetID) {
            return mid;   // found

        } else if (bookList[mid].bookID < targetID) {
            first = mid + 1;   // target is in the right half

        } else {
            last = mid - 1;    // target is in the left half
        }
    }

    return -1;   // not found
}

// -------------------------------------------------------
// BUBBLE SORT
// Chapter 8.4 - Sort by Title A to Z
// Compares neighbours, swaps if out of order
// Stops early if no swap happened in a full pass
// -------------------------------------------------------
void bubbleSort(Book* bookList, int size) {

    for (int i = 0; i < size - 1; i++) {
        bool swapped = false;

        for (int j = 0; j < size - 1 - i; j++) {
            if (bookList[j].title > bookList[j + 1].title) {
                Book temp       = bookList[j];
                bookList[j]     = bookList[j + 1];
                bookList[j + 1] = temp;
                swapped = true;
            }
        }

        // already sorted, no need to continue
        if (!swapped) break;
    }
}

// -------------------------------------------------------
// SELECTION SORT
// Chapter 8.3 - Sort by Book ID (number order)
// Find the smallest ID from remaining, swap to front
// -------------------------------------------------------
void selectionSort(Book* bookList, int size) {

    for (int i = 0; i < size - 1; i++) {

        int minIndex = i;

        for (int j = i + 1; j < size; j++) {
            if (bookList[j].bookID < bookList[minIndex].bookID)
                minIndex = j;
        }

        if (minIndex != i) {
            Book temp          = bookList[i];
            bookList[i]        = bookList[minIndex];
            bookList[minIndex] = temp;
        }
    }
}

// -------------------------------------------------------
// INSERTION SORT
// Chapter 8.2 - Sort by Author A to Z
// Pick one card at a time and insert it in the right place
// -------------------------------------------------------
void insertionSort(Book* bookList, int size) {

    for (int i = 1; i < size; i++) {
        Book current = bookList[i];
        int j = i - 1;

        // shift books with author name bigger than current to the right
        while (j >= 0 && bookList[j].author > current.author) {
            bookList[j + 1] = bookList[j];
            j--;
        }

        // put current book in its correct position
        bookList[j + 1] = current;
    }
}

// -------------------------------------------------------
// Helper: print search result table
// Used by searchBook to keep the display code clean
// -------------------------------------------------------
void printSearchTable(Book* bookList, int size, string kw, int field, bool isAdmin) {

    bool anyFound = false;

    // lowercase the keyword
    string kwLow = kw;
    for (int i = 0; i < (int)kwLow.size(); i++)
        kwLow[i] = tolower(kwLow[i]);

    cout << left
         << setw(8)  << "ID"
         << setw(38) << "Title"
         << setw(22) << "Author"
         << setw(14) << "Category";

    if (isAdmin) cout << setw(8) << "Stock";
    cout << "Status" << endl;
    cout << string(isAdmin ? 98 : 90, '-') << endl;

    for (int i = 0; i < size; i++) {
        string target = "";
        if (field == 0)      target = bookList[i].title;
        else if (field == 1) target = bookList[i].author;
        else if (field == 2) target = bookList[i].category;

        string targetLow = target;
        for (int j = 0; j < (int)targetLow.size(); j++)
            targetLow[j] = tolower(targetLow[j]);

        if (targetLow.find(kwLow) != string::npos) {
            anyFound = true;

            string title  = bookList[i].title;
            string author = bookList[i].author;
            if ((int)title.size()  > 36) title  = title.substr(0, 33)  + "...";
            if ((int)author.size() > 20) author = author.substr(0, 17) + "...";

            cout << left
                 << setw(8)  << bookList[i].bookID
                 << setw(38) << title
                 << setw(22) << author
                 << setw(14) << bookList[i].category;

            if (isAdmin) cout << setw(8) << bookList[i].stock;
            cout << bookList[i].status << endl;
        }
    }

    if (!anyFound)
        cout << "  No books found matching \"" << kw << "\"." << endl;

    cout << endl;
}

// -------------------------------------------------------
// SEARCH BOOK MENU
// Customer: search by Title, Author, Category (no stock shown)
// Admin:    same options + shows stock count
//
// Method used:
//   Title / Author / Category -> Linear Search (sequential scan)
//   Book ID                   -> Binary Search (needs sorted list first)
// -------------------------------------------------------
void searchBook(string username, bool isAdmin) {

    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books in the library right now." << endl;
        return;
    }

    cout << "\n== Search Book ==" << endl;
    cout << "1. Search by Title    (Linear Search)" << endl;
    cout << "2. Search by Author   (Linear Search)" << endl;
    cout << "3. Search by Category (Linear Search)" << endl;
    cout << "4. Search by Book ID  (Binary Search)" << endl;
    cout << "Enter choice: ";

    int searchChoice;
    cin >> searchChoice;
    cin.ignore();

    if (searchChoice == 1 || searchChoice == 2 || searchChoice == 3) {

        string fieldName = "";
        int    field     = 0;

        if (searchChoice == 1) { fieldName = "title";    field = 0; }
        if (searchChoice == 2) { fieldName = "author";   field = 1; }
        if (searchChoice == 3) { fieldName = "category"; field = 2; }

        cout << "Enter " << fieldName << " keyword: ";
        string keyword;
        getline(cin, keyword);

        cout << "\n-- Search Results for \"" << keyword << "\" --" << endl;
        printSearchTable(bookList, size, keyword, field, isAdmin);

    } else if (searchChoice == 4) {

        // Binary Search needs sorted list by ID first
        // use selection sort to sort before searching
        selectionSort(bookList, size);

        cout << "Enter Book ID to search: ";
        string targetID;
        cin >> targetID;

        int result = binarySearch(bookList, size, targetID);

        if (result != -1) {
            cout << "\n-- Book Found --" << endl;
            cout << "ID       : " << bookList[result].bookID   << endl;
            cout << "Title    : " << bookList[result].title    << endl;
            cout << "Author   : " << bookList[result].author   << endl;
            cout << "Category : " << bookList[result].category << endl;
            cout << "Year     : " << bookList[result].year     << endl;

            if (isAdmin)
                cout << "Stock    : " << bookList[result].stock << endl;

            cout << "Status   : " << bookList[result].status << endl;
        } else {
            cout << "\nBook ID \"" << targetID << "\" not found." << endl;
        }
        cout << endl;

    } else {
        cout << "\nInvalid choice." << endl;
    }

    delete[] bookList;
}

// -------------------------------------------------------
// SORT BOOKS MENU (Admin only)
// 3 sorting options:
//   1. Bubble Sort   - sort by Title A to Z
//   2. Selection Sort - sort by Book ID
//   3. Insertion Sort - sort by Author A to Z
// Saves sorted result back to Books.txt
// -------------------------------------------------------
void sortBooks() {

    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books to sort." << endl;
        return;
    }

    cout << "\n== Sort Books ==" << endl;
    cout << "1. Sort by Title A to Z   (Bubble Sort)" << endl;
    cout << "2. Sort by Book ID        (Selection Sort)" << endl;
    cout << "3. Sort by Author A to Z  (Insertion Sort)" << endl;
    cout << "Enter choice: ";

    int sortChoice;
    cin >> sortChoice;

    string sortLabel = "";

    if (sortChoice == 1) {
        bubbleSort(bookList, size);
        saveBooks(bookList, size);
        sortLabel = "Title A to Z (Bubble Sort)";

    } else if (sortChoice == 2) {
        selectionSort(bookList, size);
        saveBooks(bookList, size);
        sortLabel = "Book ID (Selection Sort)";

    } else if (sortChoice == 3) {
        insertionSort(bookList, size);
        saveBooks(bookList, size);
        sortLabel = "Author A to Z (Insertion Sort)";

    } else {
        cout << "\nInvalid choice." << endl;
        delete[] bookList;
        return;
    }

    cout << "\nBooks sorted by " << sortLabel << "!" << endl;
    cout << "Showing all " << size << " books after sorting:" << endl << endl;

    cout << left
         << setw(8)  << "ID"
         << setw(38) << "Title"
         << setw(22) << "Author"
         << setw(8)  << "Stock"
         << "Status" << endl;
    cout << string(84, '-') << endl;

    for (int i = 0; i < size; i++) {
        string title  = bookList[i].title;
        string author = bookList[i].author;
        if ((int)title.size()  > 36) title  = title.substr(0, 33)  + "...";
        if ((int)author.size() > 20) author = author.substr(0, 17) + "...";

        cout << left
             << setw(8)  << bookList[i].bookID
             << setw(38) << title
             << setw(22) << author
             << setw(8)  << bookList[i].stock
             << bookList[i].status << endl;
    }
    cout << endl;

    delete[] bookList;
}
