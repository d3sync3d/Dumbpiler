#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXSIZE 200

char stack[MAXSIZE];
char postfixexp[MAXSIZE];
int stacktop = -1;

void push(char chr){
    if(stacktop<MAXSIZE-1)
        stack[++stacktop] = chr;
}

char pop(){
    if(stacktop>=0)
        return stack[stacktop--];
    return '\0';
}

char peek(){
    if(stacktop>=0)
        return stack[stacktop];
    return '\0';
}

int isEmpty(){
    return stacktop==-1;
}


char* addExplicitConcatenation(const char* input) {
    char* output = malloc(MAXSIZE * sizeof(char));
    int outputIndex = 0;
    int inputLength = strlen(input);

    for (int i = 0; i < inputLength; i++) {
        output[outputIndex++] = input[i];

        // Check for concatenation needed after current character
        if (i < inputLength - 1) {
            char current = input[i];
            char next = input[i + 1];

            // Add '.' between:
            // 1. Closing parenthesis and letter/opening parenthesis
            // 2. Closing '*' and letter/opening parenthesis
            // 3. Letter and letter
            // 4. Letter and opening parenthesis
            // 5. Closing parenthesis and letter

            if ((current == ')' || current == '*') && 
                (next == '(' || 
                 (next >= 'a' && next <= 'z'))) {
                output[outputIndex++] = '.';
            }
            else if ((current >= 'a' && current <= 'z') && 
                     (next == '(' || 
                      (next >= 'a' && next <= 'z'))) {
                output[outputIndex++] = '.';
            }
        }
    }

    output[outputIndex] = '\0';
    return output;
}


int comparePrecedence(char inputchar, char topchar){

    if(topchar=='('){
        return 1;
    } 
    if(inputchar=='*'){
        return (inputchar!=topchar);
    }
    else if(inputchar=='.'){
       return (topchar=='+') ;
    }
    else if(inputchar=='+'){
        return 0;
    }
    
    return 0;
}

void infixtopostfix(const char* infix){
    int postfix_index = 0;
    int infix_len = strlen(infix);
    for(int i=0; i<infix_len; i++){
        char currentchar = infix[i];

        switch(currentchar){
            case '*':
            case '|':
            case '.':
                while(!isEmpty() && !comparePrecedence(currentchar, peek()))
                    postfixexp[postfix_index++] = pop();
                push(currentchar);
                break;
            case '(':
                push(currentchar);
                break;
            case ')':
                while(!isEmpty() && peek() != '(')
                    postfixexp[postfix_index++] = pop();
                if (!isEmpty()) 
                    pop(); 
                //pop();    
                break;
            
            default:
                postfixexp[postfix_index++] = currentchar;
                break;
        }
    }

    while(!isEmpty())
        postfixexp[postfix_index++] = pop();

    postfixexp[postfix_index] = '\0';
}


typedef struct Node{
    int id;
    struct Node* transitionlist[MAXSIZE];
    char transOnInput[MAXSIZE];
    int numtransitions;
} Node;

typedef struct NFA{
    Node* start;
    Node* end;
    int numofStates;
} NFA;

int globalStateCounter = 0;

Node* createState(){
    Node* newstate = (Node*)malloc(sizeof(Node));
    newstate->id = ++globalStateCounter;
    newstate->numtransitions = 0;
    return newstate;
}


NFA* nfaArray[MAXSIZE];
int nfaArrayIndex= -1;

void pushNfa(NFA* nfa){
    nfaArray[++nfaArrayIndex]=nfa;
}

NFA* popNfa(){
    return nfaArray[nfaArrayIndex--];
}

NFA* createBasicNFA(char inp){
    Node* firstState = createState();
    Node* secondState = createState();
    firstState->transitionlist[0] = secondState;
    firstState->transOnInput[0] = inp;
    firstState->numtransitions++;
    NFA* basicNfa = (NFA*)malloc(sizeof(NFA));
    basicNfa->start = firstState;
    basicNfa->end = secondState;
    basicNfa->numofStates = 2;

    return basicNfa;
}

NFA* concatenateNFA(NFA* nfa1, NFA* nfa2){
    
    NFA* concatenatedNFA = (NFA*)malloc(sizeof(NFA));
    nfa1->end->transitionlist[nfa1->end->numtransitions] = nfa2->start;
    nfa1->end->transOnInput[nfa2->end->numtransitions] = 'e';
    nfa1->end->numtransitions++;
    
    concatenatedNFA->start = nfa1->start;
    concatenatedNFA->end = nfa2->end;
    concatenatedNFA->numofStates = nfa1->numofStates+nfa2->numofStates;
    
    //NFA concatenatedNFA = {.start = nfa1->start, .end = nfa2->end, .numofStates = nfa1->numofStates+nfa2->numofStates};
    return concatenatedNFA;
}

