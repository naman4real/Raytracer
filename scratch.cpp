#include<iostream>
#include<vector>
using namespace std;
class scratch{
    public:
    int x1,y1;
    scratch(int x,int y){
        x1=x;
        y1=y;
    }


    void display(){
        cout<<"x:"<<x1<<" y:"<<y1<<endl;
    }
    scratch* get(scratch obj){
        return new scratch(x1+obj.x1,y1+obj.y1);

    }
};
int main(){
    // scratch *s = new scratch(2,3);
    // s->display();
    // scratch t(10,10);
    // scratch *newObj = s->get(t);
    // cout<<"new "<<newObj->x1<<newObj->y1;
    vector<int> vc(5,3);
    for(auto ii:vc){
        cout<<ii<<endl;
    }
    return 0;
}