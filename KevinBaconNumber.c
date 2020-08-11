#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define BUFFER 255

/*  -----------------------------------------
 * |  WRITTEN IN LINUX : BUGRA HAMZA GUNDOG |
 *  -----------------------------------------
 */
typedef struct LinkedList
{
	char *actor;            //LinkedList saves the actors if they're not in the list
	int visited;            // -1 if not visited, some unsigned integer if visited
	struct LinkedList *next;
} LinkedList;

typedef struct Graph
{
	LinkedList *element;    //Graph saves the actors
        char *film;             //The film in which the actors acted together
	struct Graph *adjacent; //Each actor has adjacents
	struct Graph *next;
} Graph;

typedef struct Queue
{
	Graph *element;         //Queue saves a graph node which will be visited
       	struct Queue *next;
} Queue;

extern int errno;       //The error number 

char * getString(FILE*);
Graph * readTextFile(char*);
LinkedList * add2List(LinkedList**, char*);
LinkedList * createElement(char*);
Graph * createGraphNode(LinkedList*);
void add2Graph(Graph**, LinkedList*);
void add2Adjacents(Graph**, LinkedList*, char*);
Graph * findActorNode(char*, Graph**);
void bfs(Graph*, Graph*);
void enqueue(Queue**, Graph*);
Graph * dequeue(Queue**);
void printVisited(Graph*);
int max(Graph*);
void printSteps(Graph*, Graph*);

int main()
{
        FILE *fp;
        Graph *actorGraph;      //The graph which includes actors
        Graph *curr;            //The current node of the graph
        char *fileName;        //The name of the input file 
        char actorName[BUFFER]; //The name of actor

        do
        {
                printf("Enter text file : "); 		        //The text file is asked
                fileName = getString(stdin);		        //	       and taken
                actorGraph = readTextFile(fileName);
        }
        while( actorGraph == NULL );  //Asks until a accesible file is found
        
        curr = findActorNode("Bacon, Kevin", &actorGraph);
        bfs(actorGraph, curr);
        printVisited(actorGraph);
        printf("FORMAT : SURNAME, NAME\n");

        do
        {
                printf("Find Kevin Bacon number for the actor : ");
                scanf("%[^\n]s", actorName);
                curr = findActorNode(actorName, &actorGraph);       //SEGMENTATION FAULT IF CURR IS NULL
                if( !curr || curr -> element -> visited == -1)
                        printf("%s's Kevin Bacon number is infinite\n", actorName);
                else    
                {
                        printf("%s's Kevin Bacon number is %d\n",actorName, curr -> element -> visited);
                        printSteps(actorGraph, curr);
                }
                getchar();
        }while(actorName[0] != '\n');
        
        printf("No input given...\nExitting...\n");
}

char * getString(FILE *fp)
{
        /*
         * getString function takes a string from the user
         *
         * :param fp: The stream the text will be taken
         * :return s: The size of the string is set automatically
         */
        char *s; //String s will be taken from the user
        int index = 0;  //Loop variable
        s = (char*) malloc(sizeof(char));       //Allocation for the text

        do
        {
                s = (char*) realloc(s, (1 + index) * sizeof(char)); //The size of the string increases
                s[index++] = fgetc(fp);
        }
        while(s[index - 1] != '\n' && s[index - 1] != EOF);	//Until newline character or EOF is given
        s[index - 1] = '\0'; //Last character of string is NULL in C

        return s;
}

Graph * readTextFile(char *textName)
{
        /*
         * readTextFile function reads a file line by line
         * 	after reading, line is separated into its parts
         *
         * 	file format:
         * 	film Name (year)/actor surname, actor name/.../actor surname, actor name\n
         *
         * :param textName: the file name from which the actors will obtain
         * :return: graph if file has opened correctly, NULL otherwise
         */

        FILE *fp = fopen(textName, "r");	//File is open with only read mode
        char *line;
        char *filmName;		        //The name of film
        char *actorName;                //The name of actor
        LinkedList *Actors = NULL;      //The Linked List of Actors
        LinkedList *addedActor2List;    //The actor who has been added into the LinkedList
        LinkedList **actorsInFilm;       //The actors in a particular film        
        Graph *graph = NULL;            //The graph to build the connections between the actors
        Graph *addedActor2Graph;        //The actor has been added into the graph
        Graph *loc;     //The address of graph node which obtained from LinkedList
        int index;      //index variable for the list of the actors in the film
        int i;          //Loop variable
        int j;          //Loop variable

        if( !fp )		//if file can't open, fp will be NULL
        {
                fprintf(stderr, "Error %d : %s\n", errno, strerror(errno)); //Error message
                return NULL;
        }
        
        while( (line = getString(fp)) && !feof(fp)  )	//File is read line by line
        {
                filmName = strtok(line, "/");		//Film Name
                actorsInFilm = (LinkedList**) malloc(sizeof(LinkedList*));        //The actors in a film are in this array
                index = 0;

                while( actorName = strtok(NULL, "/") )	//After film name, there exists the actors
                {
                        addedActor2List = add2List(&Actors, actorName);
                        add2Graph(&graph, addedActor2List);
                        actorsInFilm[index++] = addedActor2List;        //The actors who acted in the film together
                        actorsInFilm = (LinkedList**) realloc(actorsInFilm, (1 + index) * sizeof(LinkedList*)); //The size is updated

                }

                for(i = 0; i < index; i++)              //If a film has n actors, then we need n^2 operations to build a network between the actors
                {
                        for(j = 0; j < index; j++)
                        {
                                if(i != j)      //The film actor himself is not added to his own adjacents
                                {
                                        loc = findActorNode(actorsInFilm[i] -> actor, &graph);        //The location of graph node is found
                                        add2Adjacents(&loc, actorsInFilm[j], filmName); //The actors in a particular film is connected to each other
                                        
                                }
                                
                        }                        
                }

        }

        return graph;
}

