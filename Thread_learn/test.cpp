//#include <iostream>
//#include <string>
//#include "boost/lexical_cast.hpp"
//int main()
//{
//    using namespace std;
//    cout << "Enter your weight: ";
//    float weight;
//    cin >> weight;
//    string gain = "A 10% increase raises ";
//    string wt = boost::lexical_cast<string> (weight);
//    gain = gain + wt + " to ";      // string operator()
//    weight = 1.1 * weight;
//    gain = gain + boost::lexical_cast<string>(weight) + ".";
//    cout << gain << endl;
//    system("pause");
//    return 0;
//}
#include <iostream>
#include <string>
using namespace std;
class test {
public:
	string A;
	bool B;
public:
	test(test&& other) noexcept
	/*�Զ�����ƶ����캯��
	*/
	{
		A = other.A;
		B = other.B;
		cout << A << endl;
	}
	test() {}

};

test Copy(string a, bool b) {
	test temp;
	temp.A = a;
	temp.B = b;
	return temp;
}


int main() {
	test A(Copy("hello", false));
	test BB = std::move(A); //����BB�Ķ���ʹ�����Լ����ƶ����캯��
	return 0;
}