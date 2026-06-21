#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <ctime>
using namespace std;

// get today's date from the computer
string getTodayDate() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", t);
    return string(buf);
}

// make sure each borrow record gets a different ID
// count how many times the same book was borrowed before and add 1
string generateBorrowID(const string& bookYear, const string& bookID) {
    string base = "BR" + bookYear + bookID + "-";
    ifstream f("BorrowRecords.txt");
    int count = 0;
    string line;
    while (getline(f, line)) {
        if (line.find(base) == 0) count++;
    }
    return base + to_string(count + 1);
}

// forward declarations so functions can call each other freely
void searchBook(string username, bool isAdmin);
void sortBooks();
void borrowBook(string currentCustomer);
void returnBook(string currentCustomer);
void generateReport();
void viewReport();
int  CountBooks();

// customer account management (Admin side)
void manageCustomers();
void viewAllCustomers();
void searchCustomer();
void sortCustomers();
void editCustomerPassword();
void deleteCustomer();

// borrowing history and fines
void viewBorrowRecords();
void editBorrowRecord();
void overdueReport();
void myBorrowHistory(string currentCustomer);
void checkMyFines(string currentCustomer);

// customer profile
void myProfile(string currentCustomer);
void changeMyPassword(string currentCustomer);

// book reservations (Queue based)
void reserveBook(string currentCustomer);
void myReservations(string currentCustomer);
void cancelReservation(string currentCustomer);
void manageReservations();
void viewAllReservations();
void processReservation();

// extra admin / customer actions
void bookStatistics();
void renewBook(string currentCustomer);
void forceReturn();
void manageAdmins();
void topBorrowedBooks();
void restoreLastDeleted();
void displayBooksFiltered();

// helpers defined later but needed by the summary report
int  countReservations();
int  overdueDays(string borrowDate, string returnDate, string status);
int  readInt(string prompt);

struct Book{
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

// ADT Queue - used in borrow and return book
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
            string item = "";
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
// Linked Stack - keeps the books an admin deletes so the
// most recent deletion can be undone (LIFO).
// -------------------------------------------------------
struct StackNode {
    Book data;
    StackNode* next;
};

class BookStack {
    private:
        StackNode* topPtr;

    public:
        BookStack() { topPtr = nullptr; }

        ~BookStack() {
            while (topPtr != nullptr) {
                StackNode* temp = topPtr;
                topPtr = topPtr->next;
                delete temp;
            }
        }

        bool isEmpty() { return topPtr == nullptr; }

        void push(Book b) {
            StackNode* node = new StackNode;
            node->data = b;
            node->next = topPtr;
            topPtr = node;
        }

        Book pop() {
            Book b;
            if (!isEmpty()) {
                StackNode* temp = topPtr;
                b = temp->data;
                topPtr = topPtr->next;
                delete temp;
            }
            return b;
        }

        Book peek() {
            Book b;
            if (!isEmpty()) b = topPtr->data;
            return b;
        }
};

// holds books removed during the current admin session
BookStack deletedBooks;

// -------------------------------------------------------
// Account base class and subclasses
// -------------------------------------------------------
class Account
{
	protected:
		string username;
		string password;
		string role;

	public:
		Account()
		{
			username = "";
			password = "";
			role = "";
		}

		~Account() {}

		friend void displayAccount(const Account& a);
		friend void displayGuest(const Account& a);
		friend class Admin;

		string getUsername() { return username; }
		string getPassword() { return password; }
		string getRole() { return role; }
};

class User : public Account
{
	public:
		User() { role = "Customer"; }

		User(string u, string p)
		{
			username = u;
			password = p;
			role = "Customer";
		}

		~User() {}
		friend void displayUser(const User& u);

		void setUsername(string u) { username = u; }
		void setPassword(string p) { password = p; }
};

class Admin : public Account
{
	public:
		Admin() { role = "Admin"; }

		Admin(string u, string p)
		{
			username = u;
			password = p;
			role = "Admin";
		}

		~Admin() {}
		friend void displayAdmin(const Admin& a);

		void setUsername(string u) { username = u; }
		void setPassword(string p) { password = p; }
};

// guest is a visitor that has not login yet (used in Help / About)
class Guest : public Account
{
	public:
		Guest()
		{
			username = "Visitor";
			role = "Guest";
		}

