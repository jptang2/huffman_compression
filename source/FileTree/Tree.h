#pragma once
class Tree  
{  
public :  
 
	char ch;  
	int weight;  	

	Tree* left;  
	Tree* right; 

	Tree(){left = right = NULL; weight=0;ch ='\0';}  
	Tree(Tree* l,Tree* r,int w,char c){left = l; right = r;  weight=w;   ch=c;}  	
	~Tree(){delete left; delete right;}  
	bool Isleaf(){return !left && !right; }  
};  


class Compare_tree  
{  
public:  
	bool operator () (Tree* t1, Tree* t2)  
	{  
 		if (t1->weight == t2->weight)
 		{
			return t1->ch > t2->ch;  
		}
		return t1->weight > t2->weight;  
	}  
};  