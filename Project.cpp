#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
using namespace std;

void addBook();
void editBook();
void deleteBook();
void displayBooks();
void searchBook(string username, bool isAdmin);
void sortBooks();

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

class ADTqueue
{
    private:
        string queue[10];
        int head, tail;
    
    public:
        ADTqueue(){
            tail = -1;
            head = 0;
        }

        int empty(){
            if (head == tail + 1)
               return 1;
            else
               return 0;
        }

        int full(){
            if (tail == 9)
               return 1;
            else
               return 0;
        }

        void append(string item){
            if (!full()){
                tail++;
                queue[tail] = item;
            }
            else{
                cout<<"Queue is Full"<<endl;
            }
        }

        string serve(){
            if (!empty()){
                item = queue[head];
                head++;
                return item;
            }
            else{
                cout<<"Queue is Empty"<<endl;
                return "";
            }
        }
};

class Account {
    protected:
        string username;
        string password;
        string role;
    
    public:
        Account(){
            username = "";
            password = "";
            role = "";
        }

        ~Account(){}

        friend void displayAccount(const Account& a);
        friend class Admin;
        
        string getUsername(){
            return username;
        }

        string getPassword(){
            return password;
        }

        string getRole(){
            return role;
        }
};

class User : public Account {
    public :
        User() {
            role = "Customer";
        }

        User(string u, string p) {
            username = u;
            password = p;
            role = "Customer";
        }

        ~User(){}
        friend void displayUser(const User& u);

        void setUsername(string u){
            username = u;
        }

        void setPassword(string p){
            password = p;
        }
};

class Admin : public Account {
    public:
        Admin(){
            role = "Admin";
        }

        Admin(string u, string p){
            username = u;
            password = p;
            role = "Admin";
        }
        
        ~Admin(){}
        friend void displayAdmin(const Admin& a);

        void setUsername(string u){
            username = u;
        }

        void setPassword(string p){
            password = p;
        }
};