		~Guest() {}
};

// friend functions to print the welcome card after login
// they can read the protected username and role
void displayAccount(const Account& a)
{
	cout << "Account : " << a.username << " (" << a.role << ")" << endl;
}

void displayUser(const User& u)
{
	cout << "--------------------------------" << endl;
	cout << " Welcome back, customer " << u.username << endl;
	cout << " Access level: " << u.role << endl;
	cout << "--------------------------------" << endl;
}

void displayAdmin(const Admin& a)
{
	cout << "--------------------------------" << endl;
	cout << " Welcome back, admin " << a.username << endl;
	cout << " Access level: " << a.role << endl;
	cout << "--------------------------------" << endl;
}

void displayGuest(const Account& a)
{
	cout << "--------------------------------" << endl;
	cout << " Hello " << a.username << "!" << endl;
	cout << " Access level: " << a.role << " (please login for more)" << endl;
	cout << "--------------------------------" << endl;
}

// =========================
// Register / Login / Logout
// =========================
void registerUser()
{
	string u, p;
	string fu, fp, frole;

	cout << "\n== Register New Customer ==" << endl;
	cout << "Enter username: ";
	cin >> u;
	cout << "Enter password: ";
	cin >> p;
	cout << endl;

	// check first so the same username is not used twice
	ifstream checkFile("Customer.txt");
	while(getline(checkFile, fu, '|') && getline(checkFile, fp, '|') && getline(checkFile, frole))
	{
		if(u == fu)
		{
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

string loginUser()
{
	string u, p, fu, fp, frole;

	cout << "\n== Customer Login ==" << endl;
	cout << "Enter username: ";
	cin >> u;
	cout << "Enter password: ";
	cin >> p;
	cout << endl;

	ifstream file("Customer.txt");
	while(getline(file, fu, '|') && getline(file, fp, '|') && getline(file, frole))
	{
		if(u == fu && p == fp)
		{
			file.close();
			User loggedIn(u, p);
			displayUser(loggedIn); // friend function welcome card
			return u;
		}
	}
	file.close();
	cout << "Invalid username or password." << endl;
	return "";
}

string loginAdmin()
{
	string u, p, fu, fp, frole;

	cout << "\n== Admin Login ==" << endl;
	cout << "Enter username: ";
	cin >> u;
	cout << "Enter password: ";
	cin >> p;
	cout << endl;

	ifstream file("Admin.txt");
	while(getline(file, fu, '|') && getline(file, fp, '|') && getline(file, frole))
	{
		if(u == fu && p == fp)
		{
			file.close();
			Admin loggedIn(u, p);
			displayAdmin(loggedIn); // friend function welcome card
			return u;
		}
	}
	file.close();
	cout << "Invalid username or password." << endl;
	return "";
}

void logout()
{
	cout << "\nLogging out... Goodbye!" << endl << endl;
}

// -------------------------------------------------------
// File helpers - count, load, save books
// -------------------------------------------------------
int CountBooks(){
    ifstream file("Books.txt");
    if(!file.is_open())
		return 0;
    int count = 0;
    string line;
    while(getline(file, line)){
        if(!line.empty())
			count++;
    }
    file.close();
    return count;
}

Book* LoadBooks(int& size){
    size = CountBooks();
    if(size == 0) return nullptr;

    Book* bookList = new Book[size];
    ifstream file("Books.txt");
    string tempYear, tempStock;

    for(int i = 0; i < size; i++){
        getline(file, bookList[i].bookID, '|');
        getline(file, bookList[i].title, '|');
        getline(file, bookList[i].author, '|');
        getline(file, bookList[i].category, '|');
        getline(file, tempYear, '|');
        getline(file, tempStock, '|');
        getline(file, bookList[i].status);

        // some books may have a non-numeric year like "Unknown"
        try{
            if(!tempYear.empty() && tempYear != "Unknown")
                bookList[i].year = stoi(tempYear);
            else
                bookList[i].year = 0;
        } catch (...){
            bookList[i].year = 0;
        }

        try{
            if(!tempStock.empty())
                bookList[i].stock = stoi(tempStock);
            else
                bookList[i].stock = 0;
        } catch (...){
            bookList[i].stock = 0;
        }
    }
    file.close();
    return bookList;
}

void SaveBooks(Book* bookList, int size){
    ofstream file("Books.txt", ios::trunc);
	
    for(int i = 0; i < size; i++){
        file <<bookList[i].bookID<<"|"
             <<bookList[i].title<<"|"
             <<bookList[i].author<<"|"
             <<bookList[i].category<<"|"
             <<bookList[i].year<<"|"
             <<bookList[i].stock<<"|"
             <<bookList[i].status<<endl;
    }
    file.close();
}

// -------------------------------------------------------
// Borrow Book (Wen Zhe's part)
// -------------------------------------------------------
void borrowBook(string currentCustomer) {
    cin.ignore(10000, '\n');
    cout << "\n== Borrow Book ==" << endl;

    int size = 0;
    Book* bookList = LoadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "No books available in the library." << endl;
        return;
    }

    string id;
    cout << "Enter Book ID to borrow (or 0 to cancel): ";
    cin >> id;

    if (id == "0") {
        cout << "Borrow cancelled." << endl;
        delete[] bookList;
        return;
    }

    bool found = false;
    for (int i = 0; i < size; i++) {
        if (bookList[i].bookID == id) {
            found = true;

            if (bookList[i].stock > 0) {
                bookList[i].stock--;

                if (bookList[i].stock == 0)
                    bookList[i].status = "Borrowed";

                string uniqueID   = generateBorrowID(to_string(bookList[i].year), id);
                string borrowDate = getTodayDate();

                ofstream recordFile("BorrowRecords.txt", ios::app);
                recordFile << uniqueID << "|"
                           << currentCustomer << "|"
                           << bookList[i].bookID << "|"
                           << borrowDate << "|---|Borrowed" << endl;
                recordFile.close();

                cout << "Book '" << bookList[i].title
                     << "' borrowed successfully by " << currentCustomer << "!" << endl;
            } else {
                cout << "Sorry, this book is out of stock!" << endl;
            }
            break;
        }
    }

    if (found) {
        SaveBooks(bookList, size);
    } else {
        cout << "Book ID not found." << endl;
    }

    delete[] bookList;
}

// -------------------------------------------------------
// Return Book (Wen Zhe's part)
// -------------------------------------------------------
void returnBook(string currentCustomer) {
    cin.ignore(10000, '\n');
    cout << "\n=== Return Book ===" << endl;

    string id;
    cout << "Enter Book ID to return (or 0 to cancel): ";
    cin >> id;

    if (id == "0") {
        cout << "Return cancelled." << endl;
        return;
    }

    int size = 0;
    Book* bookList = LoadBooks(size);
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

    while (getline(readFile, bID, '|') && getline(readFile, custID, '|') &&
           getline(readFile, bkID, '|') && getline(readFile, bDate, '|') &&
           getline(readFile, rDate, '|') && getline(readFile, bStatus)) {

        if (custID == currentCustomer && bkID == id && bStatus == "Borrowed" && !recordUpdated) {
            string returnDate = getTodayDate();
            tempFile << bID << "|" << custID << "|" << bkID << "|"
                     << bDate << "|" << returnDate << "|Returned" << endl;
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
        SaveBooks(bookList, size);
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
        SaveBooks(bookList, size);
    }

    delete[] bookList;
}


// -------------------------------------------------------
// Report Generation and TXT File Management (Tsui Hern's part)
// -------------------------------------------------------
void generateReport() {
    try {
        int totalBooks = CountBooks();
        int availableBooks = 0;
        int borrowedBooksByStock = 0;
        int totalCustomers = 0;
        int totalBorrowRecords = 0;
        int activeBorrowRecords = 0;
        int returnedRecords = 0;

        int size = 0;
        Book* bookList = LoadBooks(size);

        for (int i = 0; i < size; i++) {
            if (bookList[i].stock > 0) {
                availableBooks++;
            } else {
                borrowedBooksByStock++;
            }
        }

        if (bookList != nullptr) {
            delete[] bookList;
        }

        ifstream customerFile("Customer.txt");
        if (customerFile.is_open()) {
            string line;
            while (getline(customerFile, line)) {
                if (line != "") {
                    totalCustomers++;
                }
            }
            customerFile.close();
        }

        int overdueCount = 0;

        ifstream borrowFile("BorrowRecords.txt");
        if (borrowFile.is_open()) {
            string bID, custID, bkID, bDate, rDate, bStatus;
            while (getline(borrowFile, bID, '|') && getline(borrowFile, custID, '|') &&
                   getline(borrowFile, bkID, '|') && getline(borrowFile, bDate, '|') &&
                   getline(borrowFile, rDate, '|') && getline(borrowFile, bStatus)) {
                totalBorrowRecords++;
                if (bStatus == "Borrowed") {
                    activeBorrowRecords++;
                } else if (bStatus == "Returned") {
                    returnedRecords++;
                }
                if (overdueDays(bDate, rDate, bStatus) > 0)
                    overdueCount++;
            }
            borrowFile.close();
        }

        int totalReservations = countReservations();

        ofstream reportFile("Report.txt");
        if (!reportFile.is_open()) {
            throw "Unable to create Report.txt";
        }

        reportFile << "========== LIBRARY BOOK RECORDS REPORT ==========" << endl;
        reportFile << "Total Books              : " << totalBooks << endl;
        reportFile << "Available Book Titles    : " << availableBooks << endl;
        reportFile << "Unavailable Book Titles  : " << borrowedBooksByStock << endl;
        reportFile << "Total Customers          : " << totalCustomers << endl;
        reportFile << "Total Borrow Records     : " << totalBorrowRecords << endl;
        reportFile << "Active Borrow Records    : " << activeBorrowRecords << endl;
        reportFile << "Returned Records         : " << returnedRecords << endl;
        reportFile << "Overdue Records          : " << overdueCount << endl;
        reportFile << "Total Reservations       : " << totalReservations << endl;
        reportFile << "=================================================" << endl;
        reportFile << "Report generated and saved successfully." << endl;
        reportFile.close();

        cout << "\n========== LIBRARY BOOK RECORDS REPORT ==========" << endl;
        cout << "Total Books              : " << totalBooks << endl;
        cout << "Available Book Titles    : " << availableBooks << endl;
        cout << "Unavailable Book Titles  : " << borrowedBooksByStock << endl;
        cout << "Total Customers          : " << totalCustomers << endl;
        cout << "Total Borrow Records     : " << totalBorrowRecords << endl;
        cout << "Active Borrow Records    : " << activeBorrowRecords << endl;
        cout << "Returned Records         : " << returnedRecords << endl;
        cout << "Overdue Records          : " << overdueCount << endl;
        cout << "Total Reservations       : " << totalReservations << endl;
        cout << "=================================================" << endl;
        cout << "Report saved successfully into Report.txt" << endl;
    }
    catch (const char* msg) {
        cout << "Error: " << msg << endl;
    }
    catch (...) {
        cout << "Error: Report generation failed." << endl;
    }
}

void viewReport() {
    try {
        ifstream reportFile("Report.txt");
        if (!reportFile.is_open()) {
            throw "No report found. Please ask Admin to generate the report first.";
        }

        string line;
        cout << "\n=== Saved Library Report ===" << endl;
        while (getline(reportFile, line)) {
            cout << line << endl;
        }
        reportFile.close();
    }
    catch (const char* msg) {
        cout << "Error: " << msg << endl;
    }
    catch (...) {
        cout << "Error: Unable to read Report.txt" << endl;
    }
}

//Yvonne's part (Add / Display / Edit / Delete Books)
class MBooks{
public:
    void AddBook(){
        cout <<"\n== Add New Book =="<<endl;
        Book newBook;
        
        // Find the largest Book ID already in use and add one,
        // so the system gives the new book a unique ID by itself
        int existSize = 0;
        Book* existing = LoadBooks(existSize);
        int maxID = 0;
        for(int i = 0; i < existSize; i++){
            try{
                int id = stoi(existing[i].bookID);
                if (id > maxID) maxID = id;
            } catch (...){
                // Skip any book whose ID is not a plain number
            }
        }
        if(existing != nullptr) delete[] existing;
        
        newBook.bookID = to_string(maxID + 1);
        cout << "Auto-generated Book ID: " << newBook.bookID << endl;
        
        cin.ignore(10000, '\n');
        cout << "Enter Title: "; getline(cin, newBook.title);
        cout << "Enter Author: "; getline(cin, newBook.author);
        cout << "Enter Category: "; getline(cin, newBook.category);
        newBook.year  = readInt("Enter Publication Year: ");
        newBook.stock = readInt("Enter Stock Quantity: ");
        
        newBook.status = (newBook.stock > 0) ? "Available" : "Borrowed";
        
        ofstream file("Books.txt", ios::app);
        file<<newBook.bookID<<"|"<< newBook.title<<"|"
            <<newBook.author<<"|"<<newBook.category<<"|"
            <<newBook.year<<"|"<<newBook.stock<<"|"
            <<newBook.status<<endl;
        file.close();
        
        // Show the full details of the book that was just added
        cout<< "\nBook successfully added!" << endl;
        cout<< "-----------------------------" << endl;
        cout<< "Book ID  : "<<newBook.bookID << endl;
        cout<< "Title    : "<<newBook.title<<endl;
        cout<< "Author   : "<<newBook.author<<endl;
        cout<< "Category : "<<newBook.category<<endl;
        cout<< "Year     : "<<newBook.year<<endl;
        cout<< "Stock    : "<<newBook.stock<<endl;
        cout<< "Status   : "<<newBook.status<<endl;
        cout<<"-----------------------------"<<endl;
    }

    void EditBook(){
        int size = 0;
        Book* bookList = LoadBooks(size);
        
        if(size == 0){
            cout<<"No books available to edit."<<endl;
            return;
        }
        
        // Show a quick list of ID and title so the admin does not have to guess
        cout<<"\n-- Current Books --"<<endl;
        for(int i = 0; i < size; i++){
            cout<<left << setw(6)<<bookList[i].bookID
                <<bookList[i].title<<endl;
        }
        
        string id;
        cout<<"\nEnter Book ID to edit (0 to cancel): ";
        cin>>id;
        
        if(id == "0"){
            cout<<"Edit cancelled."<<endl;
            delete[] bookList;
            return;
        }
        
        bool found = false;
        for(int i = 0; i < size; i++){
            if(bookList[i].bookID == id){
                found = true;
                cout<<"\nBook Found! Current Title: "<<bookList[i].title<<endl;
                cin.ignore(10000, '\n');
                
                cout<<"Enter New Title (or press Enter to keep current): ";
                string temp;
                getline(cin, temp);
                if (!temp.empty()) bookList[i].title = temp;
                
                cout<<"Enter New Author (or press Enter to keep current): ";
                getline(cin, temp);
                if(!temp.empty()) bookList[i].author = temp;
                
                cout<<"Enter New Category (or press Enter to keep current): ";
                getline(cin, temp);
                if (!temp.empty()) bookList[i].category = temp;
                
                cout<<"Enter New Publication Year (Enter -1 to keep current): ";
                int newYear;
                cin >> newYear;
                if(newYear >= 0) bookList[i].year = newYear;
                
                cout<<"Enter New Stock Quantity (Enter -1 to keep current): ";
                int newStock;
                cin>>newStock;
                
                if(newStock >= 0){
                    bookList[i].stock = newStock;
                    bookList[i].status = (newStock > 0) ? "Available" : "Borrowed";
                }break;
            }
        }
        
        if(found){
            SaveBooks(bookList, size);
            cout<<"Book details updated successfully!"<<endl;
        } else{
            cout<<"Book ID not found."<<endl;
        }
        delete[] bookList;
    }

    void DelBook(){
        int size = 0;
        Book* bookList = LoadBooks(size);
        
        if(size == 0){
            cout<<"No books available to delete."<<endl;
            return;
        }
        
        // Show a quick list of ID and title so the admin does not have to guess
        cout<<"\n-- Current Books --"<<endl;
        for(int i = 0; i < size; i++){
            cout<<left<<setw(6)<<bookList[i].bookID
                << bookList[i].title<<endl;
        }
        
        string id;
        cout<<"\nEnter Book ID to delete (0 to cancel): ";
        cin>>id;
        
        if(id == "0"){
            cout<<"Delete cancelled."<<endl;
            delete[] bookList;
            return;
        }
        
        int DelIndex = -1;
        for(int i = 0; i < size; i++){
            if(bookList[i].bookID == id){
                DelIndex = i;
                break;
            }
        }
        
        if(DelIndex != -1){
            // Keep a copy on the stack so the deletion can be undone later
            deletedBooks.push(bookList[DelIndex]);
            
            Book* newBookList = new Book[size - 1];
            int j = 0;
            for(int i = 0; i < size; i++){
                if(i == DelIndex) continue;
                newBookList[j++] = bookList[i];
            }
            SaveBooks(newBookList, size - 1);
            delete[] newBookList;
            
            cout<<"Book deleted successfully! (Use 'Undo Last Delete' to restore.)"<<endl;
        } else{
            cout<<"Book ID not found."<<endl;
        }
        delete[] bookList;
    }

    void DisBooks(){
        int size = 0;
        Book* bookList = LoadBooks(size);
        
        if(size == 0 || bookList == nullptr){
            cout<<"\nNo books found in the library."<<endl;
            return;
        }
        
        cout<<"\n========================================= Book List ========================================="<<endl;
        cout<<left << setw(10)<<"ID"
            <<setw(25)<<"Title"
            <<setw(20)<<"Author"
            <<setw(15)<<"Category"
            <<setw(8)<<"Year"
            <<setw(8)<<"Stock"
            <<"Status"<<endl;
        cout<<"---------------------------------------------------------------------------------------------"<<endl;
        
        for(int i = 0; i < size; i++){
            cout<<left << setw(10)<<bookList[i].bookID
                <<setw(25)<<bookList[i].title.substr(0, 23)
                <<setw(20)<<bookList[i].author.substr(0, 18)
                <<setw(15)<<bookList[i].category.substr(0, 13)
                <<setw(8)<<bookList[i].year
                <<setw(8)<<bookList[i].stock
                <<bookList[i].status<<endl;
        }
        cout<<"============================================================================================="<<endl;
        
        delete[] bookList;
    }
};

// =========================
// Customer and Admin menus
// =========================
void customerMenu(string username)
{
	int choice;
	do
	{
		cout << "\n=== Customer Menu ===" << endl;
		cout << "1. Borrow Book" << endl;
		cout << "2. Return Book" << endl;
		cout << "3. Renew Book" << endl;
		cout << "4. Reserve Book" << endl;
		cout << "5. My Reservations" << endl;
		cout << "6. Search Book" << endl;
		cout << "7. Display Books" << endl;
		cout << "8. My Borrow History" << endl;
		cout << "9. Check My Fines" << endl;
		cout << "10. My Profile" << endl;
		cout << "11. View Borrowing Summary" << endl;
		cout << "0. Logout" << endl << endl;
		cout << "Enter your choice: ";
		cin >> choice;

		switch(choice)
		{
			case 1: borrowBook(username); break;
			case 2: returnBook(username); break;
			case 3: renewBook(username); break;
			case 4: reserveBook(username); break;
			case 5: myReservations(username); break;
			case 6: searchBook(username, false); break;
			case 7: displayBooksFiltered(); break;
			case 8: myBorrowHistory(username); break;
			case 9: checkMyFines(username); break;
			case 10: myProfile(username); break;
			case 11: viewReport(); break; // Tsui Hern's part
			case 0: break;
			default: cout << "\nInvalid choice. Please try again." << endl;
		}
	} while(choice != 0);

	logout();
}

void adminMenu(string username)
{
	MBooks b;
	int choice;
	do
	{
		cout << "\n=== Admin Menu ===" << endl;
		cout << "1. Add Book" << endl;
		cout << "2. Edit Book" << endl;
		cout << "3. Delete Book" << endl;
		cout << "4. Search Book" << endl;
		cout << "5. Sort Books" << endl;
		cout << "6. Display Books" << endl;
		cout << "7. Manage Customers" << endl;
		cout << "8. View Borrow Records" << endl;
		cout << "9. Force Return Book" << endl;
		cout << "10. Manage Reservations" << endl;
		cout << "11. Overdue Report" << endl;
		cout << "12. Book Statistics" << endl;
		cout << "13. Top Borrowed Books" << endl;
		cout << "14. Manage Admins" << endl;
		cout << "15. Undo Last Delete" << endl;
		cout << "16. Generate Reports" << endl;
		cout << "0. Logout" << endl << endl;
		cout << "Enter your choice: ";
		cin >> choice;

		switch(choice)
		{
			case 1: b.AddBook(); break;
			case 2: b.EditBook(); break;
			case 3: b.DelBook(); break;
			case 4: searchBook(username, true); break;
			case 5: sortBooks(); break;
			case 6: b.DisBooks(); break;
			case 7: manageCustomers(); break;
			case 8: viewBorrowRecords(); break;
			case 9: forceReturn(); break;
			case 10: manageReservations(); break;
			case 11: overdueReport(); break;
			case 12: bookStatistics(); break;
			case 13: topBorrowedBooks(); break;
			case 14: manageAdmins(); break;
			case 15: restoreLastDeleted(); break;
			case 16: generateReport(); break; // Tsui Hern's part
			case 0: break;
			default: cout << "\nInvalid choice. Please try again." << endl;
		}
	} while(choice != 0);

	logout();
}

// help / about screen, can be opened from the main menu
void showHelp()
{
	Guest visitor;
	displayGuest(visitor); // greet the visitor that is not login yet

	cout << "\n=============== HELP / ABOUT ===============" << endl;
	cout << "This is a library book records system." << endl;
	cout << endl;
	cout << "Customer accounts can:" << endl;
	cout << " - borrow, renew, return and reserve books" << endl;
	cout << " - search and browse the book list" << endl;
	cout << " - view their own history, fines and profile" << endl;
	cout << endl;
	cout << "Admin accounts can:" << endl;
	cout << " - add, edit, delete and sort books" << endl;
	cout << " - manage customer and admin accounts" << endl;
	cout << " - handle borrow records and reservations" << endl;
	cout << " - generate reports and check overdue fines" << endl;
	cout << endl;
	cout << "All data is stored in plain text files so it stays" << endl;
	cout << "consistent between the two modules." << endl;
	cout << "===========================================" << endl;
}

// banner that shows one time when the program starts
void showWelcomeBanner()
{
	cout << "*********************************************" << endl;
	cout << "*      Group 8 - Library Book Records       *" << endl;
	cout << "*   Data Structure and Algorithms Project   *" << endl;
	cout << "*********************************************" << endl;
	cout << endl;
}

// =========================
// Main
// =========================
int main()
{
	int choice;
	showWelcomeBanner();

	do
	{
		cout << "=================================" << endl;
		cout << "=  Library Book Records System  =" << endl;
		cout << "=================================" << endl;
		cout << "1. Customer Login" << endl;
		cout << "2. Customer Register" << endl;
		cout << "3. Admin Login" << endl;
		cout << "4. Help / About" << endl;
		cout << "0. Exit" << endl << endl;
		cout << "Enter choice: ";
		cin >> choice;

		if(choice == 1)
		{
			// try catch so a failed login does not crash the program
			try
			{
				string loggedIn = loginUser();
				if(loggedIn.empty()) throw "Login failed!";
				customerMenu(loggedIn);
			}
			catch(const char* e)
			{
				cout << "Error: " << e << endl << endl;
			}
		}
		else if(choice == 2)
		{
			registerUser();
		}
		else if(choice == 3)
		{
			try
			{
				string loggedIn = loginAdmin();
				if(loggedIn.empty()) throw "Login failed!";
				adminMenu(loggedIn);
			}
			catch(const char* e)
			{
				cout << "Error: " << e << endl << endl;
			}
		}
		else if(choice == 4)
		{
			showHelp();
		}
		else if(choice == 0)
		{
			logout();
		}
		else
		{
			cout << "Invalid choice. Please try again." << endl;
		}
	} while(choice != 0);

	cout << "*********************************************" << endl;
	cout << "*  Thank you for using the Library System  *" << endl;
	cout << "*               Goodbye!                    *" << endl;
	cout << "*********************************************" << endl;
	return 0;
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
    Book* bookList = LoadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books in the library right now." << endl;
        return;
    }

    cout << "\n== Search Book ==" << endl;
    cout << "1. Search by Title    (Linear Search)" << endl;
    cout << "2. Search by Author   (Linear Search)" << endl;
    cout << "3. Search by Category (Linear Search)" << endl;
    cout << "4. Search by Book ID  (Binary Search)" << endl;
    cout << "5. Search by Year Range" << endl;
    cout << "Enter choice: ";

    int searchChoice;
    cin >> searchChoice;
    cin.ignore(10000, '\n');

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

    } else if (searchChoice == 5) {

        int fromYear, toYear;
        cout << "Enter start year: ";
        cin >> fromYear;
        cout << "Enter end year: ";
        cin >> toYear;

        if (fromYear > toYear) {
            int t = fromYear; fromYear = toYear; toYear = t;
        }

        cout << "\n-- Books published between " << fromYear
             << " and " << toYear << " --" << endl;
        cout << left << setw(10) << "ID" << setw(30) << "Title"
             << setw(8) << "Year" << "Status" << endl;
        cout << string(60, '-') << endl;

        int found = 0;
        for (int i = 0; i < size; i++) {
            if (bookList[i].year >= fromYear && bookList[i].year <= toYear) {
                string title = bookList[i].title;
                if ((int)title.size() > 28) title = title.substr(0, 25) + "...";
                cout << left << setw(10) << bookList[i].bookID
                     << setw(30) << title
                     << setw(8)  << bookList[i].year
                     << bookList[i].status << endl;
                found++;
            }
        }

        if (found == 0)
            cout << "No books found in that year range." << endl;
        else
            cout << "\nFound " << found << " book(s)." << endl;

    } else {
        cout << "\nInvalid choice." << endl;
    }

    delete[] bookList;
}

// -------------------------------------------------------
// MERGE SORT
// Chapter 8.5 - Divide the list, sort each half, then merge
// Sorts the books by publication year (oldest first)
// -------------------------------------------------------
void merge(Book* bookList, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Book* leftArr  = new Book[n1];
    Book* rightArr = new Book[n2];

    for (int i = 0; i < n1; i++) leftArr[i]  = bookList[left + i];
    for (int j = 0; j < n2; j++) rightArr[j] = bookList[mid + 1 + j];

    int i = 0, j = 0, k = left;

    // take the smaller year each time and place it back into the list
    while (i < n1 && j < n2) {
        if (leftArr[i].year <= rightArr[j].year)
            bookList[k++] = leftArr[i++];
        else
            bookList[k++] = rightArr[j++];
    }

    while (i < n1) bookList[k++] = leftArr[i++];
    while (j < n2) bookList[k++] = rightArr[j++];

    delete[] leftArr;
    delete[] rightArr;
}

void mergeSort(Book* bookList, int left, int right) {
    if (left < right) {
        int mid = (left + right) / 2;
        mergeSort(bookList, left, mid);
        mergeSort(bookList, mid + 1, right);
        merge(bookList, left, mid, right);
    }
}

// -------------------------------------------------------
// QUICK SORT
// Chapter 8.6 - Pick a pivot, push smaller items to its left
// Sorts the books by stock quantity (most copies first)
// -------------------------------------------------------
int partition(Book* bookList, int low, int high) {
    int pivot = bookList[high].stock;
    int i = low - 1;

    for (int j = low; j < high; j++) {
        // bigger stock goes to the left so the list ends up high to low
        if (bookList[j].stock > pivot) {
            i++;
            Book temp     = bookList[i];
            bookList[i]   = bookList[j];
            bookList[j]   = temp;
        }
    }

    Book temp          = bookList[i + 1];
    bookList[i + 1]    = bookList[high];
    bookList[high]     = temp;
    return i + 1;
}

void quickSort(Book* bookList, int low, int high) {
    if (low < high) {
        int pivotIndex = partition(bookList, low, high);
        quickSort(bookList, low, pivotIndex - 1);
        quickSort(bookList, pivotIndex + 1, high);
    }
}

// -------------------------------------------------------
// SORT BOOKS MENU (Admin only)
// Sorting options, each saved back to Books.txt:
//   1. Bubble Sort    - Title A to Z
//   2. Selection Sort - Book ID
//   3. Insertion Sort - Author A to Z
//   4. Merge Sort     - Publication year
//   5. Quick Sort     - Stock quantity
// -------------------------------------------------------
void sortBooks() {

    int size = 0;
    Book* bookList = LoadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books to sort." << endl;
        return;
    }

    cout << "\n== Sort Books ==" << endl;
    cout << "1. Sort by Title A to Z   (Bubble Sort)" << endl;
    cout << "2. Sort by Book ID        (Selection Sort)" << endl;
    cout << "3. Sort by Author A to Z  (Insertion Sort)" << endl;
    cout << "4. Sort by Year           (Merge Sort)" << endl;
    cout << "5. Sort by Stock          (Quick Sort)" << endl;
    cout << "Enter choice: ";

    int sortChoice;
    cin >> sortChoice;

    string sortLabel = "";

    if (sortChoice == 1) {
        bubbleSort(bookList, size);
        SaveBooks(bookList, size);
        sortLabel = "Title A to Z (Bubble Sort)";

    } else if (sortChoice == 2) {
        selectionSort(bookList, size);
        SaveBooks(bookList, size);
        sortLabel = "Book ID (Selection Sort)";

    } else if (sortChoice == 3) {
        insertionSort(bookList, size);
        SaveBooks(bookList, size);
        sortLabel = "Author A to Z (Insertion Sort)";

    } else if (sortChoice == 4) {
        mergeSort(bookList, 0, size - 1);
        SaveBooks(bookList, size);
        sortLabel = "Year oldest first (Merge Sort)";

    } else if (sortChoice == 5) {
        quickSort(bookList, 0, size - 1);
        SaveBooks(bookList, size);
        sortLabel = "Stock most first (Quick Sort)";

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

// =======================================================
// PART E - Customer Accounts, Borrowing History and Fines
// =======================================================

// loan rules used when calculating fines
const string SYSTEM_TODAY = "2026-06-15";   // current date the system runs on
const int    LOAN_DAYS    = 14;             // a book is due 14 days after borrowing
const double FINE_PER_DAY = 0.50;           // RM charged for each late day

// one customer account loaded from Customer.txt
struct CustomerAcc {
    string username;
    string password;
    string role;
};

// turn any text into lowercase so searches are not case sensitive
string toLowerStr(string text) {
    for (int i = 0; i < (int)text.size(); i++)
        text[i] = tolower(text[i]);
    return text;
}

// remove whatever is still sitting in the input buffer before a getline
void clearInputBuffer() {
    cin.clear();
    cin.ignore(10000, '\n');
}

// ask a simple yes / no question, returns true only on y or yes
bool askYesNo(string question) {
    string answer;
    cout << question << " (y/n): ";
    cin >> answer;
    answer = toLowerStr(answer);
    return (answer == "y" || answer == "yes");
}

// read a whole number and keep asking until the user types a valid one
int readInt(string prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            return value;
        }
        cout << "Please enter a valid number." << endl;
        cin.clear();
        cin.ignore(10000, '\n');
    }
}

// convert a YYYY-MM-DD date into a running day number so two dates can be
// compared and subtracted. Returns 0 for blank dates like "---".
int dateToDays(string date) {
    if (date.size() < 10) return 0;

    try {
        int y = stoi(date.substr(0, 4));
        int m = stoi(date.substr(5, 2));
        int d = stoi(date.substr(8, 2));

        int monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int days = y * 365 + d;
        for (int i = 0; i < m - 1; i++)
            days += monthDays[i];

        days += y / 4;   // rough leap year adjustment, accurate enough for fines
        return days;
    } catch (...) {
        return 0;
    }
}

// work out how many days a record is overdue. A returned book is compared
// against its real return date, a book still on loan against today.
int overdueDays(string borrowDate, string returnDate, string status) {
    int due = dateToDays(borrowDate) + LOAN_DAYS;
    int checkDay;

    if (status == "Returned")
        checkDay = dateToDays(returnDate);
    else
        checkDay = dateToDays(SYSTEM_TODAY);

    int late = checkDay - due;
    if (late < 0) late = 0;
    return late;
}

// look up a book title by its ID so reports read nicely instead of showing IDs
string getBookTitle(string id) {
    int size = 0;
    Book* bookList = LoadBooks(size);
    string title = "(unknown title)";

    for (int i = 0; i < size; i++) {
        if (bookList[i].bookID == id) {
            title = bookList[i].title;
            break;
        }
    }

    if (bookList != nullptr) delete[] bookList;
    return title;
}

// -------------------------------------------------------
// Customer account file helpers (dynamic array, new / delete)
// -------------------------------------------------------
int countCustomers() {
    ifstream file("Customer.txt");
    if (!file.is_open()) return 0;

    int count = 0;
    string line;
    while (getline(file, line)) {
        if (!line.empty()) count++;
    }
    file.close();
    return count;
}

CustomerAcc* loadCustomers(int& size) {
    size = countCustomers();
    if (size == 0) return nullptr;

    CustomerAcc* list = new CustomerAcc[size];
    ifstream file("Customer.txt");

    for (int i = 0; i < size; i++) {
        getline(file, list[i].username, '|');
        getline(file, list[i].password, '|');
        getline(file, list[i].role);
    }
    file.close();
    return list;
}

void saveCustomers(CustomerAcc* list, int size) {
    ofstream file("Customer.txt", ios::trunc);
    for (int i = 0; i < size; i++) {
        file << list[i].username << "|"
             << list[i].password << "|"
             << list[i].role     << endl;
    }
    file.close();
}

// -------------------------------------------------------
// Manage Customers - Admin only sub menu
// View, search, sort, reset password or delete a customer.
// -------------------------------------------------------
void viewAllCustomers() {
    int size = 0;
    CustomerAcc* list = loadCustomers(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo customers registered yet." << endl;
        return;
    }

    cout << "\n===== Registered Customers =====" << endl;
    cout << left << setw(6) << "No" << setw(20) << "Username" << "Role" << endl;
    cout << "-----------------------------------" << endl;

    for (int i = 0; i < size; i++) {
        cout << left << setw(6) << (i + 1)
             << setw(20) << list[i].username
             << list[i].role << endl;
    }
    cout << "Total customers: " << size << endl;

    delete[] list;
}

void searchCustomer() {
    int size = 0;
    CustomerAcc* list = loadCustomers(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo customers to search." << endl;
        return;
    }

    cout << "\nEnter username keyword (0 to cancel): ";
    string keyword;
    cin >> keyword;

    if (keyword == "0") {
        cout << "Search cancelled." << endl;
        delete[] list;
        return;
    }

    string kw = toLowerStr(keyword);
    bool found = false;

    cout << "\n-- Matching Customers --" << endl;
    for (int i = 0; i < size; i++) {
        if (toLowerStr(list[i].username).find(kw) != string::npos) {
            cout << " - " << list[i].username << " (" << list[i].role << ")" << endl;
            found = true;
        }
    }

    if (!found)
        cout << "No customer matches \"" << keyword << "\"." << endl;

    delete[] list;
}

void sortCustomers() {
    int size = 0;
    CustomerAcc* list = loadCustomers(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo customers to sort." << endl;
        return;
    }

    // bubble sort by username A to Z, then save back to the file
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1 - i; j++) {
            if (toLowerStr(list[j].username) > toLowerStr(list[j + 1].username)) {
                CustomerAcc temp = list[j];
                list[j]     = list[j + 1];
                list[j + 1] = temp;
            }
        }
    }

    saveCustomers(list, size);
    cout << "\nCustomers sorted by username (A to Z) and saved." << endl;
    delete[] list;

    viewAllCustomers();
}

