#include <iostream> 
#include <sqlite3.h> 

//g++ -pedantic-errors ./*.cpp -lsqlite3 -o main

using namespace std; 

static int callback(void* data, int argc, char** argv, char** azColName) 
{ 
	int i; 
	fprintf(stderr, "%s: ", (const char*)data); 

	for (i = 0; i < argc; i++) { 
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); 
	} 

	printf("\n"); 
	return 0; 
} 


int main(int argc, char** argv) 
{ 
	sqlite3* DB; 
	int exit = 0; 
	exit = sqlite3_open("chinook.db", &DB); 
	string data(""); 

    //add the database query here
	string sql(
			"select alb.albumid, alb.title, art.artistid "
			"from albums alb join artists art on alb.artistid = art.artistid;"
		); 
	if (exit) { 
		std::cerr << "Error open DB " << sqlite3_errmsg(DB) << std::endl; 
		return (-1); 
	} 
	else
		std::cout << "Opened Database Successfully!" << std::endl; 

	int rc = sqlite3_exec(DB, sql.c_str(), callback, (void*)data.c_str(), NULL); 
    

    //error handling so we can get some SQL errors
	if (rc != SQLITE_OK) 
		cerr << "Error!  " << sqlite3_str_errcode << endl; 
	else { 
		cout << "Operation OK!" << endl; 
	} 

	sqlite3_close(DB); 
	return (0); 
} 
