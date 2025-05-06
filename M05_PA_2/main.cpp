#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>
#include <sqlite3.h>
#include <functional>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
 
// Template function for displaying paged results
template <typename T, typename DisplayFunc>
int displayPagedResults(const vector<T>& items, const string& prompt, DisplayFunc display) {
	if (items.empty()) {
		cout << "No items found." << endl;
		return -1; // Indicate no selection
	}

	cout << "There are " << items.size() << " rows in the result. How many do you want to see per page?" << endl;
	int pageSize;
	while (true) {
		cout << "";
		cin >> pageSize;
		if (cin.fail() || pageSize <= 0) {
			cin.clear();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			cout << "Invalid input. Please enter a valid positive number." << endl;
		} else {
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			break;
		}
	}

	int currentPage = 0;
	int totalPages = (items.size() + pageSize - 1) / pageSize;
	int selectedIndex = -1;
	bool validSelection = false;

	while (!validSelection) {
		int startIndex = currentPage * pageSize;
		int endIndex = std::min(static_cast<int>(items.size()), startIndex + pageSize); // Use static_cast
		cout << "\n" << prompt;
		if (currentPage == 0)
			cout << "(enter 0 to go to the next page):" << endl;
		else
			cout << "(enter 0 to go to the next page or -1 to go to the previous page):" << endl;

		for (int i = startIndex; i < endIndex; i++) {
			display(items[i], i - startIndex + 1);
		}

		int choice;
		while (true) {
			cin >> choice;
			if (cin.fail()) {
				cin.clear();
				cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				cout << "Invalid input. Please enter a valid number." << endl;
			} else {
				cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				break;
			}
		}

		if (choice == 0) {
			if (currentPage < totalPages - 1)
				currentPage++;
			else
				cout << "This is the last page." << endl;
		} else if (choice == -1) {
			if (currentPage > 0)
				currentPage--;
			else
				cout << "This is the first page." << endl;
		} else if (choice >= 1 && choice <= (endIndex - startIndex)) {
			selectedIndex = startIndex + choice - 1;
			validSelection = true;
		} else {
			cout << "Invalid selection. Try again." << endl;
		}
	}
	return selectedIndex;
}

// menu choice 1
void viewRentalForCustomer(sqlite3* db) {
    // get a list of distinct customers who have rentals
    const char* custQuery = "select c.customer_id, c.first_name || ' ' || c.last_name from customer c join rental r on c.customer_id = r.rental_id order by c.customer_id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, custQuery, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Error preparing customer query: " << sqlite3_errmsg(db) << endl;
        return;
    }
    // store results.
    vector<std::pair<int, string>> customers;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* nameText = sqlite3_column_text(stmt, 1);
        string fullName = nameText ? reinterpret_cast<const char*>(nameText) : "";
        customers.push_back({id, fullName});
    }
    sqlite3_finalize(stmt);

    if (customers.empty()) {
        cout << "No customers with rentals found." << endl;
        return;
    }

    // Use the template function to display paged customers and get the selected customer ID
    auto displayCustomer = [](const std::pair<int, string>& cust, int index) {
        cout << index << ". " << cust.first << " - " << cust.second << endl;
    };
    int selectedCustomerIndex = displayPagedResults(customers, "Choose the customer you want to see rentals for ", displayCustomer);

    if (selectedCustomerIndex == -1) {
        return; // No customer selected
    }
    int selectedCustomerId = customers[selectedCustomerIndex].first;

    // get rental data
    const char* rentalQuery =
        "select r.rental_id, r.rental_date, s.first_name || ' ' || s.last_name, "
        "c.first_name || ' ' || c.last_name, "
        "f.title || ' - ' || f.description || ' $' || f.rental_rate, "
        "r.return_date "
        "from rental r "
        "join staff s on r.staff_id = s.staff_id "
        "join customer c on r.customer_id = c.customer_id "
        "join inventory i on r.inventory_id = i.inventory_id "
        "join film f on i.film_id = f.film_id "
        "where  c.customer_id = ? "
        "order by r.rental_date;";
    sqlite3_stmt* rentalStmt = nullptr;
    if (sqlite3_prepare_v2(db, rentalQuery, -1, &rentalStmt, nullptr) != SQLITE_OK) {
        cout << "Error preparing rental query: " << sqlite3_errmsg(db) << endl;
        return;
    }
    sqlite3_bind_int(rentalStmt, 1, selectedCustomerId);

    // Structure to hold each rental record.
    struct RentalRecord {
        int rental_id;
        string rental_date;
        string staff_name;
        string customer_name;
        string film_info;
        string return_date;
    };

    vector<RentalRecord> rentals;
    while (sqlite3_step(rentalStmt) == SQLITE_ROW) {
        RentalRecord rr;
        rr.rental_id = sqlite3_column_int(rentalStmt, 0);
        rr.rental_date = sqlite3_column_text(rentalStmt, 1) ? reinterpret_cast<const char*>(sqlite3_column_text(rentalStmt, 1)) : "";
        rr.staff_name = sqlite3_column_text(rentalStmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(rentalStmt, 2)) : "";
        rr.customer_name = sqlite3_column_text(rentalStmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(rentalStmt, 3)) : "";
        rr.film_info = sqlite3_column_text(rentalStmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(rentalStmt, 4)) : "";
        rr.return_date = sqlite3_column_text(rentalStmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(rentalStmt, 5)) : "";
        rentals.push_back(rr);
    }
    sqlite3_finalize(rentalStmt);

    if (rentals.empty()) {
        cout << "No rentals found for the selected customer." << endl;
        return;
    }

    // Use the template function to display paged rentals and get the selected rental index
    auto displayRental = [](const RentalRecord& rental, int index) {
        cout << index << ". " << rental.rental_id << " - " << rental.rental_date << endl;
    };
    int selectedRentalIndex = displayPagedResults(rentals, "Choose the rental you want to see ", displayRental);

    if(selectedRentalIndex == -1)
        return;

    // Display the selected rental’s details.
    RentalRecord selectedRental = rentals[selectedRentalIndex];
    cout << "\n**********Rental Date: " << selectedRental.rental_date << "**********" << endl;
    cout << "Staff: " << selectedRental.staff_name << endl;
    cout << "Customer: " << selectedRental.customer_name << endl;
    cout << "Film Information:" << endl;
    cout << selectedRental.film_info << endl;
    cout << "Return Date: " << selectedRental.return_date << endl;
}
 