void editCustomerPassword() {
    int size = 0;
    CustomerAcc* list = loadCustomers(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo customers available." << endl;
        return;
    }

    cout << "\nEnter username to reset password (0 to cancel): ";
    string target;
    cin >> target;

    if (target == "0") {
        cout << "Cancelled." << endl;
        delete[] list;
        return;
    }

    bool found = false;
    for (int i = 0; i < size; i++) {
        if (list[i].username == target) {
            found = true;
            cout << "Enter new password: ";
            string newPass;
            cin >> newPass;

            if (askYesNo("Confirm reset password for " + target + "?")) {
                list[i].password = newPass;
                saveCustomers(list, size);
                cout << "Password updated successfully." << endl;
            } else {
                cout << "No changes made." << endl;
            }
            break;
        }
    }

    if (!found)
        cout << "Customer \"" << target << "\" not found." << endl;

    delete[] list;
}

void deleteCustomer() {
    int size = 0;
    CustomerAcc* list = loadCustomers(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo customers available." << endl;
        return;
    }

    cout << "\nEnter username to delete (0 to cancel): ";
    string target;
    cin >> target;

    if (target == "0") {
        cout << "Cancelled." << endl;
        delete[] list;
        return;
    }

    int index = -1;
    for (int i = 0; i < size; i++) {
        if (list[i].username == target) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        cout << "Customer \"" << target << "\" not found." << endl;
        delete[] list;
        return;
    }

    if (!askYesNo("Really delete customer " + target + "?")) {
        cout << "Deletion cancelled." << endl;
        delete[] list;
        return;
    }

    CustomerAcc* newList = new CustomerAcc[size - 1];
    int j = 0;
    for (int i = 0; i < size; i++) {
        if (i == index) continue;
        newList[j++] = list[i];
    }

    saveCustomers(newList, size - 1);
    delete[] newList;
    delete[] list;

    cout << "Customer deleted successfully." << endl;
}

