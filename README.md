# TDS4223 - Group 8 - Library Book Records System

A console-based Library Book Records System written in C++.
The system has two modules: Admin and Customer.

## How to Run the Program

1. Open the project folder.
2. Open `Project.cpp` using Dev-C++.
3. Click Compile & Run
4. Double click the `Project.exe` to run the code.
5. Make sure all text files are in the same folder :
   - Admin.txt
   - Customer.txt
   - Books.txt
   - BorrowRecords.txt
   - Reservations.txt (created automatically)
   - Report.txt (created after generating a report)

## How to Login

### Admin
- Username: admin
- Password: abc123

### Customer
- Username: Test
- Password: abc123

You can also register a new customer account by choosing
"Customer Register" in the main menu.

## Main Menu Options

1. Customer Login
2. Customer Register
3. Admin Login
4. Help / About
0. Exit

## What Each User Can Do

- Customer: borrow, return, renew and reserve books, search and view
  books, check borrow history and fines, and manage their own profile.
- Admin: add, edit, delete and sort books, manage customer and admin
  accounts, handle borrow records and reservations, and generate reports.

## Notes
- Type 0 at any menu to go back or cancel.
- All records are saved into text files, so the data stays even after
  closing the program.
