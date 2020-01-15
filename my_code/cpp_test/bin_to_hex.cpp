#include <iostream>
#include <string>
#include <stdio.h>
#include "hex_utils.h"

using namespace std;

int main(int argc, char *argv[]) {

	if (argc < 2) {
		cout << "usage: bin_to_hex filename\n";
		return 1;
	}

	const char* in_file_name = argv[1];

	FILE* file_in  = fopen(in_file_name, "r");
	if (file_in == nullptr) {
		cerr << "Can't open file '" << in_file_name << "'\n";
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

	string result  = data_to_hex_string(buffer, file_size);
	cout << result << "\n";

	free(buffer);
	fclose(file_in);

	return 0;
}