void manageCustomers() {
    int choice;
    do {
        cout << "\n=== Manage Customers (Admin) ===" << endl;
        cout << "1. View All Customers" << endl;
        cout << "2. Search Customer" << endl;
        cout << "3. Sort Customers (A to Z)" << endl;
        cout << "4. Reset Customer Password" << endl;
        cout << "5. Delete Customer" << endl;
        cout << "0. Back" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: viewAllCustomers();    break;
            case 2: searchCustomer();      break;
            case 3: sortCustomers();       break;
            case 4: editCustomerPassword(); break;
            case 5: deleteCustomer();      break;
            case 0: break;
            default: cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);
}

// =======================================================
// Borrow records held in a singly linked list (DNP structure)
// Used by the admin record viewer and the customer history.
// =======================================================
struct RecordNode {
    BookRecord data;
    RecordNode* next;
};

class RecordList {
    private:
        RecordNode* headPtr;
        int count;

        // pull out the field we want to sort or search on
        string keyOf(BookRecord r, int field) {
            if (field == 1) return r.customerID;
            if (field == 2) return r.borrowDate;
            return r.borrowID;
        }

    public:
        RecordList() {
            headPtr = nullptr;
            count = 0;
        }

        ~RecordList() {
            clear();
        }

        bool isEmpty() { return headPtr == nullptr; }
        int  size()    { return count; }
        RecordNode* head() { return headPtr; }

