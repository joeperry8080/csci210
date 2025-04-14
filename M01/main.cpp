#include <iostream>
#include <fstream>
#include <string>
#include <climits>
#include <set>
#include <iomanip>

using namespace std;

int main() 
{
	//The code below will get the current date and format it into ISO8601 date format (yyyy-mm-dd) You can use this to get the date to put into the output file for the survey data.
	char formatDate[80];
	time_t currentDate = time(NULL);
	strftime(formatDate, 80, "%F", localtime(&currentDate)); // for date and time "%F %T"
	string inv_date(formatDate);
	
	return 0;
}




