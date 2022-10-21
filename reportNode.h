
class ReportNode{
	char* disease;
	int ztot;
	int ttof;
	int ftos;
	int sp;
	ReportNode* next;
public:
	ReportNode();
	ReportNode(char*,char*);
	void setNext(ReportNode*);
	void setReport(char*);
	ReportNode* getNext();
	char* getNextKeyOfNode(char*);
	char* getReport();
	void print();
	int printTopK(int,int);
	void incr(char*);
	int countD(int);
	~ReportNode();
};