        // add to the tail so records keep their original file order
        void addRecord(BookRecord r) {
            RecordNode* node = new RecordNode;
            node->data = r;
            node->next = nullptr;

            if (headPtr == nullptr) {
                headPtr = node;
            } else {
                RecordNode* cur = headPtr;
                while (cur->next != nullptr)
                    cur = cur->next;
                cur->next = node;
            }
            count++;
        }

        void clear() {
            while (headPtr != nullptr) {
                RecordNode* temp = headPtr;
                headPtr = headPtr->next;
                delete temp;
            }
            count = 0;
        }

        // insertion sort done directly on the linked nodes
        void sortByField(int field) {
            if (headPtr == nullptr) return;

            RecordNode* sorted = nullptr;
            RecordNode* current = headPtr;

            while (current != nullptr) {
                RecordNode* nextNode = current->next;

                if (sorted == nullptr ||
                    keyOf(current->data, field) < keyOf(sorted->data, field)) {
                    current->next = sorted;
                    sorted = current;
                } else {
                    RecordNode* s = sorted;
                    while (s->next != nullptr &&
                           keyOf(s->next->data, field) < keyOf(current->data, field))
                        s = s->next;
                    current->next = s->next;
                    s->next = current;
                }
                current = nextNode;
            }
            headPtr = sorted;
        }
};

// read every line of BorrowRecords.txt into the linked list
void loadRecordsIntoList(RecordList& list) {
    ifstream file("BorrowRecords.txt");
    if (!file.is_open()) return;

    BookRecord r;
    while (getline(file, r.borrowID, '|') &&
           getline(file, r.customerID, '|') &&
           getline(file, r.bookID, '|') &&
           getline(file, r.borrowDate, '|') &&
           getline(file, r.returnDate, '|') &&
           getline(file, r.borrowStatus)) {
        list.addRecord(r);
    }
    file.close();
}

// print the heading row used by the record tables
void printRecordHeader() {
    cout << left
         << setw(16) << "BorrowID"
         << setw(14) << "Customer"
         << setw(10) << "BookID"
         << setw(13) << "Borrowed"
         << setw(13) << "Returned"
         << "Status" << endl;
    cout << string(72, '-') << endl;
}

void printRecordRow(BookRecord r) {
    cout << left
         << setw(16) << r.borrowID
         << setw(14) << r.customerID
         << setw(10) << r.bookID
         << setw(13) << r.borrowDate
         << setw(13) << r.returnDate
         << r.borrowStatus << endl;
}

// -------------------------------------------------------
// View Borrow Records - Admin only
// Display every record, search them or sort them by a field.
// -------------------------------------------------------
void viewBorrowRecords() {
    RecordList list;
    loadRecordsIntoList(list);

    if (list.isEmpty()) {
        cout << "\nThere are no borrow records yet." << endl;
        return;
    }

    int choice;
    do {
        cout << "\n=== Borrow Records (Admin) ===" << endl;
        cout << "1. Display All Records" << endl;
        cout << "2. Search by Customer" << endl;
        cout << "3. Search by Book ID" << endl;
        cout << "4. Sort by Borrow ID" << endl;
        cout << "5. Sort by Borrow Date" << endl;
        cout << "6. Edit a Record" << endl;
        cout << "0. Back" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 6) {
            editBorrowRecord();
            // reload so the table reflects the change we just made
            list.clear();
            loadRecordsIntoList(list);
        } else if (choice == 1) {
            cout << "\nShowing " << list.size() << " record(s):" << endl;
            printRecordHeader();
            for (RecordNode* p = list.head(); p != nullptr; p = p->next)
                printRecordRow(p->data);

        } else if (choice == 2 || choice == 3) {
            cout << "Enter keyword: ";
            string kw;
            cin >> kw;
            kw = toLowerStr(kw);

            bool found = false;
            printRecordHeader();
            for (RecordNode* p = list.head(); p != nullptr; p = p->next) {
                string field = (choice == 2) ? p->data.customerID : p->data.bookID;
                if (toLowerStr(field).find(kw) != string::npos) {
                    printRecordRow(p->data);
                    found = true;
                }
            }
            if (!found)
                cout << "No matching record found." << endl;

        } else if (choice == 4 || choice == 5) {
            list.sortByField(choice == 4 ? 0 : 2);
            cout << "\nRecords sorted." << endl;
            printRecordHeader();
            for (RecordNode* p = list.head(); p != nullptr; p = p->next)
                printRecordRow(p->data);

        } else if (choice != 0) {
            cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);
}

// -------------------------------------------------------
// Edit a Borrow Record - Admin only
// Lets staff fix the return date or status of a single record.
// -------------------------------------------------------
void editBorrowRecord() {
    cout << "\nEnter Borrow ID to edit (0 to cancel): ";
    string target;
    cin >> target;

    if (target == "0") {
        cout << "Cancelled." << endl;
        return;
    }

    ifstream readFile("BorrowRecords.txt");
    if (!readFile.is_open()) {
        cout << "No records found." << endl;
        return;
    }

    ofstream tempFile("TempRecords.txt");
    string bID, custID, bkID, bDate, rDate, bStatus;
    bool edited = false;

    while (getline(readFile, bID, '|') && getline(readFile, custID, '|') &&
           getline(readFile, bkID, '|') && getline(readFile, bDate, '|') &&
           getline(readFile, rDate, '|') && getline(readFile, bStatus)) {

        if (bID == target && !edited) {
            cout << "Current return date: " << rDate << endl;
            cout << "Enter new return date (YYYY-MM-DD or - to keep): ";
            string newDate;
            cin >> newDate;
            if (newDate != "-") rDate = newDate;

            cout << "Current status: " << bStatus << endl;
            cout << "Enter new status (Borrowed/Returned or - to keep): ";
            string newStatus;
            cin >> newStatus;
            if (newStatus != "-") bStatus = newStatus;

            edited = true;
        }

        tempFile << bID << "|" << custID << "|" << bkID << "|"
                 << bDate << "|" << rDate << "|" << bStatus << endl;
    }

    readFile.close();
    tempFile.close();
    remove("BorrowRecords.txt");
    rename("TempRecords.txt", "BorrowRecords.txt");

    if (edited)
        cout << "Record updated." << endl;
    else
        cout << "Borrow ID \"" << target << "\" not found." << endl;
}