LinkedList * add2List(LinkedList **head, char *name)
{
        /*
         * add2List function adds actor name into the LinkedList 
         *                                      of the actors in the input file
         *              
         * :param head: The double pointer to the head node of the LinkedList
         * :param name: The name of the actor who will be added
         * :return curr: The LinkedList node of actor name 
         */

        LinkedList *curr = *head;	//So the head node doesn't change
        if( !curr )	//If the list has no elements
        {
                *head = createElement(name);	//new element is created and assign to the head
                return *head;
        }
        while( curr -> next && strcmp(curr -> actor, name) != 0)	//while there exists any element and the film is not added
                curr = curr -> next;

        if( !curr -> next )				//if curr reaches the end of the list
        {
                curr -> next = createElement(name);	//add new element at the end
                return curr -> next;
        }
        //otherwise, the film is already in the list
        return curr;
}

LinkedList * createElement(char *name)
{
        /*
         * createElement function creates LinkedList elements
         *
         * This function is used in add2List function twice
         *
         * :param name: The name of the new created LinkedList node
         * :return newNode: The node with initializations
         */

        LinkedList *newNode = malloc(sizeof(LinkedList));   //New node will be created
        newNode -> actor = (char*) malloc((1 + strlen(name)) * sizeof(char));	//Allocation for the name of new node
        strcpy(newNode -> actor, name);	//The given name is copied to the name of the new node
        newNode -> visited = -1;        //-1 means not visited
        newNode -> next = NULL;
        return newNode;
}

Graph * createGraphNode(LinkedList *element)
{
        /*
         * createGraphNode function creates Graph nodes
         *
         * This function is used in add2Graph function twice
         *
         * :param element: LinkedList element is the element of the graph node
         * :return newGraphNode: New initialized graph node created
         */

	Graph *newGraphNode = (Graph*) malloc(sizeof(Graph));   
	newGraphNode -> element = element;      //The element of the graph is LinkedList element
	newGraphNode -> adjacent = NULL;        //The node has no adjacents
	newGraphNode -> next = NULL;            //The node has no next nodes
	return newGraphNode;
}

void add2Graph(Graph **head, LinkedList *element)
{
        /*
         * add2Graph function adds a LinkedList element into the Graph
         *
         * This function adds the element if it is not in the Graph 
         *
         * :param head: The head node of graph, thanks to double pointer, no need to return head node
         * :param element: The element that will be added into the graph
         * :NO RETURN: 
         */
        
	LinkedList *curr = element;
	Graph *graph = *head;

	if( !graph )                                    //If there is no head node,
	{                                               // then graph == NULL
		*head = createGraphNode(element);       //create a new graph node
		return;                                 //and assign it to the head node
	}

	while( graph -> next && strcmp(graph -> element -> actor, element -> actor) != 0)       //Until it reaches the end of the graph
		graph = graph -> next;                                                          //or the actor is already in the graph
                                                                                
	if( !graph -> next )                                                                    //If it reaches the end,
		graph -> next = createGraphNode(element);                                       //then create a new node,
                                                                                                //and add to the end
}

void add2Adjacents(Graph **node, LinkedList *element, char *film)
{
        /*
         * add2Adjacents function adds a LinkedList node as adjacent to Graph node
         * 
         * This function also adds the mutual film of actors 
         *
         * :param node: The node of graph that we will add an adjacent
         * :param element: LinkedList element which will be added to the graph 
         * :param film: Film name that two actors both acted in
         * :NO RETURN:
         */

        Graph *curr = *node;    //Current node to explore the graph
        Graph *newNode = (Graph*) malloc(sizeof(Graph));        //newNode is created

        while( curr -> adjacent && strcmp(curr -> element -> actor, element -> actor) != 0)     //
            curr = curr -> adjacent;                                                            //

        if( curr -> adjacent )          //If curr has adjacent,
                return;                 //then the actor is already an adjacent

        newNode -> element = element;   //Else LinkedList element is added to the graph
        newNode -> film = (char*) malloc((1 + strlen(film)) * sizeof(char));    //Allocation for the name of the mutual film
        strcpy(newNode -> film, film);                                          //Film film which has beeen taken from the input file is copied to the node
        newNode -> next = NULL;                                                 //The next is initialized as NULL
        newNode -> adjacent = NULL;                                             //The adjacent is initialized as NULL
        curr -> adjacent = newNode;                                             //newNode is added as adjacent of current node curr
}

