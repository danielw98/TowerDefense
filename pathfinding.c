#include "pathfinding.h"
#include <stdio.h>
#include <string.h>

extern int map_path[NUM_VERTICES];


void TD_Pathfinding(int matrix[NUM_NODES][NUM_NODES], int n, int startnode, int endnode)
{
    int graph[NUM_VERTICES][NUM_VERTICES];
    int cost[NUM_VERTICES][NUM_VERTICES], distance[NUM_VERTICES], pred[NUM_VERTICES];
    int visited[NUM_VERTICES], count, mindistance, nextnode, i, j, pos = 0;

    TD_Matrix_to_Graph(matrix, graph);

    // create cost matrix
    for(i = 0; i < n; i++)
        for(j = 0; j < n; j++)
            if(graph[i][j] == 0)
                cost[i][j] = INFINITY;
            else
                cost[i][j] = graph[i][j];
    
    // init auxiliary arrays used in the algorithm
    for(i = 0; i < n; i++)
    {
        distance[i] = cost[startnode][i];
        pred[i] = startnode;
        visited[i] = 0;
    }

    // apply Dijkstra's algorithm
    distance[startnode] = 0;
    visited[startnode] = 1;
    count = 1;
    while(count < n-1)
    {
        mindistance = INFINITY;
        for(i = 0; i < n; i++)
            if(distance[i] < mindistance && !visited[i])
            {
                mindistance = distance[i];
                nextnode = i;
            }
        
        visited[nextnode] = 1;
        for(i = 0; i < n; i++)
            if(!visited[i])
                if(mindistance + cost[nextnode][i] < distance[i])
                {
                    distance[i] = mindistance + cost[nextnode][i];
                    pred[i] = nextnode;
                }
        count++;
    }
    
    // save the path into the array
    j = endnode;
    do
    {
        j = pred[j];
        map_path[pos++] = j;
    }while(j != startnode);

    // print path for own sanity
    for(i = 0; i < pos; i++)
        printf("%d ", map_path[pos]);
    printf("\n");
}

void TD_Matrix_to_Graph(int matrix[NUM_NODES][NUM_NODES], int graph[NUM_VERTICES][NUM_VERTICES])
{
    memset(graph, 0, NUM_VERTICES * NUM_VERTICES * sizeof(int));
    for(int i = 0; i < NUM_NODES; i++)
    {
        for(int j = 0; j < NUM_NODES; j++)
        {
            int elem_i = NUM_NODES * i + j;
            int elem_j;

            if(matrix[i][j] != 0)
                continue;

            if(i + 1 < NUM_NODES)
            {
                elem_j = (i + 1) * NUM_NODES + j;
                if(matrix[i+1][j] == 0)
                    graph[elem_i][elem_j] = 1;
            }
            if(i - 1 >= 0)
            {
                elem_j = (i - 1) * NUM_NODES + j;
                if(matrix[i-1][j] == 0)
                    graph[elem_i][elem_j] = 1;
            }
            if(j + 1 < NUM_NODES)
            {
                elem_j = i * NUM_NODES + j + 1;
                if(matrix[i][j+1] == 0)
                    graph[elem_i][elem_j] = 1;
            }
            if(j - 1 >= 0)
            {
                elem_j = i * NUM_NODES + j - 1;
                if(matrix[i][j-1] == 0)
                    graph[elem_i][elem_j] = 1;
            }
        }
        
    }
}