#include "request.hpp" // structure for client requests
#include <map>			// STL map for storing client connection data
#include <vector>		// STL vector for storing client data
#include <string>

using std::vector;
using std::map;
using std::string;

typedef vector< request_t > client_data_t;

typedef map< string, client_data_t > client_table_t;