void registerUser(){
    string u, p;
    cout << "\n== Register New Customer ==" << endl;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;
    cout << endl;

    string fu, fp, frole;
    ifstream checkFile("Customer.txt");
    while(getline(checkFile, fu, '|') && getline(checkFile, fp, '|') && getline(checkFile, frole)){
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

string loginUser(){
    string u, p, fu,fp, frole;
    cout << "\n== Customer Login ==" << endl;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;
    cout << endl;

    ifstream file("Customer.txt");
    while (getline(file,fu,'|') && getline(file, fp, '|') && getline(file, frole)){
        if(u == fu && p == fp)
        {
            file.close();
            cout << "Login successful! Welcome, " << u << "!" << endl;
            return u;
        }
    }

    file.close();
    cout << "Invalid username or password." << endl;
    return "";
}

string loginAdmin(){
    string u, p, fu, fp, frole;
    cout << "\n== Admin Login ==" << endl;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;
    cout << endl;

    ifstream file("Admin.txt");
    while(getline(file, fu,'|') && getline(file, fp, '|') && getline(file, frole)){
        if(u == fu && p == fp)
        {
            file.close();
            cout << "Login successful! Welcome, Admin " << u << "!" << endl;
            return u;
        }
    }

    file.close();
    cout << "Invalid username or password." << endl;
    return "";
}

void logout(){
    cout << "\nLogging out... Goodbye!" << endl <<endl;
}

// 1. Borrow Book
void borrowBook(string currentCustomer){
    cin.ignore(); //Clear input buffer to prevent skipping inputs
    cout<<"\n== Borrow Book ==" <<endl;

    //Load all books from file
    int size = 0;
    Book* bookList = loadBooks(size);

    if(size == 0 || bookList == nullptr){
        cout<<"No books available in the library."<<endl;
        return;
    }

    string id;
    cout<<"Enter Book ID to borrow: ";
    cin>>id;

    bool found = false;
    for(int i =0; i<size; i++){
        if(bookList[i].bookID ==id){
            found = true;

            //Check stock available
            if(bookList[i].stock > 0){
                bookList[i].stock--; //Deduct stock quantity

                //If stock 0, update status to borrowed
                if(bookList[i].stock == 0){
                    bookList[i].status = "Borrowed";
                }
                
                //Queue processing
                ADTqueue borrowQueue;
                borrowQueue.append(currentCustomer); 

                //queue serve to complete processing
                string processingUser = borrowQueue.serve();

                //Wrte transaction borrowing history record file
                ofstream recordFile("BorrowRecords.txt", ios::app);
                recordFile << "BR" << bookList[i].year << id << "|"
                           << processingUser << "|"
                           << bookList[i].bookID << "|"
                           << "2026-06-07|---|Borrowed" << endl;
                recordFile.close();

                cout << "Book'" << bookList[i].title << "' borrowed successfully by " << processingUser << "!" <<endl;
            }else{
                cout<<"Sorry, this book is out of stock!" << endl;
            }
            break;
            }
        }
        
        if(found){
            saveBooks(bookList, size); //Save updated stock details
        }else{
            cout << "Book ID not found." << endl;
        }

        delete[] bookList; 
    }

//2.Return Book
void returnBook(string currentCustomer){
    cin.ignore(); 
    cout << "\n=== Return Book ===" << endl;

    string id;
    cout << "Enter Book ID to return: ";
    cin >> id;

    //Load book update stock
    int size = 0;
    Book* bookList = loadBooks(size);
    bool bookFound = false;

    for (int i=0; i<size; i++){
        if (bookList[i].bookID ==id){
            bookList[i].stock++; //Restock book
            bookList[i].status = "Available"; 
            bookFound = true;
            break;

        }
    }

    if(!bookFound){
        cout << "This book was not found in library system." << endl;
        if(bookList != nullptr) delete[] bookList;
        return;
    }

    //Read borrowing logs to process the return transaction
    ifstream readFile("BorrowRecords.txt");
    if(!readFile.is_open()){
        cout << "No borrowing records found." << endl;
        delete[] bookList;
        return;
    }

    ofstream tempFile("TempRecords.txt");
    string bID, custID, bkID, bDate, rDate, bStatus; //booking, customerID, bookID, borrowDate, returnDate, borrowStatus
    bool recordUpdated = false;

    //Queue processing for update records sequentially
    ADTqueue returnQueue;

    while (getline(readFile, bID, '|') && getline(readFile, custID, '|') &&
           getline(readFile, bkID, '|') && getline(readFile, bDate, '|') &&
           getline(readFile, rDate, '|') && getline(readFile, bStatus)){

            if(custID == currentCustomer && bkID == id && bStatus == "Borrowed" && !recordUpdated){
                returnQueue.append(custID); //Add request returning
                string processingCust = returnQueue.serve(); //Process item

                tempFile << bID << "|" <<processingCust << "|" << bkID << "|" << 
                bDate << "|2026-06-14|Returned" <<endl;
                recordUpdated = true;
            }else{
                tempFile << bID << "|" << custID << "|" << bkID << "|" << 
                bDate << "|" << rDate << "|" << bStatus <<endl;
            }
        }

        readFile.close();
        tempFile.close();

        //Replace old record file with updated record
        remove("BorrowRecords.txt");
        rename("TempRecords.txt", "BorrowRecords.txt");

        if(recordUpdated){
            saveBooks(bookList, size); //Save book update
            cout << "Book returned successfully! Thank you, " << currentCustomer << "!" << endl;
        }else{
            cout << "You have not borrowed this book or it was already returned." << endl;
            for (int i=0; i<size; i++){
                if(bookList[i].bookID == id){
                    bookList[i].stock--; //Revert stock update
                    if(bookList[i].stock == 0){
                        bookList[i].status = "Borrowed";
                    }
                    break;
                }
            }
            saveBooks(bookList, size); //Save reverted stock update
        }
        delete[] bookList; //Free memory
}

        
void customerMenu(string username){
    int choice;
    do{
        cout << "\n=== Customer Menu ===" << endl;
        cout << "1. Borrow Book" << endl;
        cout << "2. Return Book" << endl;
        cout << "3. Search Book" << endl;
        cout << "4. Display Books" << endl;
        cout << "5. View Borrowing Summary" << endl;
        cout << "0. Logout" << endl << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch(choice) {
        	case 1:
<<<<<<< HEAD
        		break;
        	
        	case 2:
        		break;
        	
        	case 3:
        		searchBook(username, false);
=======
        		borrowBook(username);
        		break;
        	
        	case 2:
        		returnBook(username);
        		break;
        	
        	case 3:
        		searchBook();
>>>>>>> 07f33b8690c0eae95021a8178a43d4c41d7233b9
        		break;
        	
        	case 4:
        		displayBooks();
        		break;

            case 5:
                break;

            case 0:
                break;

            default: cout<<"\nInvalid choice. Please try again."<<endl;
		}
    } while(choice != 0);

    logout();
}

void adminMenu(string username){
    int choice;
    do{
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

        switch(choice) {
        	case 1:
        		addBook();
				break;
			
			case 2:
				editBook();
				break;
			
			case 3:
				deleteBook();
				break;
			
			case 4:
				searchBook(username, true);
				break;
			
			case 5:
				sortBooks();
				break;
				
			case 6:
				displayBooks();
				break;

            case 7:
                break;

            case 0:
                break;

            default: cout<<"\nInvalid choice. Please try again."<<endl;
		}
    } while(choice != 0);

    logout();
}

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

        switch(choice){
            case 1: {
                try{
                    string loggedIn = loginUser(); 
                    
                    if(loggedIn.empty())
                    {
                        throw "Login failed!";
                    }
                    customerMenu(loggedIn);

                } catch(const char* e){
                    cout << "Error: " << e << endl << endl;
                }
                break;
            }

            case 2:{
                registerUser();
                break;
            }

            case 3:{
                try{
                    string loggedIn = loginAdmin();
                    
                    if(loggedIn.empty())
                    {
                        throw "Login failed!";
                    }
                    adminMenu(loggedIn);
                } catch(const char* e){
                    cout << "Error: " << e << endl << endl; 
                }
                break;
            }

            case 0:{
                logout();
                break;
            }

            default:{
                cout << "Invalid choice. Please try again." << endl;
                break;
            }
        }
    } while(choice != 0);

    return 0;
}

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

        if (!tempYear.empty()) bookList[i].year = stoi(tempYear);
        if (!tempStock.empty()) bookList[i].stock = stoi(tempStock);
    }
    file.close();
    return bookList;
}

