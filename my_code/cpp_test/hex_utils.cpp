#include "hex_utils.h"

using namespace std;

static const char* char_to_hex(char ch) {
	const char* res = "";
	switch(ch){
		case 0x00: res = "0"; break;
		case 0x01: res = "1"; break;
		case 0x02: res = "2"; break;
		case 0x03: res = "3"; break;
		case 0x04: res = "4"; break;
		case 0x05: res = "5"; break;
		case 0x06: res = "6"; break;
		case 0x07: res = "7"; break;
		case 0x08: res = "8"; break;
		case 0x09: res = "9"; break;
		case 0x0a: res = "a"; break;
		case 0x0b: res = "b"; break;
		case 0x0c: res = "c"; break;
		case 0x0d: res = "d"; break;
		case 0x0e: res = "e"; break;
		case 0x0f: res = "f"; break;
	}

	return res;
}

static const char hex_to_char(char ch) {
	char res = 0; 
	switch(ch){
		case '0': res = 0x0; break;
		case '1': res = 0x1; break;
		case '2': res = 0x2; break;
		case '3': res = 0x3; break;
		case '4': res = 0x4; break;
		case '5': res = 0x5; break;
		case '6': res = 0x6; break;
		case '7': res = 0x7; break;
		case '8': res = 0x8; break;
		case '9': res = 0x9; break;
		case 'a': res = 0xa; break;
		case 'b': res = 0xb; break;
		case 'c': res = 0xc; break;
		case 'd': res = 0xd; break;
		case 'e': res = 0xe; break;
		case 'f': res = 0xf; break;
	}

	return res;
}

static char pair_to_char(char ch_pair[2]) {
	char hi = hex_to_char(ch_pair[0]);
	char lo = hex_to_char(ch_pair[1]);

	char res = ((hi << 4) | lo);
	return res;
}

string data_to_hex_string(char *data, size_t len) {

	string result;
	result.reserve(len * 4);

	for (size_t i = 0; i < len; ++i) {
		char element = data[i];
		char hi = (element & 0xf0) >> 4;
		char lo = (element & 0x0f);

		result += char_to_hex(hi);
		result += char_to_hex(lo);
		if (i % 2 == 1){
			result += " ";
		}

		if (i % 16 == 15){
			result += "\n";
		}
	}

	return result;
}

vector<char> hex_data_to_vector(char *data, size_t len) 
{
	vector<char> res;
	res.reserve(len * 3);

	char ch_pair[2];
	int k = 0;
	for (size_t i = 0; i < len; ++i) {
		char element = data[i];

		if (element == ' ' || element == '\n') {
			k = 0;
			continue;
		}

		ch_pair[k] = element;
		k++;

		if (k==2) {
			char ch = pair_to_char(ch_pair);
			res.push_back(ch);
			k = 0;
		}

	}

	return res;
}






