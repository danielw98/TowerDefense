#ifndef _PATHFINDING_H
#define _PATHFINDING_H

#define NUM_NODES 10
#define NUM_VERTICES (NUM_NODES * NUM_NODES)

#define INFINITY 9999
 

void TD_Pathfinding(int matrix[NUM_NODES][NUM_NODES], int n, int startnode, int endnode);
void TD_Matrix_to_Graph(int matrix[NUM_NODES][NUM_NODES], int graph[NUM_VERTICES][NUM_VERTICES]);
void TD_howtocallfunc(int matrix[NUM_NODES][NUM_NODES])
{
    int startnode = 50, endnode = 59, n = NUM_VERTICES;
    TD_Pathfinding(matrix, n, startnode, endnode);
}
#endif // _PATHFINDING_H