void saveBooks(Book* bookList, int size) {
    ofstream file("Books.txt", ios::trunc);
    for (int i = 0; i < size; i++) {
        file << bookList[i].bookID << "|"
             << bookList[i].title << "|"
             << bookList[i].author << "|"
             << bookList[i].category << "|"
             << bookList[i].year << "|"
             << bookList[i].stock << "|"
             << bookList[i].status << endl;
    }
    file.close();
}

void addBook() {
    cout << "\n== Add New Book ==" << endl;
    Book newBook;
    
    cout << "Enter Book ID: "; cin >> newBook.bookID;
    cin.ignore();
    cout << "Enter Title: "; getline(cin, newBook.title);
    cout << "Enter Author: "; getline(cin, newBook.author);
    cout << "Enter Category: "; getline(cin, newBook.category);
    cout << "Enter Publication Year: "; cin >> newBook.year;
    cout << "Enter Stock Quantity: "; cin >> newBook.stock;
    
    newBook.status = (newBook.stock > 0) ? "Available" : "Borrowed";

    ofstream file("Books.txt", ios::app);
    file << newBook.bookID << "|" << newBook.title << "|" << newBook.author << "|"
         << newBook.category << "|" << newBook.year << "|" << newBook.stock << "|" 
         << newBook.status << endl;
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
    cout << left << setw(10) << "ID" << setw(25) << "Title" << setw(20) << "Author" 
         << setw(15) << "Category" << setw(8) << "Year" << setw(8) << "Stock" << "Status" << endl;
    cout << "---------------------------------------------------------------------------------------------" << endl;

    for (int i = 0; i < size; i++) {
        cout << left << setw(10) << bookList[i].bookID 
             << setw(25) << bookList[i].title.substr(0, 23)
             << setw(20) << bookList[i].author.substr(0, 18)
             << setw(15) << bookList[i].category.substr(0, 13)
             << setw(8) << bookList[i].year 
             << setw(8) << bookList[i].stock 
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

int linearSearch(Book* bookList, int size, string keyword) {

    string kw = keyword;
    for (int i = 0; i < (int)kw.size(); i++)
        kw[i] = tolower(kw[i]);

    for (int i = 0; i < size; i++) {

        string t = bookList[i].title;
        for (int j = 0; j < (int)t.size(); j++)
            t[j] = tolower(t[j]);

        if (t.find(kw) != string::npos)
            return i;
    }

    return -1;
}

int binarySearch(Book* bookList, int size, string targetID) {

    int first = 0;
    int last  = size - 1;

    while (first <= last) {

        int mid = (first + last) / 2;

        if (bookList[mid].bookID == targetID) {
            return mid;

        } else if (bookList[mid].bookID < targetID) {
            first = mid + 1;

        } else {
            last = mid - 1;
        }
    }

    return -1;
}

void bubbleSort(Book* bookList, int size) {

    for (int i = 0; i < size - 1; i++) {
        bool swapped = false;

        for (int j = 0; j < size - 1 - i; j++) {
            if (bookList[j].title > bookList[j + 1].title) {

                Book temp        = bookList[j];
                bookList[j]      = bookList[j + 1];
                bookList[j + 1]  = temp;
                swapped = true;
            }
        }

        if (!swapped) break;
    }
}

void selectionSort(Book* bookList, int size) {

    for (int i = 0; i < size - 1; i++) {

        int minIndex = i;

        for (int j = i + 1; j < size; j++) {
            if (bookList[j].bookID < bookList[minIndex].bookID) {
                minIndex = j;
            }
        }

        if (minIndex != i) {
            Book temp          = bookList[i];
            bookList[i]        = bookList[minIndex];
            bookList[minIndex] = temp;
        }
    }
}

void searchBook(string username, bool isAdmin) {

    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books in the library right now." << endl;
        return;
    }

    cout << "\n== Search Book ==" << endl;
    cout << "1. Search by Title (keyword)" << endl;
    cout << "2. Search by Book ID (exact)" << endl;
    cout << "Enter choice: ";

    int searchChoice;
    cin >> searchChoice;
    cin.ignore();

    if (searchChoice == 1) {
        cout << "Enter keyword to search in title: ";
        string keyword;
        getline(cin, keyword);

        bool anyFound = false;
        string kw = keyword;
        for (int i = 0; i < (int)kw.size(); i++)
            kw[i] = tolower(kw[i]);

        cout << "\n-- Search Results for \"" << keyword << "\" --" << endl;
        cout << left
             << setw(8)  << "ID"
             << setw(40) << "Title"
             << setw(20) << "Author"
             << setw(12) << "Category";

        if (isAdmin)
            cout << setw(8) << "Stock";

        cout << "Status" << endl;
        cout << string(isAdmin ? 96 : 88, '-') << endl;

        for (int i = 0; i < size; i++) {
            string t = bookList[i].title;
            string tl = t;
            for (int j = 0; j < (int)tl.size(); j++)
                tl[j] = tolower(tl[j]);

            if (tl.find(kw) != string::npos) {
                anyFound = true;

                string title  = bookList[i].title;
                string author = bookList[i].author;
                if ((int)title.size()  > 38) title  = title.substr(0, 35)  + "...";
                if ((int)author.size() > 18) author = author.substr(0, 15) + "...";

                cout << left
                     << setw(8)  << bookList[i].bookID
                     << setw(40) << title
                     << setw(20) << author
                     << setw(12) << bookList[i].category;

                if (isAdmin)
                    cout << setw(8) << bookList[i].stock;

                cout << bookList[i].status << endl;
            }
        }

        if (!anyFound)
            cout << "  No books found matching \"" << keyword << "\"." << endl;

        cout << endl;

    } else if (searchChoice == 2) {
        selectionSort(bookList, size);

        cout << "Enter Book ID to search: ";
        string targetID;
        cin >> targetID;

        int result = binarySearch(bookList, size, targetID);

        if (result != -1) {
            cout << "\n-- Book Found --" << endl;
            cout << "ID       : " << bookList[result].bookID    << endl;
            cout << "Title    : " << bookList[result].title     << endl;
            cout << "Author   : " << bookList[result].author    << endl;
            cout << "Category : " << bookList[result].category  << endl;
            cout << "Year     : " << bookList[result].year      << endl;

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

void sortBooks() {

    int size = 0;
    Book* bookList = loadBooks(size);

    if (size == 0 || bookList == nullptr) {
        cout << "\nNo books to sort." << endl;
        return;
    }

    cout << "\n== Sort Books ==" << endl;
    cout << "1. Sort by Title A to Z  (Bubble Sort)" << endl;
    cout << "2. Sort by Book ID       (Selection Sort)" << endl;
    cout << "Enter choice: ";

    int sortChoice;
    cin >> sortChoice;

    if (sortChoice == 1) {
        bubbleSort(bookList, size);
        saveBooks(bookList, size);

        cout << "\nBooks sorted by Title (A to Z)!" << endl;
        cout << "Showing first 10 results:" << endl << endl;

        cout << left
             << setw(8)  << "ID"
             << setw(42) << "Title"
             << setw(20) << "Author"
             << "Status" << endl;
        cout << string(78, '-') << endl;

        int show = (size < 10) ? size : 10;
        for (int i = 0; i < show; i++) {
            string title  = bookList[i].title;
            string author = bookList[i].author;
            if ((int)title.size()  > 40) title  = title.substr(0, 37)  + "...";
            if ((int)author.size() > 18) author = author.substr(0, 15) + "...";

            cout << left
                 << setw(8)  << bookList[i].bookID
                 << setw(42) << title
                 << setw(20) << author
                 << bookList[i].status << endl;
        }
        if (size > 10)
            cout << "  ... and " << size - 10 << " more books. Use Display Books to see all." << endl;

    } else if (sortChoice == 2) {
        selectionSort(bookList, size);
        saveBooks(bookList, size);

        cout << "\nBooks sorted by Book ID!" << endl;
        cout << "Showing first 10 results:" << endl << endl;

        cout << left
             << setw(8)  << "ID"
             << setw(42) << "Title"
             << setw(20) << "Author"
             << "Status" << endl;
        cout << string(78, '-') << endl;

        int show = (size < 10) ? size : 10;
        for (int i = 0; i < show; i++) {
            string title  = bookList[i].title;
            string author = bookList[i].author;
            if ((int)title.size()  > 40) title  = title.substr(0, 37)  + "...";
            if ((int)author.size() > 18) author = author.substr(0, 15) + "...";

            cout << left
                 << setw(8)  << bookList[i].bookID
                 << setw(42) << title
                 << setw(20) << author
                 << bookList[i].status << endl;
        }
        if (size > 10)
            cout << "  ... and " << size - 10 << " more books. Use Display Books to see all." << endl;

    } else {
        cout << "\nInvalid choice." << endl;
    }

    cout << endl;
    delete[] bookList;
}