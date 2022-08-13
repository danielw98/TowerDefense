#include <stdio.h>
#include <string.h>

#define NUM_NODES 10
#define NUM_VERTICES (NUM_NODES * NUM_NODES)

#define INFINITY 9999
#define MAX 10
 
void dijkstra(int G[NUM_VERTICES][NUM_VERTICES], int n, int startnode, int endnode)
{
 
    int cost[NUM_VERTICES][NUM_VERTICES], distance[NUM_VERTICES], pred[NUM_VERTICES];
    int visited[NUM_VERTICES], count, mindistance, nextnode, i, j;
    //pred[] stores the predecessor of each node
    //count gives the number of nodes seen so far
    //create the cost matrix
    for(i = 0; i < n; i++)
        for(j = 0; j < n; j++)
            if(G[i][j] == 0)
                cost[i][j] = INFINITY;
            else
                cost[i][j] = G[i][j];
    //initialize pred[],distance[] and visited[]
    for(i = 0; i < n; i++)
    {
        distance[i] = cost[startnode][i];
        pred[i] = startnode;
        visited[i] = 0;
    }
    distance[startnode] = 0;
    visited[startnode] = 1;
    count = 1;
    while(count < n-1)
    {
        mindistance = INFINITY;
        //nextnode gives the node at minimum distance
        for(i = 0; i < n; i++)
            if(distance[i] < mindistance&&!visited[i])
            {
                mindistance = distance[i];
                nextnode = i;
            }
        //check if a better path exists through nextnode
        visited[nextnode] = 1;
        for(i = 0;i < n;i++)
        if(!visited[i])
        if(mindistance+cost[nextnode][i] < distance[i])
        {
            distance[i] = mindistance + cost[nextnode][i];
            pred[i] = nextnode;
        }
        count++;
    }
    
    //print the path and distance of each node
    printf("\nDistance of node%d=%d", endnode, distance[endnode]);
    printf("\nPath=%d", endnode);
    j = endnode;
    do
    {
        j = pred[j];
        printf("<-%d", j);
    }while(j != startnode);
}
void matrix_to_graph(int matrix[NUM_NODES][NUM_NODES], int graph[NUM_VERTICES][NUM_VERTICES])
{
    memset(graph, 0, NUM_VERTICES*NUM_VERTICES*sizeof(int));
    for(int i = 0; i < NUM_NODES; i++)
    {
        for(int j = 0; j < NUM_NODES; j++)
        {
            int elem_i = NUM_NODES * i + j;
            int elem_j;
            if(matrix[i][j] == 0)
            {

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
}


int main()
{
    int graph[NUM_VERTICES][NUM_VERTICES];
    int matrix[NUM_NODES][NUM_NODES];
    FILE *fp = fopen("matrix.txt", "r");
    for(int i = 0; i < NUM_NODES; i++)
    {
        char crt;
        for(int j = 0; j < NUM_NODES; j++)
        {
            fscanf(fp, "%c", &crt);
            matrix[i][j] = (int) (crt - '0');
        }
        fscanf(fp,"%c", &crt);
    }
    fclose(fp);
    
    matrix_to_graph(matrix, graph);
    fp = fopen("matrix_out.txt", "w");

    for(int i = 0; i < NUM_VERTICES; i++)
    {
        for(int j = 0; j < NUM_VERTICES; j++)
        {
            fprintf(fp, "%d", graph[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    int startnode, endnode, n = NUM_VERTICES;
    printf("\nEnter the starting node:");
    scanf("%d", &startnode);
    printf("\nEnter the final node:");
    scanf("%d", &endnode);
    dijkstra(graph, n, startnode, endnode);
    return 0;
}