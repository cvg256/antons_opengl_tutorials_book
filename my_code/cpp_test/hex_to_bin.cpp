#include <iostream>
#include <string>
#include <stdio.h>
#include "hex_utils.h"

using namespace std;

int main(int argc, char *argv[]) {

	if (argc < 3) {
		cout << "usage: hex_to_bin in_file out_file\n";
		return 1;
	}

	const char* in_file_name = argv[1];
	const char* out_file_name = argv[2];

	FILE* file_in  = fopen(in_file_name, "r");
	if (file_in == nullptr) {
		cerr << "Can't open input file '" << in_file_name << "'\n";
		return 1;
	}

	FILE* file_out = fopen(out_file_name, "w");
	if (file_out == nullptr) {
		cerr << "Can't open output file '" << out_file_name << "'\n";
		return 1;
	}


	fseek(file_in, 0L, SEEK_END);
	size_t file_size = ftell(file_in);
	fseek(file_in, 0L, SEEK_SET);

	char* buffer = (char*)malloc(file_size);

	size_t bytes_read = fread(buffer, 1, file_size, file_in);
	if (bytes_read != file_size) {
		cerr << "Can't read file '" << in_file_name << "'\n";
		return 1;
	}

	vector<char> result = hex_data_to_vector(buffer, file_size);
	cout << "out file size: " << result.size() << "\n"; 

	free(buffer);
	buffer = nullptr;

	size_t bytes_written = fwrite(reinterpret_cast<void*>(result.data()), 1, result.size(), file_out); 
	if (bytes_written != result.size()) {
		cerr << "Can't write file '" << out_file_name << "'\n";
		return 1;
	}

	fclose(file_out);
	fclose(file_in);

	return 0;
}