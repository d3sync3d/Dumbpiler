#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>

#define MAXSIZE 200


// . -> concatenation
// + -> or
// * -> kleen (0 or more)


//infix to postfix conversion
char stack[MAXSIZE];
char postfixexp[MAXSIZE];
int stackTop =0, postfixindex=0;

void postfixpush(char input){

    postfixexp[postfixindex]=input;
    postfixindex++;
}

void push(char input){
    stack[stackTop]=input;
    stackTop++;
}

char pop(){
    stackTop--;
    return stack[stackTop];
}

//priority (higher to lower) : * > . > +
bool comparePrecedence(char inputChar, char topChar){ //return true if higher precedence else false
    if(inputChar=='*'){
        if(inputChar!=topChar) return true;
        else return false;
    }
    else if(inputChar=='.'){
        if(topChar!='*' || topChar!='.') return true;
        else return false;
    }
    else if(inputChar=='+'){
        if(topChar!='*'||topChar!='.'||topChar!='+') return true;
        else return false;
    }
    else
        return false;
    
}

void infixToPostfix(std::string infixexp){
    
    for(int i=0; i<infixexp.length(); i++){
        char curchar=infixexp[i];
        switch(curchar){
            case '*':
            case '+':
            case '.':
                if(stackTop==0){
                    push(curchar);
                }
                else if(comparePrecedence(curchar, stack[stackTop-1])){
                    push(curchar);
                }
                else{
                    postfixpush(pop());
                }
                break;
            case '(':
                push(curchar);
                break;
            case ')':
                postfixpush(pop());
                pop();
                break;
            default:
                postfixpush(curchar);
                break;
        }
    }

    while(stackTop!=0){
        postfixpush(pop());
    }

    
}

//postfix to nfa

int glob_stateNum = 0;


class Edge{
    public:
    std::string transitionOnInput;
    int destinationId;
    
    Edge(std::string input, int destId):
        transitionOnInput(input), destinationId(destId){
    }
};

class State{
    public:

    int stateid;
    std::string stateName;
    std::vector<Edge> outEdges;

    State(){
        this->stateid = glob_stateNum;
        this->stateName = generateStateName();
        glob_stateNum++;
    }

    int getStateId(){
        return stateid;
    }

    void addEdge(Edge& edge){
        outEdges.push_back(edge);
    }
private:
    std::string result;
    std::string generateStateName(){
       if(stateid<10){
            result = "S0"+std::to_string(stateid); 
       } 
       else{
            result = "S"+std::to_string(stateid);
       }
       return result;
   } 
};


std::string getStateName(int id){
    std::string result;
    if(id<10){
        result = "S0"+std::to_string(id); 
    } 
    else{
        result = "S"+std::to_string(id);
    }
    return result;
} 

class NFA{
    public:
    State start;
    State accept;
    std::vector<State> innerStates;

    NFA(const State& initial, const State& final, std::vector<State>&inStates):
        start(initial), accept(final), innerStates(inStates){

    }

    void updateInnerStates(){
        innerStates[0] = start;
        innerStates[1]=accept;
    }
};


std::vector<NFA> NfaStack;

void nfaStack_push(NFA& pushNFA){
    NfaStack.push_back(pushNFA);
}

NFA nfaStack_pop(){
    NFA retNfa = NfaStack.back();
    NfaStack.pop_back();
    return retNfa;
}

NFA OrNFA(){
    NFA nfa2 = nfaStack_pop();
    NFA nfa1 = nfaStack_pop();

    State state1;
    State state2;

    Edge a1 =  Edge("ε", nfa1.start.getStateId());
    Edge a2 = Edge("ε", nfa2.start.getStateId());

    Edge b1 = Edge("ε", state2.getStateId());
    //Edge b2 = Edge("ε", state2.getStateId());

    state1.addEdge(a1);
    state1.addEdge(a2);

    nfa1.accept.addEdge(b1);
    nfa2.accept.addEdge(b1);

    nfa1.updateInnerStates();
    nfa2.updateInnerStates();

    std::vector<State> temp;
    temp.push_back(state1);
    temp.push_back(state2);
    temp.insert(temp.end(), nfa1.innerStates.begin(), nfa1.innerStates.end());
    temp.insert(temp.end(), nfa2.innerStates.begin(), nfa2.innerStates.end()); 
    
    NFA resultNFA(state1, state2, temp);
    nfaStack_push(resultNFA);
    return resultNFA;
}

