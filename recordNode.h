
class RecordNode{
	char* id;
	char* fn;
	char* ln;
	char* disease;
	char* age;
	char* inDate;
	char* outDate;
	RecordNode* next;
public:
	RecordNode(char*,char*,char*,char*,char*,char*);
	void setNext(RecordNode*);
	RecordNode* getNext();
	char* getInDate();
	char* getOutDate();
	void setOutDate(char*);
	char* getVir();
	char* getId();
	char* getAge();
	char* getNextKeyOfNode(char*);
	void print();
	int getNumOfCharsInPrint();
	bool same(char*,char*,char*,char*);
	bool less(char*,char*);
	~RecordNode();
};
