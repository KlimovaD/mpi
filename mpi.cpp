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
#include <mpi.h>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp> 
#include <boost/archive/text_iarchive.hpp> 


using namespace std;

int numproc = 1;
std::vector<string> vecText;



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
	return std::make_pair(a.first, a.second + b.second);
}

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

map<std::string, int> calculateFreq(int id){
	std::map<std::string, int> map;
	int n = 0;
	int step = vecText.size() / numproc;
	auto tid = id;
	auto from = tid*step;
	auto to = (tid + 1)*step;
	if (tid == numproc - 1)
		to = vecText.size();
	long count = 0;
	for (int i = from; i < to-2; ++i){
		std::string s = vecText[i] + " " + vecText[i+1] + " " + vecText[i+2];
		auto&& it = map.find(s);
		if(it != map.end()){
			++it->second;
			++count;
		}
		else{
			map.insert(std::make_pair(s, 1));
		}
	}
	return map;
}

int main(int argc, char *argv[])
{
	std::vector<float>times;
	auto&& reps = 10;
	for (int rep = 0; rep < reps; ++rep){
		cout << "rep=" << rep << endl;
		setlocale(LC_ALL, "rus");
		string fileName("War_and_piece.txt");
		auto&& num = 2;
		vecText = readFileByWords(fileName);
		std::vector<string> vecGr;
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
		int rank;
		MPI_Init(&argc, &argv);
		MPI_Comm_size(MPI_COMM_WORLD, &numproc);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();
		auto&& map = calculateFreq(rank);
	    std::stringstream ss;
	    MPI_Status st;
		if(rank != 0){
			std::stringstream ss;
			boost::archive::text_oarchive oa(ss); 
			oa & map;
			int size = ss.str().size(); 
			MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Send(const_cast<char*>(ss.str().c_str()), ss.str().size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		}
		else {
			auto genMap = map;
			auto cmap = std::map<std::string, int>();
			for(int i = 0; i <numproc-1; ++i){
				int size;
				MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &st);
				const int buf_size = size;
				char buf[buf_size];
				MPI_Recv(buf, buf_size, MPI_CHAR, st.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
				std::stringstream ss;
				ss.str(buf);
				boost::archive::text_iarchive ia(ss); 
				std::map<std::string, int> recmap;
				ia >> recmap; // No size/range needed
				merge_apply(recmap.begin(), recmap.end(), genMap.begin(), genMap.end(), inserter(cmap, cmap.begin()),
						compare_first<pair<std::string, int> >, sum_pairs<pair<std::string, int> >);
				genMap = cmap;
				cmap.clear();
			}
			end = std::chrono::system_clock::now();
			int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
			int L = 0;
			for(auto&& it:genMap){
				if(it.second == 1) continue;
				L=L+it.second;
			}
			float frequency = ((float)L / (float)genMap.size());
			cout << "N=" << genMap.size() << " L= " << L << " freq= " << frequency << endl;
			vecText.clear();
			unsigned int timeEnd = clock();
			unsigned int workingTime = elapsed_seconds;
			cout<<"workingTime="<<workingTime<<endl;
		}

	}
	MPI_Finalize();
	return 0;
}