NFA* orNFA(NFA* nfa1, NFA* nfa2){
    printf("nfa1:\ntranstiions: %d, start: %d, end: %d\n", nfa1->numofStates, nfa1->start->id, nfa1->end->id);
    NFA* ornfa = (NFA*)malloc(sizeof(NFA));
    Node* startnode = createState();
    Node* endnode = createState();
    startnode->transitionlist[startnode->numtransitions]=nfa1->start;
    startnode->transOnInput[startnode->numtransitions] = 'e';
    startnode->numtransitions++;
    startnode->transitionlist[startnode->numtransitions]=nfa2->start;
    startnode->transOnInput[startnode->numtransitions] = 'e';
    startnode->numtransitions++;
    nfa1->end->transitionlist[nfa1->end->numtransitions] = endnode; 
    nfa1->end->transOnInput[nfa1->end->numtransitions] = 'e';
    nfa1->end->numtransitions++;
    nfa2->end->transitionlist[nfa2->end->numtransitions] = endnode; 
    nfa2->end->transOnInput[nfa2->end->numtransitions] = 'e';
    nfa2->end->numtransitions++;
    ornfa->start = startnode;
    ornfa->end = endnode;
    ornfa->numofStates = nfa1->numofStates+nfa2->numofStates+2;
    return ornfa;
}

NFA* zeroOrMore(NFA* nfa){

    NFA* newNFA = (NFA*)malloc(sizeof(NFA));
    Node* startnode = createState();
    Node* endnode = createState();
    startnode->transitionlist[startnode->numtransitions] = nfa->start;
    startnode->transOnInput[startnode->numtransitions] = 'e';
    startnode->numtransitions++;


    nfa->end->transitionlist[nfa->end->numtransitions] = nfa->start;
    nfa->end->transOnInput[nfa->end->numtransitions] = 'e';
    nfa->end->numtransitions++;
    
    nfa->end->transitionlist[nfa->end->numtransitions] = endnode;
    nfa->end->transOnInput[nfa->end->numtransitions] = 'e';
    nfa->end->numtransitions++;

    startnode->transitionlist[startnode->numtransitions] = endnode;
    startnode->transOnInput[startnode->numtransitions] = 'e';
    startnode->numtransitions++;

    newNFA->start = startnode;
    newNFA->end = endnode;
    newNFA->numofStates = nfa->numofStates+2;

    return newNFA;
}

NFA* oneOrMore(NFA* nfa){
    NFA* newNfa = (NFA*)malloc(sizeof(NFA));
    Node* startnode = createState();
    Node* endnode = createState();

    startnode->transitionlist[startnode->numtransitions] = nfa->start;
    startnode->transOnInput[startnode->numtransitions] = 'e';
    startnode->numtransitions++;

    nfa->end->transitionlist[nfa->end->numtransitions] = endnode;
    nfa->end->transOnInput[nfa->end->numtransitions] = 'e';
    nfa->end->numtransitions++;

    endnode->transitionlist[endnode->numtransitions] = nfa->start;
    endnode->transOnInput[endnode->numtransitions] = 'e';
    endnode->numtransitions++;

    newNfa->start = startnode;
    newNfa->end = endnode;
    newNfa->numofStates = nfa->numofStates+2;
    
    return newNfa;
}

NFA* zeroOrOne(NFA* nfa){
    NFA* newnfa = (NFA*)malloc(sizeof(NFA));
    Node* startnode = createState();
    Node* endnode = createState();

    startnode->transitionlist[startnode->numtransitions] = nfa->start;
    startnode->transOnInput[startnode->numtransitions] = 'e';
    startnode->numtransitions++;

    startnode->transitionlist[startnode->numtransitions] = endnode;
    startnode->transOnInput[startnode->numtransitions] = 'e';
    startnode->numtransitions++;

    nfa->end->transitionlist[nfa->end->numtransitions] = endnode;
    nfa->end->transOnInput[nfa->end->numtransitions] = 'e';
    nfa->end->numtransitions++;

    newnfa->start = startnode;
    newnfa->end = endnode;
    newnfa->numofStates = nfa->numofStates+2;

    return newnfa;
}

