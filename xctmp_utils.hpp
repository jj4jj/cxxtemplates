#include <string>
inline int 
_strtrim(std::string & str, const char * trimchar = " \t\r\n"){
	auto begmatch = str.begin();
	int count = 0; //erase first not of trimchar
	while (begmatch != str.end()){ 
		if (strchr(trimchar, *begmatch)){
			++begmatch;
			++count;
		}
		break;
	}
	if (str.begin() < begmatch){
		str.erase(str.begin(), begmatch);
	}
	//
	auto rbegmatch = str.end();
	while (rbegmatch != str.begin()){
		if (strchr(trimchar, *rbegmatch)){
			--rbegmatch;
			++count;
		}
		break;
	}
	if (rbegmatch != str.end()){
		str.erase(rbegmatch + 1, str.end());
	}
	return  count;
}
inline  void
_strreplace(std::string & ts, const std::string & p, const std::string & v){
	auto beg = 0;
	auto fnd = ts.find(p, beg);
	while (fnd != std::string::npos){
		ts.replace(fnd, p.length(), v);
		beg = fnd + v.length();
		fnd = ts.find(p, beg);
	}
}