// menu choice 2
void viewCustomerInformation(sqlite3* db) {
    // get a list of distinct customers in the database
    const char* custQuery = "select c.customer_id, c.first_name || ' ' || c.last_name from customer c order by c.customer_id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, custQuery, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Error preparing customer summary query: " << sqlite3_errmsg(db) << endl;
        return;
    }
    vector<std::pair<int, string>> customers;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* nameText = sqlite3_column_text(stmt, 1);
        string name = nameText ? reinterpret_cast<const char*>(nameText) : "";
        customers.push_back({id, name});
    }
    sqlite3_finalize(stmt);

    if (customers.empty()) {
        cout << "No customers found in the database." << endl;
        return;
    }

    // Use the template function to display paged customers.
    auto displayCustomer = [](const std::pair<int, string>& cust, int index) {
        cout << index << ". " << cust.first << " - " << cust.second << endl;
    };
    int selectedCustomerIndex = displayPagedResults(customers, "Choose the customer you want to see: ", displayCustomer);
    if (selectedCustomerIndex == -1) {
        return; // No customer selected
    }
    int selectedCustomerId = customers[selectedCustomerIndex].first;

    // Query detailed customer information.
    const char* detailQuery =
        "select c.first_name, c.last_name, a.address, a.phone, ct.city, a.postal_code, c.email, c.active, c.last_update "
        "from customer c join address a ON c.address_id = a.address_id "
        "join city ct ON a.city_id = ct.city_id "
        "where c.customer_id = ?;";
    sqlite3_stmt* detailStmt = nullptr;
    if (sqlite3_prepare_v2(db, detailQuery, -1, &detailStmt, nullptr) != SQLITE_OK) {
        cout << "Error preparing customer details query: " << sqlite3_errmsg(db) << endl;
        return;
    }
    sqlite3_bind_int(detailStmt, 1, selectedCustomerId);
    cout << "Executing query for customer ID: " << selectedCustomerId << endl;
    int stepResult = sqlite3_step(detailStmt);
    if (stepResult == SQLITE_ROW) {
        cout << "Customer details found!" << endl;
        string firstName = sqlite3_column_text(detailStmt, 0) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 0)) : "";
        string lastName = sqlite3_column_text(detailStmt, 1) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 1)) : "";
        string address = sqlite3_column_text(detailStmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 2)) : "";
        string phone = sqlite3_column_text(detailStmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 3)) : "";
        string city = sqlite3_column_text(detailStmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 4)) : "";
        string postal = sqlite3_column_text(detailStmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 5)) : "";
        string email = sqlite3_column_text(detailStmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 6)) : "";
        int active = sqlite3_column_int(detailStmt, 7);
        string lastUpdate = sqlite3_column_text(detailStmt, 8) ? reinterpret_cast<const char*>(sqlite3_column_text(detailStmt, 8)) : "";

        cout << "\n**********Customer Information**********" << endl;
        cout << "Name: " << firstName << " " << lastName << endl;
        cout << "Address: " << address << endl;
        cout << city << ", " << postal << endl;
        cout << "Phone Number: " << phone << endl;
        cout << "Email: " << email << endl;
        cout << "Active: " << active << endl;
        cout << "Last Update: " << lastUpdate << endl;
    } else if (stepResult == SQLITE_DONE) {
        cout << "No customer details found for the selected id." << endl;
    }
     else {
        cout << "Error fetching customer details: " << sqlite3_errmsg(db) << endl;
    }
    sqlite3_finalize(detailStmt);
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open("sakila.db", &db) != SQLITE_OK) {
        cout << "Error in connection: unable to open database file: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }
    cout << "Welcome to Sakila" << endl;

    int choice;
    do {
        cout << "\n**********Please choose an option (enter -1 to quit):**********" << endl;
        cout << "1. View the rentals for a customer" << endl;
        cout << "2. View Customer Information" << endl;
        cout << "Enter Choice: ";
        while (true) {
            cin >> choice;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a valid number." << endl;
            } else {
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
            }
        }
        if (choice == 1) {
            viewRentalForCustomer(db);
        } else if (choice == 2) {
            viewCustomerInformation(db);
        } else if (choice != -1) {
            cout << "Invalid option. Please choose a valid menu option." << endl;
        }
    } while (choice != -1);

    sqlite3_close(db);
    cout << "Database closed. Exiting program." << endl;
    return 0;
}