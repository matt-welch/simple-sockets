#include "request.hpp" // structure for client requests
#include <map>			// STL map for storing client connection data
#include <vector>		// STL vector for storing client data
#include <string>

using std::vector;
using std::map;
using std::string;

// client_data is a vector of the last few strings that the server has updated 
typedef vector< string > client_data_t;

// client_table is a map of the client key to a vector of strings
typedef map< string, client_data_t > client_table_t;