NFA ZeroAndMoreNFA(){
    NFA nfa = nfaStack_pop();

    State state1;
    State state2;
    Edge edge1 = Edge("ε", state2.getStateId());
    Edge edge2 = Edge("ε", nfa.start.getStateId());
    Edge edge3 = Edge("ε", state1.getStateId());

    state1.addEdge(edge1);
    state1.addEdge(edge2);
    nfa.accept.addEdge(edge1);
    nfa.accept.addEdge(edge2);

    nfa.updateInnerStates();

    std::vector<State> temp;
    temp.push_back(state1);
    temp.push_back(state2);

    temp.insert(temp.end(), nfa.innerStates.begin(), nfa.innerStates.end());

    NFA resultNFA(state1, state2, temp);
    nfaStack_push(resultNFA);
    return resultNFA;
}

NFA ConcatenateNFA(){
    NFA nfa2 = nfaStack_pop();
    NFA nfa1 = nfaStack_pop();

    Edge tempEdge = Edge("ε", nfa2.start.getStateId());
    nfa1.accept.addEdge(tempEdge);
    nfa1.updateInnerStates();


    std::vector<State> temp;
    
    temp.insert(temp.end(), nfa1.innerStates.begin(), nfa1.innerStates.end());
    temp.insert(temp.end(), nfa2.innerStates.begin(), nfa2.innerStates.end());


    NFA resultNFA(nfa1.start, nfa2.accept, temp);
    nfaStack_push(resultNFA);
    return resultNFA;
}

NFA CreateNfa(std::string transitionOnInput){
    State state1;
    State state2;
    Edge edgeObj = Edge(transitionOnInput, state2.getStateId());
    state1.addEdge(edgeObj);
    std::vector<State> temp;
    temp.push_back(state1);
    temp.push_back(state2);
    NFA newNFA(state1, state2, temp);
    nfaStack_push(newNFA);

    return newNFA;

}
//nfa to dfa
//print fa
void printNFA(NFA& nfa){
    std::ofstream outFile("temp.dot");
    outFile<<"digraph G{\n";
    outFile<<"bgcolor=\"transparent\";\n";
    outFile<<"node [color=\"white\"];\n";
    outFile<<"edge [color=\"white\"];\n";
    outFile<<"rankdir=\"LR\";\n";
    outFile<<nfa.accept.stateName<<" [shape=\"doublecircle\"];\n";
    for(int i=0; i<nfa.innerStates.size(); i++){
        outFile<<nfa.innerStates[i].stateName<<" [label=\"\"];\n";
        for(int j=0; j<nfa.innerStates[i].outEdges.size(); j++){
            outFile<<nfa.innerStates[i].stateName<<" -> "<<getStateName(nfa.innerStates[i].outEdges[j].destinationId)<<
                " [label=\""<<nfa.innerStates[i].outEdges[j].transitionOnInput<<"\" fontcolor=\"white\"];\n";
        }
    }

    outFile<<"}\n";
    outFile.close();

    system("dot -Tsvg temp.dot -o NFA.svg");
    system("rsvg-convert NFA.svg > NFA.png");
}
//main
int main(){
    std::string inputinfix;
    std::cout<<"Enter Regular Expression: ";
    std::cin>>inputinfix;

    infixToPostfix(inputinfix);

    std::cout<<"Postfix notation: "<<postfixexp<<std::endl;


    for(int i=0; i<postfixindex;i++){
        if(postfixexp[i]=='.'){
            NFA myNFA = ConcatenateNFA();
            printNFA(myNFA);
        }
        else if(postfixexp[i]=='+'){
            NFA myNFA = OrNFA();
            printNFA(myNFA);
        }
        else if(postfixexp[i]=='*'){
            NFA myNFA = ZeroAndMoreNFA();
            printNFA(myNFA);
        }
        else{
            CreateNfa(std::string(1, postfixexp[i]));
        }
    }
    return 0;
}