void printNFA(NFA* nfa) {
    if (nfa == NULL) {
        printf("NFA is NULL.\n");
        return;
    }

    printf("NFA Graph:\n");

    // To track visited states and avoid printing the same state multiple times
    int visited[MAXSIZE] = {0};

    // Queue for BFS traversal
    Node* queue[MAXSIZE];
    int front = 0, rear = 0;

    // Start BFS from the initial state
    queue[rear++] = nfa->start;

    while (front < rear) {
        Node* currentState = queue[front++];
        
        // Skip already visited states
        if (visited[currentState->id]) {
            continue;
        }

        // Mark the current state as visited
        visited[currentState->id] = 1;

        // Print the current state and its transitions
        printf("State %d: ", currentState->id);
        for (int i = 0; i < currentState->numtransitions; i++) {
            if (currentState->transOnInput[i] == 'e') {
                printf("On ɛ -> State %d; ", currentState->transitionlist[i]->id);
            } else {
                printf("On input '%c' -> State %d; ", currentState->transOnInput[i], currentState->transitionlist[i]->id);
            }

            // Add the next state to the queue if it's not already visited
            if (!visited[currentState->transitionlist[i]->id]) {
                queue[rear++] = currentState->transitionlist[i];
            }
        }
        printf("\n");
    }
}

void writeNFAToDot(NFA* nfa, const char* filename) {
    if (nfa == NULL || nfa->start == NULL) {
        printf("Invalid NFA.\n");
        return;
    }

    FILE* dotfile = fopen(filename, "w");
    if (dotfile == NULL) {
        printf("Error opening file.\n");
        return;
    }

    fprintf(dotfile, "digraph NFA {\n");
    fprintf(dotfile, "    rankdir=LR;\n");  // Horizontal layout
    fprintf(dotfile, "    node [shape=circle];\n");
    
    // Track visited states to avoid duplicate entries
    int visited[MAXSIZE] = {0};
    
    // Queue for BFS traversal
    Node* queue[MAXSIZE];
    int front = 0, rear = 0;
    
    // Start BFS from initial state
    queue[rear++] = nfa->start;
    
    while (front < rear) {
        Node* currentState = queue[front++];
        
        // Skip already visited states
        if (visited[currentState->id]) {
            continue;
        }
        
        // Mark current state as visited
        visited[currentState->id] = 1;
        
        // Add transitions for current state
        for (int i = 0; i < currentState->numtransitions; i++) {
            char* label = (currentState->transOnInput[i] == 'e') ? "ε" : 
                          (char[]){currentState->transOnInput[i], '\0'};
            
            fprintf(dotfile, "    %d -> %d [label=\"%s\"];\n", 
                    currentState->id, 
                    currentState->transitionlist[i]->id, 
                    label);
            
            // Add unvisited states to queue
            if (!visited[currentState->transitionlist[i]->id]) {
                queue[rear++] = currentState->transitionlist[i];
            }
        }
    }
    
    // End the DOT file
    fprintf(dotfile, "}\n");
    fclose(dotfile);
    
    printf("NFA graph written to %s\n", filename);

    system("dot -Tpng hello -o hello.png");
}



NFA* createNFA(char* postfix){
    char pflen = strlen(postfix);
    for(int i=0; i<pflen; i++){
        char curchar= postfix[i];
        switch(curchar){
            case '.':
                NFA* nfa2 = popNfa();
                NFA* nfa1 = popNfa();
                NFA* conNFA = concatenateNFA(nfa1, nfa2);
                pushNfa(conNFA);
                break;

            case '|':
                NFA* nfa4 = popNfa();
                NFA* nfa3 = popNfa();
                NFA* ornfa = orNFA(nfa3, nfa4);
                pushNfa(ornfa);
                break;

            case '*':
                NFA* nfa5 = popNfa();
                NFA* zeromoreNFA = zeroOrMore(nfa5);
                pushNfa(zeromoreNFA);
                break;
            default:
                NFA* basicNFA = createBasicNFA(curchar);
                pushNfa(basicNFA);
                break;
        }
    }

    return popNfa();
    
}




// Example usage
int main() {
    globalStateCounter=0;
    char infix[MAXSIZE];
    //char explicitInfix[MAXSIZE];
    printf("Enter infix expression: ");
    scanf("%s", infix);
    char* explicitInfix = addExplicitConcatenation(infix);
    printf("Original: %s\n", infix);
    printf("Converted: %s\n", explicitInfix);
    infixtopostfix(explicitInfix);
    printf("Postfix expression: %s\n", postfixexp);
    
    NFA* finalNFA = createNFA(postfixexp);
    
    printNFA(finalNFA);
    writeNFAToDot(finalNFA, "hello");
    return 0;
}

