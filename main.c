//
//  main.c
//  Lab06
//
//  Created by NUTH on 8/23/2559 BE.
//  Copyright © 2559 NUTHYLL. All rights reserved.
//

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int **alloc_2d_int(int rows,int cols){
    int i ;
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array = (int **)malloc(rows*sizeof(int*));
    for (i=0; i<rows; i++) {
        array[i] = &(data[cols*i]);
    }
    return array;
}


int main(int argc, char * argv[]) {
    
    int rank,size;
    int rowsB,columnsB;
    int rowsA,columnsA;
    int eachRow,modRow;

    MPI_Init(&argc, &argv);
    int **matA=NULL,**matB = NULL;
    double starttime = 0,endtime=0;

    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    
    if(rank==0){
        FILE *fp1,*fp2;
        int i,j;
        
        /* read matrix A */
        fp1 = fopen("/Users/Nuth/Desktop/MatrixMultipleData/matrix1.txt" , "r");
        fscanf(fp1,"%d %d",&rowsA,&columnsA);
        matA = alloc_2d_int(rowsA, columnsA);
        
        for (i=0; i<rowsA; i++) {
            for (j=0; j<columnsA; j++) {
                fscanf(fp1, "%d ",&matA[i][j]);
            }
        }
        
        fclose(fp1);
    
        
        /*read matrix B*/
        fp2 = fopen("/Users/Nuth/Desktop/MatrixMultipleData/matrix2.txt" , "r");
        fscanf(fp2,"%d %d",&rowsB,&columnsB);
        matB = alloc_2d_int(rowsB, columnsB);
        
        for (i=0; i<rowsB; i++) {
            for (j=0; j<columnsB; j++) {
                fscanf(fp2, "%d ",&matB[i][j]);
            }
        }
        
        fclose(fp2);
        starttime = MPI_Wtime();
        eachRow = rowsA/size;
        modRow = rowsA%size;
    }
    
    
    MPI_Bcast(&rowsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&columnsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rowsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&columnsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank!=0){
        matB = alloc_2d_int(rowsB, columnsB);
    }
    
    MPI_Bcast(&matB[0][0],rowsB*columnsB, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&eachRow, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&modRow, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(rank==0){
        
        int i=0,j=0,k=0;
        for (i=1; i<size; i++) {
            MPI_Send(&matA[(eachRow*i)+modRow][0],eachRow*columnsA, MPI_INT, i, i, MPI_COMM_WORLD);
        }
        
        int **result = alloc_2d_int(rowsA, columnsB);

        for (i=0; i<eachRow+modRow; i++) {
            for (j=0; j<columnsB; j++) {
                result[i][j]=0;
                for (k=0; k<rowsB; k++) {
                    result[i][j] += matA[i][k]*matB[k][j];
                }
            }
        }
        
        for (i=1; i<size; i++) {
            MPI_Recv(&result[(eachRow*i)+modRow][0], eachRow*columnsB, MPI_INT,i, 115, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        endtime = MPI_Wtime();
        printf("Total %lf sec.\n",endtime-starttime);
        
        FILE *of= fopen("/Users/Nuth/Desktop/MatrixMultipleData/out_medium.txt", "w");
        for (i=0; i<rowsA; i++) {
            for (j=0; j<columnsB; j++) {
                fprintf(of,"%d ",result[i][j]);
            }
            fprintf(of,"\n");
        }
        
        fclose(of);


        
    }else{
        int **parA = alloc_2d_int(eachRow, columnsA);
        int **result = alloc_2d_int(eachRow,columnsB);
        MPI_Recv(&parA[0][0], eachRow*columnsA, MPI_INT, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int i,j,k;

        for (i=0; i<eachRow; i++) {
            for (j=0; j<columnsB; j++) {
                result[i][j]=0;
                for (k=0; k<rowsB; k++) {
                    result[i][j] += parA[i][k]*matB[k][j];
                }
            }
        }
        
        MPI_Send(&result[0][0], eachRow*columnsB, MPI_INT, 0, 115, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}
