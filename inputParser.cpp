#include <cstdlib>
#include <cstring>

int inputParserM(char** a,int* b,int* c,char** e,int* f,int inNum, char** in){  //returns 0 upon success and 1 upon failure
	int flag0 = 0, flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0;
	if (inNum != 11)
		return 1;
	for(int i=1; i<inNum; i++){
		if(in[i][0] == '-'){   //if it's a flag
			if (!strcmp(in[i],"-i")){
				if(in[i+1][0] != '-'){
					*a = (char*)malloc((strlen(in[i+1])+1)*sizeof(char));
					strcpy(*a,in[i+1]);
				}
				else
					return 1;
				if(!flag0)
					flag0++;
			}
			else if (!strcmp(in[i],"-w")){
                if(!(*b = std::atoi(in[i+1])))
					return 1;
				if(!flag1)
					flag1++;
			}
			else if (!strcmp(in[i],"-b")){
                if(!(*c = std::atoi(in[i+1])))
					return 1;
				if(!flag2)
					flag2++;
			}
			else if (!strcmp(in[i],"-s")){
				if(in[i+1][0] != '-'){
					*e = (char*)malloc((strlen(in[i+1])+1)*sizeof(char));
					strcpy(*e,in[i+1]);
				}
				else
					return 1;
				if(!flag3)
					flag3++;
			}
			else if (!strcmp(in[i],"-p")){
                if(!(*f = std::atoi(in[i+1])))
					return 1;
				if(!flag4)
					flag4++;
			}
			else
				return 1;
		}
	}
	if (flag0 + flag1 + flag2 + flag3 + flag4 != 5)
		return 1;
	return 0;
}

int inputParserS(int* b,int* c,int* e,int* f,int inNum, char** in){  //returns 0 upon success and 1 upon failure
	int flag0 = 0, flag1 = 0, flag2 = 0, flag3 = 0;
	if (inNum != 9)
		return 1;
	for(int i=1; i<inNum; i++){
		if(in[i][0] == '-'){   //if it's a flag
			if (!strcmp(in[i],"-q")){
                if(!(*b = std::atoi(in[i+1])))
					return 1;
				if(!flag0)
					flag0++;
			}
			else if (!strcmp(in[i],"-s")){
                if(!(*c = std::atoi(in[i+1])))
					return 1;
				if(!flag1)
					flag1++;
			}
			else if (!strcmp(in[i],"-w")){
                if(!(*e = std::atoi(in[i+1])))
					return 1;
				if(!flag2)
					flag2++;
			}
			else if (!strcmp(in[i],"-b")){
                if(!(*f = std::atoi(in[i+1])))
					return 1;
				if(!flag3)
					flag3++;
			}
			else
				return 1;
		}
	}
	if (flag0 + flag1 + flag2 + flag3 != 4)
		return 1;
	return 0;
}

int inputParserC(char** a,int* b,int* c,char** e,int inNum, char** in){  //returns 0 upon success and 1 upon failure
	int flag0 = 0, flag1 = 0, flag2 = 0, flag3 = 0;
	if (inNum != 9)
		return 1;
	for(int i=1; i<inNum; i++){
		if(in[i][0] == '-'){   //if it's a flag
			if (!strcmp(in[i],"-q")){
				if(in[i+1][0] != '-'){
					*a = (char*)malloc((strlen(in[i+1])+1)*sizeof(char));
					strcpy(*a,in[i+1]);
				}
				else
					return 1;
				if(!flag0)
					flag0++;
			}
			else if (!strcmp(in[i],"-w")){
                if(!(*b = std::atoi(in[i+1])))
					return 1;
				if(!flag1)
					flag1++;
			}
			else if (!strcmp(in[i],"-sp")){
                if(!(*c = std::atoi(in[i+1])))
					return 1;
				if(!flag2)
					flag2++;
			}
			else if (!strcmp(in[i],"-sip")){
                if(in[i+1][0] != '-'){
					*e = (char*)malloc((strlen(in[i+1])+1)*sizeof(char));
					strcpy(*e,in[i+1]);
				}
				else
					return 1;
				if(!flag3)
					flag3++;
			}
			else
				return 1;
		}
	}
	if (flag0 + flag1 + flag2 + flag3 != 4)
		return 1;
	return 0;
}
