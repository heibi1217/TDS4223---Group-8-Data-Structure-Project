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

struct Book {
    string bookID;
    string title;
    string author;
    string category;
    int year;
    int stock;
    string status; // "Available" or "Borrowed"
};

struct BookRecord {
    string borrowID;
    string customerID;
    string bookID;
    string borrowDate;
    string returnDate;
    string borrowStatus; // "Borrowed" or "Returned"
};

class Account {
    protected:
        string username;
        string password;
        string role; // "Customer" or "Librarian"
    
    public:
        Account(){
            username = "";
            password = "";
            role = "";
        }

        ~Account(){} // Destructor (Run when the object is destroyed)

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

    // Check if username already exists
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

    // Dynamically allocate a new User object
    User* newUser = new User(u, p); // Create a new User with username and password
    ofstream file("Customer.txt", ios::app); // append mode, 加在后面不会覆盖内容
    file << newUser->getUsername() << "|" << newUser->getPassword() << "|Customer" << endl;
    file.close();
    delete newUser; // Free memory after use

    cout << "Customer registered successfully!" << endl << endl;
}

string loginUser(){
    string u, p, fu,fp, frole; // fu = file username, fp = file password, frole = file role
    cout << "\n== Customer Login ==" << endl;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;
    cout << endl;

    ifstream file("Customer.txt");
    // getline(自己取的file name, 读出来的资料存去哪个变量, 遇到什么stop)
    while (getline(file,fu,'|') && getline(file, fp, '|') && getline(file, frole)){
        if(u == fu && p == fp)
        {
            file.close();
            cout << "Login successful! Welcome, " << u << "!" << endl;
            return u; // Return the username of the logged in user
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

void customerMenu(string username){
    int choice;
    do{
        cout << "\n=== Customer Menu ===" << endl;
        cout << "1. Borrow Book" << endl; // Wen Zhe part
        cout << "2. Return Book" << endl; // Wen Zhe part
        cout << "3. Search Book" << endl; // Zhong Bao part
        cout << "4. Display Books" << endl; // Yvonne part
        cout << "5. View Borrowing Summary" << endl; // Tsui Hern part
        cout << "0. Logout" << endl << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch(choice) {
        	case 1:
        		
        		break;
        	
        	case 2:
        		
        		break;
        	
        	case 3:
        		
        		break;
        	
        	case 4:
        		displayBooks();
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
        cout << "1. Add Book" << endl; // Yvonne part
        cout << "2. Edit Book" << endl;// Yvonne part
        cout << "3. Delete Book" << endl;// Yvonne part
        cout << "4. Search Book" << endl; // Zhong Bao part
        cout << "5. Sort Books" << endl; // Zhong Bao part
        cout << "6. Display Books" << endl; // Yvonne part
        cout << "7. Generate Reports" << endl; // Tsui Hern part
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
				
				break;
			
			case 5:
				
				break;
				
			case 6:
				displayBooks();
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
                try{ // Try to login, if failed throw error
                    string loggedIn = loginUser(); 
                    
                    if(loggedIn.empty())
                    {
                        throw "Login failed!"; // Throw error if login failed
                    }
                    customerMenu(loggedIn); // Enter customer menu if login successful

                } catch(const char* e){
                    // Catch the error and display the error message
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
    ofstream file("Books.txt", ios::trunc); // Overwrite mode
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

// 1. Add
void addBook() {
    cout << "\n== Add New Book ==" << endl;
    Book newBook;
    
    cout << "Enter Book ID: "; cin >> newBook.bookID;
    cin.ignore(); // Clear buffer
    cout << "Enter Title: "; getline(cin, newBook.title);
    cout << "Enter Author: "; getline(cin, newBook.author);
    cout << "Enter Category: "; getline(cin, newBook.category);
    cout << "Enter Publication Year: "; cin >> newBook.year;
    cout << "Enter Stock Quantity: "; cin >> newBook.stock;
    
    newBook.status = (newBook.stock > 0) ? "Available" : "Borrowed";

    // Append to file
    ofstream file("Books.txt", ios::app);
    file << newBook.bookID << "|" << newBook.title << "|" << newBook.author << "|"
         << newBook.category << "|" << newBook.year << "|" << newBook.stock << "|" 
         << newBook.status << endl;
    file.close();

    cout << "Book successfully added!" << endl;
}

// 2. Display
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

// 3. Edit
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

// 6. Delete Book
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
        // Create a new array shrinking the size by 1
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