// -------------------------------------------------------
// Overdue Report - Admin only
// Lists late records, totals the fines and saves them to a txt file.
// -------------------------------------------------------
void overdueReport() {
    RecordList list;
    loadRecordsIntoList(list);

    if (list.isEmpty()) {
        cout << "\nNo borrow records to check." << endl;
        return;
    }

    ofstream out("OverdueReport.txt");
    out << "========== OVERDUE / FINE REPORT ==========" << endl;
    out << "Report date: " << SYSTEM_TODAY << endl;
    out << "-------------------------------------------" << endl;

    cout << "\n========== Overdue / Fine Report ==========" << endl;
    cout << left << setw(14) << "Customer" << setw(10) << "BookID"
         << setw(8) << "Days" << "Fine(RM)" << endl;
    cout << "-------------------------------------------" << endl;

    double totalFine = 0.0;
    int    lateCount = 0;

    for (RecordNode* p = list.head(); p != nullptr; p = p->next) {
        int late = overdueDays(p->data.borrowDate, p->data.returnDate, p->data.borrowStatus);
        if (late > 0) {
            double fine = late * FINE_PER_DAY;
            totalFine += fine;
            lateCount++;

            cout << left << setw(14) << p->data.customerID
                 << setw(10) << p->data.bookID
                 << setw(8)  << late
                 << fixed << setprecision(2) << fine << endl;

            out << p->data.customerID << " | " << p->data.bookID
                << " | " << late << " day(s) | RM "
                << fixed << setprecision(2) << fine << endl;
        }
    }

    if (lateCount == 0)
        cout << "No overdue records. Everyone is on time!" << endl;

    cout << "-------------------------------------------" << endl;
    cout << "Overdue records : " << lateCount << endl;
    cout << "Total fine      : RM " << fixed << setprecision(2) << totalFine << endl;

    out << "-------------------------------------------" << endl;
    out << "Overdue records : " << lateCount << endl;
    out << "Total fine      : RM " << fixed << setprecision(2) << totalFine << endl;
    out.close();

    cout << "Report saved to OverdueReport.txt" << endl;
}

// -------------------------------------------------------
// My Borrow History - Customer only
// Shows only the records that belong to the logged in customer.
// -------------------------------------------------------
void myBorrowHistory(string currentCustomer) {
    RecordList list;
    loadRecordsIntoList(list);

    if (list.isEmpty()) {
        cout << "\nYou have no borrowing history yet." << endl;
        return;
    }

    int sortChoice;
    cout << "\n=== My Borrow History ===" << endl;
    cout << "Sort by 1. Borrow Date  2. Book ID  (0 to keep order): ";
    cin >> sortChoice;

    if (sortChoice == 1) list.sortByField(2);
    else if (sortChoice == 2) list.sortByField(0);   // borrowID begins with book year/id

    cout << "\n" << left << setw(10) << "BookID"
         << setw(28) << "Title"
         << setw(13) << "Borrowed"
         << setw(13) << "Returned"
         << "Status" << endl;
    cout << string(74, '-') << endl;

    int mine = 0;
    for (RecordNode* p = list.head(); p != nullptr; p = p->next) {
        if (p->data.customerID == currentCustomer) {
            string title = getBookTitle(p->data.bookID);
            if ((int)title.size() > 26) title = title.substr(0, 23) + "...";

            cout << left << setw(10) << p->data.bookID
                 << setw(28) << title
                 << setw(13) << p->data.borrowDate
                 << setw(13) << p->data.returnDate
                 << p->data.borrowStatus << endl;
            mine++;
        }
    }

    if (mine == 0)
        cout << "(no records found under your account)" << endl;
    else
        cout << "\nTotal records: " << mine << endl;
}

// -------------------------------------------------------
// Check My Fines - Customer only
// Adds up any late fees from the customer's own records.
// -------------------------------------------------------
void checkMyFines(string currentCustomer) {
    RecordList list;
    loadRecordsIntoList(list);

    if (list.isEmpty()) {
        cout << "\nNo records found, so you have no fines." << endl;
        return;
    }

    cout << "\n=== My Fines ===" << endl;
    cout << "Loan period: " << LOAN_DAYS << " days. "
         << "Late fee: RM " << fixed << setprecision(2)
         << FINE_PER_DAY << " per day." << endl;
    cout << "-----------------------------------" << endl;

    double total = 0.0;
    int late = 0;

    for (RecordNode* p = list.head(); p != nullptr; p = p->next) {
        if (p->data.customerID != currentCustomer) continue;

        int days = overdueDays(p->data.borrowDate, p->data.returnDate, p->data.borrowStatus);
        if (days > 0) {
            double fine = days * FINE_PER_DAY;
            total += fine;
            late++;
            cout << "Book " << p->data.bookID << " - " << days
                 << " day(s) late - RM " << fixed << setprecision(2) << fine << endl;
        }
    }

    if (late == 0)
        cout << "You have no outstanding fines. Well done!" << endl;
    else
        cout << "Total fine to pay: RM " << fixed << setprecision(2) << total << endl;
}

// -------------------------------------------------------
// My Profile - Customer only
// Lets a customer see their details and change their password.
// -------------------------------------------------------
void changeMyPassword(string currentCustomer) {
    int size = 0;
    CustomerAcc* list = loadCustomers(size);
    if (size == 0 || list == nullptr) {
        cout << "Account data not available." << endl;
        return;
    }

    cout << "\nEnter current password (0 to cancel): ";
    string oldPass;
    cin >> oldPass;

    if (oldPass == "0") {
        cout << "Cancelled." << endl;
        delete[] list;
        return;
    }

    int index = -1;
    for (int i = 0; i < size; i++) {
        if (list[i].username == currentCustomer) {
            index = i;
            break;
        }
    }

    if (index == -1 || list[index].password != oldPass) {
        cout << "Current password is incorrect." << endl;
        delete[] list;
        return;
    }

    cout << "Enter new password: ";
    string newPass;
    cin >> newPass;

    if (askYesNo("Save new password?")) {
        list[index].password = newPass;
        saveCustomers(list, size);
        cout << "Password changed successfully." << endl;
    } else {
        cout << "No changes made." << endl;
    }

    delete[] list;
}

void myProfile(string currentCustomer) {
    int choice;
    do {
        cout << "\n=== My Profile ===" << endl;
        cout << "Logged in as: " << currentCustomer << endl;
        cout << "1. View Account Details" << endl;
        cout << "2. Change Password" << endl;
        cout << "0. Back" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            int size = 0;
            CustomerAcc* list = loadCustomers(size);
            bool shown = false;
            for (int i = 0; i < size; i++) {
                if (list[i].username == currentCustomer) {
                    User me(list[i].username, list[i].password);
                    cout << endl;
                    displayAccount(me);   // friend function reads protected fields
                    shown = true;
                    break;
                }
            }
            if (!shown) cout << "Account not found." << endl;
            if (list != nullptr) delete[] list;

        } else if (choice == 2) {
            changeMyPassword(currentCustomer);

        } else if (choice != 0) {
            cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);
}

// =======================================================
// PART F - Book Reservations (Queue based)
// A customer can queue for a book. When a copy comes back the
// admin processes the queue in first-in-first-out order.
// =======================================================
struct Reservation {
    string reservationID;
    string customerID;
    string bookID;
    string reserveDate;
    string status;
};

int countReservations() {
    ifstream file("Reservations.txt");
    if (!file.is_open()) return 0;

    int count = 0;
    string line;
    while (getline(file, line)) {
        if (!line.empty()) count++;
    }
    file.close();
    return count;
}

Reservation* loadReservations(int& size) {
    size = countReservations();
    if (size == 0) return nullptr;

    Reservation* list = new Reservation[size];
    ifstream file("Reservations.txt");

    for (int i = 0; i < size; i++) {
        getline(file, list[i].reservationID, '|');
        getline(file, list[i].customerID, '|');
        getline(file, list[i].bookID, '|');
        getline(file, list[i].reserveDate, '|');
        getline(file, list[i].status);
    }
    file.close();
    return list;
}

void saveReservations(Reservation* list, int size) {
    ofstream file("Reservations.txt", ios::trunc);
    for (int i = 0; i < size; i++) {
        file << list[i].reservationID << "|"
             << list[i].customerID    << "|"
             << list[i].bookID        << "|"
             << list[i].reserveDate   << "|"
             << list[i].status        << endl;
    }
    file.close();
}

// -------------------------------------------------------
// Reserve Book - Customer only
// -------------------------------------------------------
void reserveBook(string currentCustomer) {
    int size = 0;
    Book* bookList = LoadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books in the library to reserve." << endl;
        return;
    }

    cout << "\n=== Reserve Book ===" << endl;
    cout << "Enter Book ID to reserve (0 to cancel): ";
    string id;
    cin >> id;

    if (id == "0") {
        cout << "Reservation cancelled." << endl;
        delete[] bookList;
        return;
    }

    int index = -1;
    for (int i = 0; i < size; i++) {
        if (bookList[i].bookID == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        cout << "Book ID not found." << endl;
        delete[] bookList;
        return;
    }

    if (bookList[index].stock > 0) {
        cout << "This book is in stock. You can borrow it directly instead." << endl;
        if (!askYesNo("Reserve it anyway?")) {
            delete[] bookList;
            return;
        }
    }

    string title = bookList[index].title;
    delete[] bookList;

    // stop the same customer from reserving the same book twice
    int rSize = 0;
    Reservation* rList = loadReservations(rSize);
    for (int i = 0; i < rSize; i++) {
        if (rList[i].customerID == currentCustomer &&
            rList[i].bookID == id && rList[i].status == "Waiting") {
            cout << "You already have a waiting reservation for this book." << endl;
            if (rList != nullptr) delete[] rList;
            return;
        }
    }
    if (rList != nullptr) delete[] rList;

    ofstream file("Reservations.txt", ios::app);
    file << "RSV" << (rSize + 1) << "|"
         << currentCustomer << "|"
         << id << "|"
         << SYSTEM_TODAY << "|Waiting" << endl;
    file.close();

    cout << "Reservation placed for '" << title << "'. You are in the queue." << endl;
}

