#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h> 

typedef struct Point
{
	double x, y;
	int parent;
	double distance[10];
}Point; 

typedef struct Centroid
{
	double x, y;	
}Centroid; 

double eclidDis(Point p, Centroid c)
{
	return sqrt(pow((c.x - p.x),2) + pow((c.y - p.y),2));
}


void cpy(Centroid* src, Centroid* dest, int limit)
{
	int i;
	for(i=0;i<limit;i++)
	{
		dest[i].x = src[i].x;
		dest[i].y = src[i].y;
	}
}


bool dComp(double x, double y)
{
	double diff = x-y;
	if(diff > 0.001 || diff < -0.001)
	{
		return false;
	}
	return true;
}

bool equal(Centroid* c1, Centroid* c2, int limit)
{
	int i;
	for(i=0;i<limit;i++)
	{
		if(!dComp(c1[i].x, c2[i].x))
		{
			return false;
		}
		if(!dComp(c1[i].y, c2[i].y))
		{
			return false;
		}
	}
	return true;
}

int main ()
{
	int id, centroidsNum, size=0;
	
	char line [100];
	
	Point points[100];

	Point maxPoint, minPoint;
	
	maxPoint.x = maxPoint.y = -1e9;
	minPoint.x = minPoint.y = 1e9;

	#pragma omp parallel shared(centroidsNum) 
	{
		centroidsNum = omp_get_num_threads();
	}	 

        /* Read The data file  */	
	FILE* my_file = fopen("list.txt", "r");

	if(my_file)
	{
		while (fgets(line, 100, my_file))
		{
			sscanf(line, "%lf %lf", &points[size].x, &points[size].y);

			if (points[size].x > maxPoint.x) maxPoint.x = points[size].x;
			if (points[size].x < minPoint.x) minPoint.x = points[size].x;

			if (points[size].y > maxPoint.y) maxPoint.y = points[size].y;
			if (points[size].y < minPoint.y) minPoint.y = points[size].y;

			size++;
		}
		fclose(my_file);
	}

	maxPoint.x += 10;
	maxPoint.y += 10;
	
       /*Initiate 2 random numbers for each thread/cluster (x , y)*/

	srand(time(NULL));	
	Centroid *centroids =  malloc (centroidsNum * sizeof(Centroid));
	int i;	
	for(i = 0; i < centroidsNum; i++)
	{
		int base = maxPoint.x - minPoint.x + 1;
		centroids[i].x = (rand()%base) + minPoint.x;
		centroids[i].y = (rand()%base) + minPoint.y;
                //printf("centroid %d : %lf, %lf ", i+1, centroids[i].x, centroids[i].y);
	}

        //printf("\n");
	

	Centroid *oldCentroids =  malloc (centroidsNum * sizeof(Centroid));
	int k=0;

	while(!equal(oldCentroids, centroids, centroidsNum))
	{
		cpy(centroids, oldCentroids, centroidsNum);
	        /*Calculate the distance between each point and cluster centroid.*/
                #pragma omp parallel shared(points , centroidsNum , size) private(i)
		{
			id = omp_get_thread_num();
			
			for (i = 0; i < size ; i++)
			{
				points[i].distance[id] = eclidDis(points[i],centroids[id]);
			}
		}
		
	        /*Filter each point distances depending on minimum value*/	
		for (i = 0; i < size ; i ++)
		{
			double currMin = 1e9;
			int j ;
			for (j = 0; j < centroidsNum ; j ++)
			{
				if(points[i].distance[j] < currMin)
				{
					currMin = points[i].distance[j];
					points[i].parent = j;				
				}
			}
		}

		double sumX, sumY;
		int count;
	        /*Calculate the mean for each cluster as new cluster centroid */
	        #pragma omp parallel shared(points , centroidsNum , size) private(i, sumX, sumY, count)
		{
			id = omp_get_thread_num();
			sumX = 0.0, sumY = 0.0;
			count = 0;
			for (i = 0; i < size ; i ++)
			{			
				if(points[i].parent == id)
				{
					count++;
					sumX += points[i].x;
					sumY += points[i].y;
				}
			}

			if(count != 0)
			{
				centroids[id].x = sumX / count;
				centroids[id].y = sumY / count;
			}else
			{
				centroids[id].x = rand()%10;
				centroids[id].y = rand()%10;
			}		
		}
		k++;
	}

	//printf("took %d iterations\n", k);
	int j;
	for(j = 0; j < centroidsNum; j++)
	{
		printf("Cluster %d:\n", j+1);	

		for (i = 0; i < size ; i ++)
		{
			if(points[i].parent == j)
			{
				printf("(%lf ,%lf)\n",points[i].x,points[i].y);	
			}			
		}	
	}
	
	//printf("cents %d\n", centroidsNum);

	/*
	for(i = 0; i < centroidsNum; i++)
	{
            printf("centroid %d : %lf, %lf ", i+1, centroids[i].x, centroids[i].y);
        }
   	 printf("\n");
        */

}
