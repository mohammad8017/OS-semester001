
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


void* workingThread( void* sets );


typedef struct thSetsTag
{
    pthread_t thID;                     
    int thNum;                          
    unsigned long long int iniTerm;     
    unsigned long long int endTerm;     
    double localRes;                    
} thSets;

int main( int argc, char *argv[] )
{
    int n;                              
    int res;                           
    void* thrdRest;                    
    int maxThrds;                       
    unsigned long long int maxTerms;    
    unsigned long long int termsInc;    
    thSets* groupSets;                  
    double finalRes = 0.0;              

    
    if ( argc != 3 )
    {
        printf( "\nArguments missing!\n\n" );
        printf( "Usage:  piMultiThread <threads> <terms>\n" );
        exit( EXIT_FAILURE );
    }

    maxThrds = atoi( argv[ 1 ] );               // Total amount of threads
    maxTerms = strtoull( argv[ 2 ], NULL, 0 );  // Total amount of terms

    
    // Set up threads
    
    termsInc = maxTerms / maxThrds;             

    groupSets = ( thSets* )calloc( maxThrds, sizeof( thSets ) );

    for ( n = 0; n < maxThrds; n++ )
    {
        groupSets[ n ].thNum = n + 1;

        groupSets[ n ].iniTerm = termsInc * n + 1;

        if ( n == maxThrds - 1 )
            groupSets[ n ].endTerm = maxTerms;
        else
            groupSets[ n ].endTerm = termsInc * ( n + 1 );

        groupSets[ n ].localRes = 0.0;
    }

    
    // Start threads
    
    for ( n = 0; n < maxThrds; n++ )
    {
        res = pthread_create( &groupSets[ n ].thID, NULL,
                              workingThread, ( void * )( groupSets + n ) );

        if ( res != 0 )
        {
            printf( "Thread %d creation failed\n", n + 1 );
            exit( EXIT_FAILURE );
        }
    }

    // Wait for threads to finish
    
    printf( "\nWaiting for threads to finish...\n\n" );

    for ( n = 0; n < maxThrds; n++ )
    {
        res = pthread_join( groupSets[ n ].thID, &thrdRest );

        if ( res != 0 )
        {
            printf( "Thread %d join failed\n", n + 1 );
            exit( EXIT_FAILURE );
        }
    }

    
    // Calculate and display final result
    
    finalRes = 0.0;

    printf( "\tThrd |     Ini Term |     End Term |   Parcial Res\n" );

    for ( n = 0; n < maxThrds; n++ )
    {
        printf( "\t%4d | %12llu | %12llu | %13.10f\n",
            groupSets[ n ].thNum,
            groupSets[ n ].iniTerm,
            groupSets[ n ].endTerm,
            groupSets[ n ].localRes );

        finalRes += groupSets[ n ].localRes;
    }

    printf( "\nPI using %llu terms: %.10f\n",
            ( unsigned long long int )maxTerms, finalRes );

    
    // Housekeeping
    
    free( groupSets );

    exit( EXIT_SUCCESS );
}

// Worker threads function
void* workingThread( void* sets )
{
    unsigned long long int iniTerm = 1;     
    unsigned long long int endTerm = 1;     
    unsigned long long int n = 1;           
    int s = 1;                              

    // Get limit terms
    iniTerm = ( ( thSets * )sets )->iniTerm;
    endTerm = ( ( thSets * )sets )->endTerm;

    // Reset local result
    ( ( thSets * )sets )->localRes = 0.0;

    // Calculate local result
    for ( n = iniTerm; n <= endTerm; n++ )
    {
        // Set up term's sign
        if ( n % 2 )
            s = 1;
        else
            s = -1;

        // Update local result
        ( ( thSets * )sets )->localRes += s / ( double )( ( n * 2 ) - 1 );
    }

    pthread_exit( "done" );
}