// -------------------------------------------------------
// My Reservations / Cancel Reservation - Customer only
// -------------------------------------------------------
void cancelReservation(string currentCustomer) {
    int size = 0;
    Reservation* list = loadReservations(size);

    if (size == 0 || list == nullptr) {
        cout << "You have no reservations to cancel." << endl;
        return;
    }

    cout << "Enter Reservation ID to cancel (0 to go back): ";
    string id;
    cin >> id;

    if (id == "0") {
        cout << "Cancelled." << endl;
        delete[] list;
        return;
    }

    bool done = false;
    for (int i = 0; i < size; i++) {
        if (list[i].reservationID == id &&
            list[i].customerID == currentCustomer &&
            list[i].status == "Waiting") {
            list[i].status = "Cancelled";
            done = true;
            break;
        }
    }

    if (done) {
        saveReservations(list, size);
        cout << "Reservation cancelled." << endl;
    } else {
        cout << "No matching waiting reservation found under your account." << endl;
    }

    delete[] list;
}

void myReservations(string currentCustomer) {
    int size = 0;
    Reservation* list = loadReservations(size);

    if (size == 0 || list == nullptr) {
        cout << "\nYou have no reservations yet." << endl;
        return;
    }

    cout << "\n=== My Reservations ===" << endl;
    cout << left << setw(10) << "RsvID" << setw(10) << "BookID"
         << setw(14) << "Date" << "Status" << endl;
    cout << "----------------------------------------" << endl;

    int mine = 0;
    for (int i = 0; i < size; i++) {
        if (list[i].customerID == currentCustomer) {
            cout << left << setw(10) << list[i].reservationID
                 << setw(10) << list[i].bookID
                 << setw(14) << list[i].reserveDate
                 << list[i].status << endl;
            mine++;
        }
    }

    if (mine == 0)
        cout << "(nothing under your account)" << endl;

    delete[] list;

    if (mine > 0 && askYesNo("\nCancel one of these?"))
        cancelReservation(currentCustomer);
}

// -------------------------------------------------------
// Manage Reservations - Admin only
// Processing serves the waiting queue in FIFO order using ADTqueue.
// -------------------------------------------------------
void viewAllReservations() {
    int size = 0;
    Reservation* list = loadReservations(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo reservations recorded." << endl;
        return;
    }

    cout << "\n===== All Reservations =====" << endl;
    cout << left << setw(10) << "RsvID" << setw(14) << "Customer"
         << setw(10) << "BookID" << setw(14) << "Date" << "Status" << endl;
    cout << "--------------------------------------------------" << endl;

    for (int i = 0; i < size; i++) {
        cout << left << setw(10) << list[i].reservationID
             << setw(14) << list[i].customerID
             << setw(10) << list[i].bookID
             << setw(14) << list[i].reserveDate
             << list[i].status << endl;
    }

    delete[] list;
}

void processReservation() {
    int size = 0;
    Reservation* list = loadReservations(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo reservations to process." << endl;
        return;
    }

    cout << "\nEnter Book ID to process (0 to cancel): ";
    string id;
    cin >> id;

    if (id == "0") {
        cout << "Cancelled." << endl;
        delete[] list;
        return;
    }

    // load every waiting reservation for this book into the queue, in order
    ADTqueue waiting;
    bool anyWaiting = false;
    for (int i = 0; i < size; i++) {
        if (list[i].bookID == id && list[i].status == "Waiting") {
            waiting.append(list[i].reservationID);
            anyWaiting = true;
        }
    }

    if (!anyWaiting) {
        cout << "No waiting reservations for this book." << endl;
        delete[] list;
        return;
    }

    // serve the front of the queue - that customer is next in line
    string nextID = waiting.serve();

    for (int i = 0; i < size; i++) {
        if (list[i].reservationID == nextID) {
            list[i].status = "Fulfilled";
            cout << "Reservation " << nextID << " for customer "
                 << list[i].customerID << " has been fulfilled." << endl;
            break;
        }
    }

    saveReservations(list, size);
    delete[] list;
}

// remove every reservation that is no longer waiting to keep the file tidy
void clearCompletedReservations() {
    int size = 0;
    Reservation* list = loadReservations(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo reservations to clean up." << endl;
        return;
    }

    int kept = 0;
    for (int i = 0; i < size; i++) {
        if (list[i].status == "Waiting")
            kept++;
    }

    if (kept == size) {
        cout << "\nNothing to remove. All reservations are still waiting." << endl;
        delete[] list;
        return;
    }

    if (!askYesNo("Remove all fulfilled and cancelled reservations?")) {
        cout << "Cleanup cancelled." << endl;
        delete[] list;
        return;
    }

    Reservation* newList = new Reservation[kept > 0 ? kept : 1];
    int j = 0;
    for (int i = 0; i < size; i++) {
        if (list[i].status == "Waiting")
            newList[j++] = list[i];
    }

    saveReservations(newList, kept);
    delete[] newList;
    delete[] list;

    cout << (size - kept) << " reservation(s) removed." << endl;
}

void manageReservations() {
    int choice;
    do {
        cout << "\n=== Manage Reservations (Admin) ===" << endl;
        cout << "1. View All Reservations" << endl;
        cout << "2. Process Next In Queue" << endl;
        cout << "3. Clear Completed Reservations" << endl;
        cout << "0. Back" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: viewAllReservations(); break;
            case 2: processReservation();  break;
            case 3: clearCompletedReservations(); break;
            case 0: break;
            default: cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);
}

// =======================================================
// PART G - Extra actions: statistics, renew and force return
// =======================================================

// -------------------------------------------------------
// Book Statistics - Admin only
// A quick breakdown of the collection by stock and category.
// -------------------------------------------------------
void bookStatistics() {
    int size = 0;
    Book* bookList = LoadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books to analyse." << endl;
        return;
    }

    int totalStock = 0;
    int available  = 0;
    int outOfStock = 0;

    for (int i = 0; i < size; i++) {
        totalStock += bookList[i].stock;
        if (bookList[i].stock > 0) available++;
        else outOfStock++;
    }

    cout << "\n========== Book Statistics ==========" << endl;
    cout << "Total titles        : " << size << endl;
    cout << "Total copies (stock): " << totalStock << endl;
    cout << "Available titles    : " << available << endl;
    cout << "Out of stock titles : " << outOfStock << endl;

    // count how many titles fall under each category without using STL maps
    cout << "\n-- Titles per Category --" << endl;
    bool* counted = new bool[size];
    for (int i = 0; i < size; i++) counted[i] = false;

    for (int i = 0; i < size; i++) {
        if (counted[i]) continue;

        int catCount = 1;
        counted[i] = true;
        for (int j = i + 1; j < size; j++) {
            if (!counted[j] && bookList[j].category == bookList[i].category) {
                catCount++;
                counted[j] = true;
            }
        }
        cout << " - " << setw(16) << left << bookList[i].category
             << ": " << catCount << endl;
    }

    delete[] counted;
    delete[] bookList;
    cout << "=====================================" << endl;
}

// -------------------------------------------------------
// Renew Book - Customer only
// Resets the 14 day loan clock on a book the customer still holds.
// -------------------------------------------------------
void renewBook(string currentCustomer) {
    cout << "\n=== Renew Book ===" << endl;
    cout << "Enter Book ID to renew (0 to cancel): ";
    string id;
    cin >> id;

    if (id == "0") {
        cout << "Renewal cancelled." << endl;
        return;
    }

    ifstream readFile("BorrowRecords.txt");
    if (!readFile.is_open()) {
        cout << "No borrowing records found." << endl;
        return;
    }

    ofstream tempFile("TempRecords.txt");
    string bID, custID, bkID, bDate, rDate, bStatus;
    bool renewed = false;

    while (getline(readFile, bID, '|') && getline(readFile, custID, '|') &&
           getline(readFile, bkID, '|') && getline(readFile, bDate, '|') &&
           getline(readFile, rDate, '|') && getline(readFile, bStatus)) {

        if (custID == currentCustomer && bkID == id &&
            bStatus == "Borrowed" && !renewed) {
            // push the borrow date forward to today so the due date is reset
            tempFile << bID << "|" << custID << "|" << bkID << "|"
                     << SYSTEM_TODAY << "|" << rDate << "|" << bStatus << endl;
            renewed = true;
        } else {
            tempFile << bID << "|" << custID << "|" << bkID << "|"
                     << bDate << "|" << rDate << "|" << bStatus << endl;
        }
    }

    readFile.close();
    tempFile.close();

    remove("BorrowRecords.txt");
    rename("TempRecords.txt", "BorrowRecords.txt");

    if (renewed)
        cout << "Loan renewed. New due date is " << LOAN_DAYS
             << " days from " << SYSTEM_TODAY << "." << endl;
    else
        cout << "No active loan found for that book under your account." << endl;
}

// -------------------------------------------------------
// Force Return - Admin only
// Lets staff return a book on behalf of a customer.
// -------------------------------------------------------
void forceReturn() {
    cout << "\n=== Force Return (Admin) ===" << endl;
    cout << "Enter Customer username (0 to cancel): ";
    string cust;
    cin >> cust;
    if (cust == "0") {
        cout << "Cancelled." << endl;
        return;
    }

    cout << "Enter Book ID: ";
    string id;
    cin >> id;

    int size = 0;
    Book* bookList = LoadBooks(size);
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
        cout << "Book ID not found." << endl;
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
    bool updated = false;

    while (getline(readFile, bID, '|') && getline(readFile, custID, '|') &&
           getline(readFile, bkID, '|') && getline(readFile, bDate, '|') &&
           getline(readFile, rDate, '|') && getline(readFile, bStatus)) {

        if (custID == cust && bkID == id && bStatus == "Borrowed" && !updated) {
            tempFile << bID << "|" << custID << "|" << bkID << "|"
                     << bDate << "|" << SYSTEM_TODAY << "|Returned" << endl;
            updated = true;
        } else {
            tempFile << bID << "|" << custID << "|" << bkID << "|"
                     << bDate << "|" << rDate << "|" << bStatus << endl;
        }
    }

    readFile.close();
    tempFile.close();
    remove("BorrowRecords.txt");
    rename("TempRecords.txt", "BorrowRecords.txt");

    if (updated) {
        SaveBooks(bookList, size);
        cout << "Book returned on behalf of " << cust << "." << endl;
    } else {
        // no matching loan, so undo the stock change we made earlier
        for (int i = 0; i < size; i++) {
            if (bookList[i].bookID == id) {
                bookList[i].stock--;
                if (bookList[i].stock == 0)
                    bookList[i].status = "Borrowed";
                break;
            }
        }
        SaveBooks(bookList, size);
        cout << "No active loan found for that customer and book." << endl;
    }

    delete[] bookList;
}

