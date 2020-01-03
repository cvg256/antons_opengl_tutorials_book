#include <iostream>

using namespace std;

template <typename T>
class SimpleVector {
public:
	explicit SimpleVector(size_t size) {
		data_ = new T[size];
		end_ = data_ + size;
	}
	
	~SimpleVector() {
		delete[] data_;
	}
	
	T& operator[] (size_t index) {
		return data_[index];
	}
	
	const T* begin() const{
		return data_;
	}

	const T* end() const {
		return end_;
	}

	T* begin() {
		return data_;
	}
	
	T* end() {
		return end_;
	}

	
private:
	T* data_;
	T* end_;
};

template <typename T>
void Print(const SimpleVector<T>& v) {
	for (const auto& e : v) {
		cout << e << ' ';
	}
}

int main() {
	SimpleVector<int> sv(5);
	
	for (int i = 0; i < 5; ++i) {
		sv[i] = 5 - i;
	}

	Print(sv);
	cout << endl;
	
	for (auto& e : sv) {
		e *= 2;
	}
	Print(sv);
	cout << endl;

	return 0;
}