Graph * findActorNode(char *actorName, Graph **head)
{
        /*
         * finActorNode function finds a Graph node from an actor name
         *
         * This function is used to find the actor in the graph for bfs algorithm 
         *
         * :param actor: The name of the actor
         * :param head: Double pointer to the head node of Graph
         * :return curr: The Graph node of the actor
         */

        Graph *curr = *head;
        
        while(curr && strcmp(curr -> element -> actor, actorName) != 0)         //Until the actor is found,
                curr = curr -> next;                                            //the nodes are checked

        return curr;
}

void bfs(Graph *head, Graph *start)
{
        /*
         * bfs function finds the distance from Kevin Bacon
         *
         * :param head: The head node of graph
         * :param start: The start node of graph
         * :NO RETURN:
         */

	int isVisit;
	Queue *q = NULL;
	Graph *curr = head;
	
	start -> element -> visited = 0;
	enqueue(&q, start);

	while( q )
	{
		curr = dequeue(&q);
		curr = findActorNode(curr -> element -> actor, &head);
		isVisit = curr -> element -> visited;
		while( curr = curr -> adjacent )
		{	
			if(curr -> element -> visited == -1)
			{
				curr -> element -> visited = isVisit + 1;
				enqueue(&q, curr);
			}
		}
	}
}

void enqueue(Queue **q, Graph *element)
{
        /*
         * enqueue function enqueues a  graph node to the queue
         *
         * In bfs function, the adjacents are added to the queue 
         *
         * :param q: 
         * :param element:
         * :NO RETURN:
         */

	Queue *newNode = (Queue*) malloc(sizeof(Queue));
	Queue *curr = *q;
	
	if( !curr )
	{
	        newNode -> element = element;
        	newNode -> next = NULL;
	        *q = newNode;
		return;
	}

	while( curr -> next )
	       	curr = curr -> next;

	newNode -> element = element;
	newNode -> next = NULL;
	curr -> next = newNode;
}

Graph * dequeue(Queue **q)
{
        /*
         * dequeue function dequeues a node from the queue
         *
         * When an element dequeues from the queue, return value is the element
         *
         * :param q: The head node of Queue
         * :return g: The Graph g dequeues from the Queue
         */

	Graph *g = (*q) -> element;
	(*q) = (*q) -> next;
	return g;
}

void printVisited(Graph *actors)
{
        /*
         * visiteds function prints the numbers of visited
         *
         * :param actors: LinkedList of actors has visited numbers
         * :NO RETURN:
         */

        Graph *curr;      //Current node curr is used to explore LinkedList actors  
        int maxNum = max(actors);
        int i;  //Loop variable  
        int count;      //Count the actors

        printf("The actors with Kevin Bacon number of 0 : 1\n");
        for(i = 1; i <= maxNum; i++)
        {
                count = 0;
                curr = actors;
                while( curr )
                {
                        if(curr -> element -> visited == i)
                                count++;
                        curr = curr -> next;
                }
                printf("The actors with Kevin Bacon number of %d : %d\n", i, count);
        }
        
        curr = actors;
        count = 0;
        while( curr )
        {
                if(curr -> element -> visited == -1)
                        count++;
                curr = curr -> next;
        }
        printf("The actors with Kevin Bacon number of infinite : %d\n", count);
}   

int max(Graph *graph)
{
        /*
         * max function searches for the maximum number in a graph
         *
         * :param graph: Graph which the number is searched in
         * :return max: The maximum number in the graph
         */

        Graph *curr = graph -> next;            //Current node curr 
        int max = graph -> element -> visited;  //maximum element

        while( curr )
        {
                if(curr -> element -> visited > max)
                        max = curr -> element -> visited;
                curr = curr -> next;
        }

        return max;
}

void printSteps(Graph *head, Graph *start)
{
        /*
         * printSteps function prints out the relation with Kevin Bacon 
         *
         * :param start: The Graph node of actor whose relation is searched
         * :NO RETURN:
         */
        
        Graph *curr = head;
        Graph *currA = start;
        Graph *currB;
        int val;
        
        while( strcmp(currA -> element -> actor, "Bacon, Kevin") != 0 )
        {
                val = 0;
                currB = currA;

                while( currB -> element -> visited != currA -> element -> visited + 1)
                        currA = currA -> adjacent;
                printf("%s - %s : %s\n", currB -> element -> actor, currA -> element -> actor, currA -> film);
                currA = findActorNode(currA -> element -> actor, &head);
        }

        
}