// -------------------------------------------------------
// Top Borrowed Books - Admin only
// Counts how many times each book ID appears in the records
// and prints them from the most borrowed to the least.
// -------------------------------------------------------
void topBorrowedBooks() {
    RecordList list;
    loadRecordsIntoList(list);

    if (list.isEmpty()) {
        cout << "\nNo borrow records to rank yet." << endl;
        return;
    }

    int total = list.size();
    string* ids    = new string[total];
    int*    counts = new int[total];
    int unique = 0;

    // tally every borrow record into a small unique list of book IDs
    for (RecordNode* p = list.head(); p != nullptr; p = p->next) {
        int found = -1;
        for (int i = 0; i < unique; i++) {
            if (ids[i] == p->data.bookID) {
                found = i;
                break;
            }
        }

        if (found == -1) {
            ids[unique]    = p->data.bookID;
            counts[unique] = 1;
            unique++;
        } else {
            counts[found]++;
        }
    }

    // simple selection sort so the highest count sits at the top
    for (int i = 0; i < unique - 1; i++) {
        int maxIndex = i;
        for (int j = i + 1; j < unique; j++) {
            if (counts[j] > counts[maxIndex])
                maxIndex = j;
        }
        if (maxIndex != i) {
            int tc = counts[i]; counts[i] = counts[maxIndex]; counts[maxIndex] = tc;
            string ts = ids[i]; ids[i] = ids[maxIndex]; ids[maxIndex] = ts;
        }
    }

    cout << "\n========== Top Borrowed Books ==========" << endl;
    cout << left << setw(6) << "Rank" << setw(10) << "BookID"
         << setw(30) << "Title" << "Times" << endl;
    cout << "----------------------------------------------" << endl;

    for (int i = 0; i < unique; i++) {
        string title = getBookTitle(ids[i]);
        if ((int)title.size() > 28) title = title.substr(0, 25) + "...";
        cout << left << setw(6) << (i + 1)
             << setw(10) << ids[i]
             << setw(30) << title
             << counts[i] << endl;
    }

    delete[] ids;
    delete[] counts;
    cout << "========================================" << endl;
}

// =======================================================
// PART H - Manage Admin Accounts (Admin only)
// Same idea as the customer manager but for the Admin.txt file.
// =======================================================
struct AdminAcc {
    string username;
    string password;
    string role;
};

int countAdmins() {
    ifstream file("Admin.txt");
    if (!file.is_open()) return 0;

    int count = 0;
    string line;
    while (getline(file, line)) {
        if (!line.empty()) count++;
    }
    file.close();
    return count;
}

AdminAcc* loadAdmins(int& size) {
    size = countAdmins();
    if (size == 0) return nullptr;

    AdminAcc* list = new AdminAcc[size];
    ifstream file("Admin.txt");

    for (int i = 0; i < size; i++) {
        getline(file, list[i].username, '|');
        getline(file, list[i].password, '|');
        getline(file, list[i].role);
    }
    file.close();
    return list;
}

void saveAdmins(AdminAcc* list, int size) {
    ofstream file("Admin.txt", ios::trunc);
    for (int i = 0; i < size; i++) {
        file << list[i].username << "|"
             << list[i].password << "|"
             << list[i].role     << endl;
    }
    file.close();
}

void viewAllAdmins() {
    int size = 0;
    AdminAcc* list = loadAdmins(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo admin accounts found." << endl;
        return;
    }

    cout << "\n===== Admin Accounts =====" << endl;
    cout << left << setw(6) << "No" << setw(20) << "Username" << "Role" << endl;
    cout << "-----------------------------------" << endl;
    for (int i = 0; i < size; i++) {
        cout << left << setw(6) << (i + 1)
             << setw(20) << list[i].username
             << list[i].role << endl;
    }

    delete[] list;
}

void addAdmin() {
    cout << "\nEnter new admin username (0 to cancel): ";
    string u;
    cin >> u;
    if (u == "0") {
        cout << "Cancelled." << endl;
        return;
    }

    int size = 0;
    AdminAcc* list = loadAdmins(size);
    for (int i = 0; i < size; i++) {
        if (list[i].username == u) {
            cout << "That admin username already exists." << endl;
            if (list != nullptr) delete[] list;
            return;
        }
    }
    if (list != nullptr) delete[] list;

    cout << "Enter password: ";
    string p;
    cin >> p;

    if (askYesNo("Create admin " + u + "?")) {
        ofstream file("Admin.txt", ios::app);
        file << u << "|" << p << "|Admin" << endl;
        file.close();
        cout << "Admin account created." << endl;
    } else {
        cout << "No changes made." << endl;
    }
}

void deleteAdmin() {
    int size = 0;
    AdminAcc* list = loadAdmins(size);

    if (size == 0 || list == nullptr) {
        cout << "\nNo admin accounts available." << endl;
        return;
    }

    if (size == 1) {
        cout << "\nCannot delete the last admin account." << endl;
        delete[] list;
        return;
    }

    cout << "\nEnter admin username to delete (0 to cancel): ";
    string target;
    cin >> target;
    if (target == "0") {
        cout << "Cancelled." << endl;
        delete[] list;
        return;
    }

    int index = -1;
    for (int i = 0; i < size; i++) {
        if (list[i].username == target) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        cout << "Admin \"" << target << "\" not found." << endl;
        delete[] list;
        return;
    }

    if (!askYesNo("Really delete admin " + target + "?")) {
        cout << "Deletion cancelled." << endl;
        delete[] list;
        return;
    }

    AdminAcc* newList = new AdminAcc[size - 1];
    int j = 0;
    for (int i = 0; i < size; i++) {
        if (i == index) continue;
        newList[j++] = list[i];
    }

    saveAdmins(newList, size - 1);
    delete[] newList;
    delete[] list;
    cout << "Admin account deleted." << endl;
}

void manageAdmins() {
    int choice;
    do {
        cout << "\n=== Manage Admins ===" << endl;
        cout << "1. View All Admins" << endl;
        cout << "2. Add New Admin" << endl;
        cout << "3. Delete Admin" << endl;
        cout << "0. Back" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: viewAllAdmins(); break;
            case 2: addAdmin();      break;
            case 3: deleteAdmin();   break;
            case 0: break;
            default: cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);
}

// -------------------------------------------------------
// Undo Last Delete - Admin only
// Pops the most recently deleted book off the stack and writes
// it back into Books.txt.
// -------------------------------------------------------
void restoreLastDeleted() {
    if (deletedBooks.isEmpty()) {
        cout << "\nNothing to undo. No book was deleted this session." << endl;
        return;
    }

    Book b = deletedBooks.peek();
    cout << "\nLast deleted book: " << b.bookID << " - " << b.title << endl;

    if (!askYesNo("Restore this book?")) {
        cout << "Undo cancelled." << endl;
        return;
    }

    b = deletedBooks.pop();

    ofstream file("Books.txt", ios::app);
    file << b.bookID   << "|" << b.title  << "|"
         << b.author   << "|" << b.category << "|"
         << b.year     << "|" << b.stock   << "|"
         << b.status   << endl;
    file.close();

    cout << "Book '" << b.title << "' restored successfully." << endl;
}

// -------------------------------------------------------
// Display Books with a filter
// Lets the user list everything, or only one category, or only
// the books that are currently in stock.
// -------------------------------------------------------
void displayBooksFiltered() {
    cout << "\n=== Display Books ===" << endl;
    cout << "1. Show All" << endl;
    cout << "2. Show by Category" << endl;
    cout << "3. Show Available Only" << endl;
    cout << "Enter choice: ";
    
    MBooks b;
    int choice;
    cin >> choice;

    if (choice == 1) {
        b.DisBooks();
        return;
    }

    int size = 0;
    Book* bookList = LoadBooks(size);
    if (size == 0 || bookList == nullptr) {
        cout << "No books found." << endl;
        return;
    }

    string wantCategory = "";
    if (choice == 2) {
        cout << "Enter category: ";
        cin >> wantCategory;
        wantCategory = toLowerStr(wantCategory);
    } else if (choice != 3) {
        cout << "Invalid choice." << endl;
        delete[] bookList;
        return;
    }

    cout << "\n" << left << setw(10) << "ID" << setw(28) << "Title"
         << setw(16) << "Category" << setw(8) << "Stock" << "Status" << endl;
    cout << string(72, '-') << endl;

    int shown = 0;
    for (int i = 0; i < size; i++) {
        bool match = false;

        if (choice == 2 && toLowerStr(bookList[i].category) == wantCategory)
            match = true;
        else if (choice == 3 && bookList[i].stock > 0)
            match = true;

        if (match) {
            string title = bookList[i].title;
            if ((int)title.size() > 26) title = title.substr(0, 23) + "...";

            cout << left << setw(10) << bookList[i].bookID
                 << setw(28) << title
                 << setw(16) << bookList[i].category
                 << setw(8)  << bookList[i].stock
                 << bookList[i].status << endl;
            shown++;
        }
    }

    if (shown == 0)
        cout << "No books matched that filter." << endl;
    else
        cout << "\nShowing " << shown << " book(s)." << endl;

    delete[] bookList;
}
