#ifndef STRINGNODE_H
#define STRINGNODE_H

class StringNode{
	char* str;
	StringNode* next;
public:
	StringNode(char*);
	void setNext(StringNode*);
	void setString(char*);
	StringNode* getNext();
	char* getNextKeyOfNode(char*);
	char* getString();
	void print();
	static bool less(StringNode*,StringNode*);
	static void swap(StringNode*,StringNode*);
	~StringNode();
};

#endif