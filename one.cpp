#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <numeric>
#include <pthread.h>
#include <math.h>
#include <chrono>
#include <map>
#include <set>

using namespace std;

std::vector<string> vecText;


vector<string> readFileByWords(string& fileName)
{
	vector <string> vec;
	ifstream file(fileName.c_str());
	string word;
	while (file >> word){
		vec.push_back(word);
	}
	return vec;
}

std::string deleteZn(std::string& s){
	string ret;
	for (auto&& ch : s){
		if (!iswpunct(ch)){
			ret.push_back(tolower(ch));
		}
	}
	return ret;
}

typedef struct thread_data {
	int index;
	std::map<std::string, int> map;

} thread_data;

map<string, int> calculateFreq(){
	map<string, int> map;
	for (int i = 0; i < vecText.size()-2; ++i){
		std::string s = vecText[i] + " " + vecText[i+1] + " " + vecText[i+2];
		auto&& it = map.find(s);
		if(it != map.end()){
			++it->second;
		}
		else{
			map.insert(std::make_pair(s, 1));
		}
	}
	return map;
}


template<class InputIterT1, class InputIterT2, class OutputIterT, class Comparator, class Func>
OutputIterT merge_apply(
		InputIterT1 first1, InputIterT1 last1,
		InputIterT2 first2, InputIterT2 last2,
		OutputIterT result, Comparator comp, Func func) {
	while (true)
	{
		if (first1 == last1) return std::copy(first2, last2, result);
		if (first2 == last2) return std::copy(first1, last1, result);

		if (comp(*first1, *first2) == 0 ) {
			*result = func(*first1, *first2);
			++first1;
			++first2;
		} else if (comp(*first1, *first2) > 0) {
			*result = *first2;
			++first2;
		} else {
			*result = *first1;
			++first1;
		}
		++result;
	}
}

template<class T>
int compare_first(T a, T b) {
	return a.first.compare(b.first);
}

template<class T>
T sum_pairs(T a, T b) {
	if(a.second == b.second && a.second == 0)
		return std::make_pair(a.first, 1);
	else
		return std::make_pair(a.first, a.second + b.second);
}


int main(int argc, char *argv[])
{
	std::vector<float>times;
	auto&& reps = 20;
	for (int rep = 0; rep < reps; ++rep){
		cout << "rep=" << rep << endl;
		setlocale(LC_ALL, "rus");
		string fileName("War_and_piece.txt");
		auto&& num = 2;
		vecText = readFileByWords(fileName);
		/*filter ---*/
		char c = (int) -30;
		char d = (int) -128;
		char e = (int) -109;
		std::string fi;
		fi.push_back(c);
		fi.push_back(d);
		fi.push_back(e);
		vecText.erase(std::remove_if(vecText.begin(), vecText.end(), [fi](string& word){
			if (word.size() == 1 && !iswalpha((unsigned char)word[0]))
				return true;
			else if(word == fi)
				return true;
			else
				return false; }), vecText.end());
		for (int i = 0; i < vecText.size(); ++i){
			vecText[i] = deleteZn(vecText[i]);
		}
		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();
		auto map = calculateFreq();
		int L=0;
		cout<<"size="<<map.size()<<endl;
		std::pair<string, int> max;
		max.second = 0;
		for(auto&& it:map){
			if(it.second == 1) continue;
			L=L+it.second;
			if(it.second > max.second){
				max.second = it.second;
				max.first = it.first;
			}
		}
		//cout<<"max="<<max.first<<" "<<max.second<<endl;
		end = std::chrono::system_clock::now();
		int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
		float frequency = ((float)L / (float)map.size());
		cout << "N=" << map.size() << " L= " << L << " freq= " << frequency << endl;
		vecText.clear();
		unsigned int workingTime = elapsed_seconds;
		times.push_back(workingTime);
	}
	float m = std::accumulate(times.begin(), times.end(), 0) / reps;
	float disp = 0;
	for (auto&& num : times){
		cout << num << " m= " << m << endl;
		disp += std::pow((num - m), 2);
	}
	disp = disp / (reps - 1);
	float sigma = std::sqrt(disp);
	float t = 1.984;
	float interHigh = m + t*(sigma / (sqrt(reps)));
	float interLow = m - t*(sigma / (sqrt(reps)));

	cout << "M=" << m << endl;
	cout << "disp=" << disp << endl;
	cout << "inter= [" << (long) interLow<<";"<<(long) interHigh <<"]"<< endl;
	return 0;
}
