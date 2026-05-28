#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
using namespace std;

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