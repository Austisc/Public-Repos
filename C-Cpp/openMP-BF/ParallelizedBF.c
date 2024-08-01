#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#define EXTRAPADDING 2000//constant for extrapadding
/*
   Initialise of  a bit array  
   which I use the size of Integer as each bit in the index is the position of the bit array for the bloom filter
   using bit-wise operator which the size is corresponding to the size int which it is acted as constant.
*/

const unsigned long int_size_t=sizeof(int);
/*
  using bitwise operator  to flip the bit   and then shift the bit to the correct position by index % int_size_t at at index / int_size_t
  @param int * Integer bitArray
  @para, index 
*/
void flipbit(int *array, int index) {
    array[index / int_size_t] |= 1 << (index % int_size_t);
}

/*
  using bitwise operator  to get the bit at respective index at index / int_size_t which at the position of index% int_size_t
  @param int * Integer bitArray
  @para, index * index
*/
int get_bit(int *bitArray, int index) {
    return  1 & bitArray[index / int_size_t] >>index% int_size_t;
}
/*
  using floor(size/int_size_t)+size%int_size_t this formula and calloc to initialize the correct size of bit array.
  @param int * Integer size
*/
int* initializeBitArr(int size) {
    return (int*)calloc(floor(size/int_size_t)+size%int_size_t,sizeof(int_size_t));
}
//header of the funciton
int NumHashFunc(int sizeARR,int numElement);
int SizeBITArray(float FalsePositiveRate,int numElement);
char **ReadFrom(char *ptrFN,int *numElem);
bool isPrime(int num);
unsigned  primeNumbers(int sizeArr);
unsigned  RN(int a,int b,int seed,int m);
unsigned int RNG(int a,int b,int seed,int m,int NumFunc);
unsigned int UniversalHash(char * stri,int seed,int primes,int STRindex,int m );
int HashInsert(char * stri,int *BitArray,int seed,int primes,int m,int NumFunc,int num_thread );
int* MultiInsert(char **input,int primes,int m,int NumFunc,int size,int *isInserted,int total_thread,int thread_per_region);
int lookUp(char * stri,int *BitArray,int seed,int primes,int m,int NumFunc ,int num);
int *MultilookUp(char **input,int * CBitArray,int primes,int m,int NumFunc,int size,int total_thread,int thread_per_region);
int* MultiNaiveComparison(char **inputStrings,char **LookUpStrings,int NumInput,int NumLookUpSts,int *isInserted);
void WriteFile(char *pFN,int * MultiLookUp,int *TruthArr,char**Arr,int CnumElem );
float FalsePositiveRate(int * MultiLookUp,int *TruthArr,int CnumElem );
// global intilaization
  struct timespec start,end,readstart,readend,par_insert_start,
    par_insert_end,par_lookup_start,par_lookup_end,par_nc_start,
    par_nc_end,writefilestart , writefileend,fp_start,fp_end,in_lu_nc_start, in_lu_nc_end;
    //time taken 
    double time_taken,read_time_taken,insert_time_taken,quert_time_taken,naive_compare_time_taken,in_lu_nc_timetaken,
    fp_calculate_time_taken,writing_and_freeing_time_taken;  //time taken for each section to further analysis
    int numElem,numElem2; //number of unique element in both file 
    int SizeBITY;   //size of bit array
    int NumsHashF; // number of hash function

int main(){

    //initilize timespec for each seciton with start and the end point
  
    clock_gettime(CLOCK_MONOTONIC, &readstart);
    char **inStrArr;
    char **luStrArr;
    int scan;
    int total_thread;
    int thread_per_region;
    
    

    //time of start reading
    
  
   
    //Reading from the file
    inStrArr=ReadFrom("spUnique.txt",&numElem);

    luStrArr=ReadFrom("combination.txt",&numElem2);



    clock_gettime(CLOCK_MONOTONIC, &readend);
    read_time_taken =(readend.tv_sec - readstart.tv_sec) + (readend.tv_nsec - readstart.tv_nsec) * 1e-9;

    printf("Overall Reading Files time (s):             %lf\n", read_time_taken);

     //time of start end reading

    

    //section to calculate the optimise value for the bloom filter with the formula
    //using the larger number of element to iniliaize the bit array
    int maxNumElem=(numElem2>numElem)? numElem2:numElem;
    // to calculate the optimised size of bit array
    // as well as 10 percentage to calculate the size of bit array with maximisse value
    SizeBITY= SizeBITArray(10.0,maxNumElem);
    // to calculate the optimised number of hash function for the number of hash element
    NumsHashF=NumHashFunc(SizeBITY,numElem);
   // the next prime number that larger than size of bit array for hash funciton
    int primesNum=primeNumbers(SizeBITY);

    printf("Number of Unique words to Insert:           %d\n",numElem);
    printf("Number of Unique words to Look Up:          %d\n",numElem2);
    printf("SIZE Of BIT ARRAY:                          %d\n",SizeBITY);
    printf("Number of Hash Function(Times to be Hashed):%d\n",NumsHashF);
    printf("Next Prime Value:                           %d\n",primesNum);
   

    printf(" Case 1:  Data-partion Parallelism: one threads, one word, simaltaneous Task  \n");
    printf("Case 2: Nested parallelism: Several Master Threads which each working on a word with its own slave threads     \n" );
    printf("Case  3: Parallelised hashing for Inserting and Querying: All the threads working hashing and working on a words at one time \n");
    
    printf("Case 4: Basically Serial: One Threads                          \n");
//choose case for each which would have different outcome in terms of time

    printf("Enter the number betweem 1 and 4 :   ");
    scanf("%d",&scan);
    switch (scan) {
        case 1:
            total_thread=omp_get_max_threads();
            thread_per_region=1;
            break;
 
        case 2:
            if(NumsHashF<16){
                total_thread=omp_get_max_threads()/2;
                thread_per_region=2;
            }
            else{
            total_thread=4;
            thread_per_region=omp_get_max_threads()/4;
            }
            break;
 
        case 3:
            total_thread=1;
            thread_per_region=omp_get_max_threads();

          
            break;
        case 4:
            total_thread=1;
            thread_per_region=1;

          
            break;

 
        default:
            break;
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
   
    //serial region
    printf("total number of threads:             %d\n",total_thread);
    printf("total number of threads per region:  %d\n",thread_per_region);
    printf("Number of Unique words to Insert:    %d\n",numElem);
    printf("Number of Unique words to Look Up:   %d\n",numElem2);
    printf("SIZE Of BIT ARRAY:                   %d\n",SizeBITY);
    printf("Number of Hash Function:             %d\n",NumsHashF);
    printf("Next Prime Value:                    %d\n",primesNum);
    printf("Overall Reading Files time (s):             %lf\n", read_time_taken);
    // the bit array for to know whether the item is insereted or not

    int *  isItinsert =  initializeBitArr(numElem);
    clock_gettime(CLOCK_MONOTONIC, &in_lu_nc_start);
    clock_gettime(CLOCK_MONOTONIC, &par_insert_start);
    //Insertion of Bloom Filter
    int *  BitArr= MultiInsert(inStrArr,primesNum,SizeBITY,NumsHashF,numElem,isItinsert ,total_thread,thread_per_region);
    clock_gettime(CLOCK_MONOTONIC, &par_insert_end);
    
    insert_time_taken =(par_insert_end.tv_sec - par_insert_start.tv_sec) + (par_insert_end.tv_nsec - par_insert_start.tv_nsec) * 1e-9;
    printf("Overall Insert time (s):                          %lf\n", insert_time_taken);

    clock_gettime(CLOCK_MONOTONIC, &par_lookup_start);
    //Query of Bloom Filter
    int *  LU= MultilookUp(luStrArr,BitArr,primesNum,SizeBITY,NumsHashF,numElem2,total_thread,thread_per_region);
    clock_gettime(CLOCK_MONOTONIC, &par_lookup_end);
    quert_time_taken =(par_lookup_end.tv_sec - par_lookup_start.tv_sec) + (par_lookup_end.tv_nsec - par_lookup_start.tv_nsec) * 1e-9;
    printf("Overall Look Up time (s):                         %lf\n", quert_time_taken);
    
    clock_gettime(CLOCK_MONOTONIC, &par_nc_start);
     //Comparision between isnerting and querying word list to get the confirmed result
    int *  TT= MultiNaiveComparison(inStrArr,luStrArr,numElem,numElem2,isItinsert );

    clock_gettime(CLOCK_MONOTONIC, &par_nc_end);
    
    naive_compare_time_taken =(par_nc_end.tv_sec - par_nc_start.tv_sec) + (par_nc_end.tv_nsec - par_nc_start.tv_nsec) * 1e-9;
    printf("Overall Naive Comparison time (s):                 %lf\n", naive_compare_time_taken);

    clock_gettime(CLOCK_MONOTONIC, &fp_start);
    int FP= FalsePositiveRate(LU,TT,numElem2);
    clock_gettime(CLOCK_MONOTONIC, &fp_end);
    fp_calculate_time_taken=(par_nc_end.tv_sec - par_nc_start.tv_sec) + (par_nc_end.tv_nsec - par_nc_start.tv_nsec) * 1e-9;
    printf("Overall False Positive Computational time (s):      %lf\n", fp_calculate_time_taken);

    clock_gettime(CLOCK_MONOTONIC, &in_lu_nc_end);
    in_lu_nc_timetaken=(in_lu_nc_end.tv_sec -in_lu_nc_start.tv_sec) + (in_lu_nc_end.tv_nsec - in_lu_nc_start.tv_nsec) * 1e-9;

    printf("Overall Parallilized Bloom Filter time (s):        %lf\n",  insert_time_taken+quert_time_taken+naive_compare_time_taken+fp_calculate_time_taken);
    
    clock_gettime(CLOCK_MONOTONIC, &writefilestart);
    
    //Wrting the query reuslt parts.
    WriteFile("output.txt",LU,TT,luStrArr,numElem2);
    
    
    // freeing the string inside the string array
    for (int i = 0; i < numElem; i++) {
            free(inStrArr[i]);
            
    }
     // freeing the string inside the string array
    for (int i = 0; i < numElem2; i++) {
            free(luStrArr[i]);
    }
    	
    // freeing all the array
    free(isItinsert);
    free(inStrArr);
    free(luStrArr);
    free(BitArr);
    free(LU);
    free(TT);
    clock_gettime(CLOCK_MONOTONIC, &writefileend);
    writing_and_freeing_time_taken =(writefileend.tv_sec - writefilestart.tv_sec) + (writefileend.tv_nsec - writefilestart.tv_nsec) * 1e-9;
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Overall Writing Files time (s):               %lf\n", writing_and_freeing_time_taken );

time_taken =(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
float total_time_taken=read_time_taken +  insert_time_taken+quert_time_taken+naive_compare_time_taken+fp_calculate_time_taken+writing_and_freeing_time_taken;
printf("Overall time (s):                                 %lf\n", total_time_taken);
    
    return 0;
}

/*
Function to read file which it dynamically reallocate the array of string
which while reading the file, it will check if the character is inside the array of string or not else it would not isnsert or nnot usign the while loop
However it needs to know how many elements is needed to insert to the words to avoid fault segmetaiton
@ptrFN: pointer to File
@numELem: number of items to isnert


*/

char **ReadFrom(char *ptrFN,int *numElem){
     
     
    FILE *ptFILE = fopen(ptrFN,"r"); // to open the file

    fscanf(ptFILE,"%d",numElem);

    int numUnique=0;
    int index=0;
    char  **words=(char**)malloc((*numElem)*sizeof(char*));     
    char *string;
   
     for (int i = 0; i < *numElem; i++) {
        
        string=(char*)malloc(sizeof(char)*256); //assume the size of string is 256
        fscanf(ptFILE, "%s", string);
        int j=0;
        bool unique=true;
        while( numUnique-1>j ){ //naive compare
                    int res=strcmp(words[j],string);//to compare the string
                    if(res == 0 ){ 

                            unique=false;
                            break;
                    }
                   
                    j++;
         }
        
                            
        if(unique){ // if uniqiue insert it into the string of array
            int sizeSTR=(int)strlen(string);
            words[numUnique] = (char *)malloc(1+sizeSTR*sizeof(char)); 
        
            strcpy(words[numUnique],string);
            numUnique+=1;
            
        }
       
        }
    free(string);//free the string
    words=realloc(words,numUnique*sizeof(char *)); //to realloc so to its corresponding Number of Unique Words

  
     *numElem=numUnique; 
     
     fclose(ptFILE);
     return words;

}
/*
the formula for positive rate = (1-(1/m)^(n/h))^h
where it is not used in this implementaiton
@param sizeArr size of BitArray
@param numFunc numbe rof funciton
@param num of element
@return possible overall positive rate
*/

float FalsePositiveRpte(int sizeArr,int numFunc,int numElem){
        float P=pow((1-pow(1-(1/sizeArr),numElem*numFunc)),numFunc);
        return P*100;
}

/*
the optmised size of m based on number of item to inser and the false positive rate 
@param sizeArr size of BitArray
m=-(ln(P)*n)//ln(2)^2
n is the number of element and P is false positive rate in percenrtage
@param false positve rate in percentage u want
@param num of element
@return m
*/

int SizeBITArray(float FalsePositiveRate,int numElement){
        float m=((numElement*log(FalsePositiveRate))/(pow(log(2),2)));
        int sizeArr=ceil(m);
         return sizeArr;
}
/*
the optimised nubmer of hash function dpends on the size of m and 

@param num of element
@pararm number of item
@return int the optimised number of hash funciton
*/

int NumHashFunc(int sizeARR,int numElement){
        float m=(sizeARR/numElement)*(log(2));
        int NUMFUNC=ceil(m);
        return NUMFUNC;
}
/*
basic implemenation to check whether it is prime or not 

@param num
@return bool is it prime or not
*/

bool isPrime(int num){
    if(num%2==0){
        return false;
    }
    for(int i=3; i<sqrt(num);i+=2){
            if(num%i==0){
                return false;
            }
    }
    return true;

}
/*
basic implemenation to find the next prime number
 than larger to sizeARR(for Univerasl Hashing)

@param size of bit array
@return bool is it prime or not
*/
unsigned int primeNumbers(int sizeARR) {
    int *primes = (int *)malloc(sizeARR*sizeof(int));
    int prime=sizeARR+1;
    while(!isPrime(prime)){
            prime++;

    }
    return prime;
}


/*
to find the random value based on the input

@param int a b seed m paramete for psuedo integer random number generator
@return int  random number
*/
unsigned int RN(int a,int b,int seed,int m){
        
    int X= (a+b*seed)%m;
    return X;
}

/*
Universal Hash Function based on the equation,
https://en.wikipedia.org/wiki/Universal_hashing
@param char* string that want to be hashed
@param int seed primes 
@param int n , primes >=m, 
@param STRindex, the range of the size that it will be iterate and hased
@param m size of bitarary to ensure that it is within a range
@return hash value based on the words
*/

unsigned int UniversalHash(char * stri,int seed,int primes,int STRindex,int m ){
    unsigned int h= seed;

    for(int i=0; i<STRindex;i++){
        h+=((h*seed+(int)stri[i]))%primes; 
    };
    return h%m>=0?h%m:m-h%m; // to find the same congreunce class for the set of integer if it is smaller than 0
}

/*
Inserting several Hash value to a bit array in this given function
which basically hash the value and flip the bit, else if not, accumumulate it and if it does not
if accumulate==NumFunc, it is all well occupied, which we will know is it okay to insert into the bit array or not

@param char* string that want to be hashed
@param int  bit array we want to insert
@param int n , primes >=m, 
@param seed,prime to controll the universe hash.
@param m size of bit array to know its size
@param NumFunc: number of time it need ot be hashed 
@param num number of threads for function
@return 0 or 1 to know it is inserted or not
*/

int HashInsert(char * stri,int *BitArray,int seed,int primes,int m,int NumFunc,int num ){
    int partition=floor(strlen(stri)/NumFunc); // to get the partition that to ensure that it hashed NumFunc times
    int p=ceil(strlen(stri)/num); // for schedule which it is divided the number for the chunk size so that it could be partition based on case 2
    //as partition is higher, the hash function will run longer, which schedule guide, can help
    // and chunk size keep decreasing as the universal hash run longer, which it is more distributed in run time
    //while it assigned the task as dynamic
    int isInserted=0;
    int RNa;//RNa random number for hash function
    int i;
                                                                    //to ensure it is private
    #pragma omp parallel for schedule (guided,p) num_threads(num) private(RNa) reduction(+:isInserted)  // to ensure it is corrected
    for( i=0; i<NumFunc*partition;i+=partition){
       int RNa=RN(i*NumFunc,pow(2,31)*(i),(int)stri[i],m); //Random number based on the instance of i and stri[i]
       int h = UniversalHash(stri,RNa,primes,NumFunc*partition,m);//Universal Hash which it will be working on the given parameter to produce a hash
       if(get_bit(BitArray,h)==1){// if ==1, isInserted +=1

            isInserted++;
       }
       #pragma omp critical // to ensure that it is not insert bit to allow that two item able to insert
       {
        if (get_bit(BitArray,h)==0) //else flip te bit
       {
        flipbit(BitArray,h);
       }
       }
    }
    if (isInserted==NumFunc){// to check whether it is inserted or not
        return 0;
    }
    return 1;
}

/*

same concept as above but instead it is checking and not inserting which it does check with number, it is 
all occupied it is inserted
@param char* string that want to be hashed
@param int  bit array we want to insert
@param int n , primes >=m, 
@param seed,prime to controll the universe hash.
@param m size of bit array to know its size
@param NumFunc: number of time it need ot be hashed 
@param num number of threads for function
@return 0 or 1 to know it is inserted or not
Parallelizaiton same as above
*/

int lookUp(char *stri,int *BitArray,int seed,int primes,int m,int NumFunc,int num ){

    
    int partition=floor(strlen(stri)/NumFunc);
    int p=ceil(strlen(stri)/num);
    int isInserted=0;
    int RNa;
    int i;
    #pragma omp parallel for schedule (guided,p) num_threads(num) private(RNa) reduction(+:isInserted)
    for( i=0; i<NumFunc*partition;i+=partition){
       int RNa=RN(i*NumFunc,pow(2,31)*(i),(int)stri[i],m);
       int h = UniversalHash(stri,RNa,primes,NumFunc*partition,m);
       
       if(get_bit(BitArray,h)==1){
            
            isInserted++;
       }
    }
    if (isInserted==NumFunc){
        return 1;
    }
    return 0;
}


/*
basically at first, the shorter length can be hashed the numAFunc amount of time correctly but
now it is basically to add random uncerainess into the street which could lead to varied of padding of string
for the bloom filter has a good range of distribuation and adding the total cost of the computational to measure 
parallised time
@param char* string that want to be hashed
@param int string
@param seed,prime to controll the random number
@param NumFunc: number of time it need ot be hashed 
@param length the number that exponential increase its lenght
@return string that is padding and adjusted by length

*/

char* padding(char* input,int prime,int NumFunc,int length){
   
     unsigned int len=strlen(input);
   
     int additional_padding=(len-len%NumFunc)+len*length+1;// additional padding
     char *stri=(char*)malloc(len+additional_padding+1);// allocate an array of stri
     
     char x;
     
     strcpy(stri,input);//to copy the inputto the address stri
    
    int rn=(int)input[0];
    for(int i=len; i<len+additional_padding;i++){
                char numChar[1]; // to store the character to pad
                int rn=RN(i,prime,pow(2,31)*((int)stri[i]),NumFunc);//random number to generate new character
                numChar[0]=(char)(rn);//char casting
                strcat(stri, numChar); // append to the string
        }
    return stri;
    
}
/*
Inserting an array of string with the help of the funciton given above, which it returns an the bit array for bloom filter
which to know it is inserted or not, an Count Bit Array is needed.

@param char**  an array of string
@param int primes           for Universal hashes and and control it
@param int m                size of bit array
@param int NumFunc            number fo times a string need to be hased
@param int isInserted         Count array to confirm it is insered or not
@return Bit Array to know which it is inserted

Its parallelization is very staright forward which it si schedule dynamic which that it has hard 
to determine the hash insert based on the string, which it is available , it will get the string for the data set
*/

int * MultiInsert(char **input,int primes,int m,int NumFunc,int size,int *isInserted,int total_thread,int thread_per_region){
        
        int*BitArray=initializeBitArr(m);//initlailise of the bit Array
        int h; // initliased the hash     
        int index;
                                                                                    //to control number of thread
        #pragma omp parallel for schedule (dynamic) shared(isInserted,BitArray) num_threads(total_thread) 
        for(index=0;index<size;index++){
            char*stri=padding(input[index],primes,NumFunc,EXTRAPADDING);//padding of the string
            h=HashInsert(stri,BitArray,primes+primes*m,primes,m,NumFunc,thread_per_region);//insert and hashed into bit array which return whether it is inserting or not
            //critical is not needed as it is always writing at different locaiton
            if (h == 1) {//if it can be inserted, flip bit
                flipbit(isInserted,index);
            }
            }
        
        return BitArray;
        
}

/*
same as above but this one is for querying which it will LU to know which querying item
has been potentially in the set which might be a false positive.
@param char**  an array of string
@param int primes           for Universal hashes and and control it
@param int m                size of bit array
@param int NumFunc            number fo times a string need to be hased
@param int isInserted         Count array to confirm it is insered or not
@return LU to know it can be potentially in the bit array
Parallelizaiton method same as above
*/




int *MultilookUp(char **input,int * CBitArray,int primes,int m,int NumFunc,int size,int total_thread,int thread_per_region){
        
        int* LookUpTable=initializeBitArr(size);
        int index;
        
        
        #pragma omp parallel for schedule (dynamic) shared(CBitArray, LookUpTable) num_threads(total_thread) 
        for( index=0;index<size;index++){
             
             char*stri=padding(input[index],primes,NumFunc,EXTRAPADDING);
             int h=lookUp(stri,CBitArray,primes+primes*m,primes,m,NumFunc,thread_per_region);//insert and hashed into bit array which return whether it is found or not
             //critcial is not needed
             if(h==1){ //if it is find,flipped at that index
                 flipbit(LookUpTable,index);
             }
        }
        return LookUpTable;

}

/*
Naive comparsion to know which query item exist in inserting item
@param char **inputStrings         an array of inserting string
@param char **LookUpStrings        an array of querying strings
@param int NumInput                number of inserting item
@param int NumLookStr            number of querying item
@param int isInserted         Count array to confirm it is insered or not
@return input, to confirm it is can be found in the MultiNaiveComparsion or not
Parallelizaiton method it is impossible ot use collapse as which the each run in each outer loop
needed to be run but inner loop which it does not need to run the whole, sicne there is a break statement
which it will be wasted to run 
using static which since there is no additonal padding  and partition it into round -robin fashion which reduce overhead
*/

int* MultiNaiveComparison(char **inputStrings,char **LookUpStrings,int NumInput,int NumLookUpSts,int *isInserted){
    int* input=initializeBitArr(NumLookUpSts);
    int i;
    int chunk= ceil(NumLookUpSts/omp_get_max_threads()); // to evenly distribute it
    #pragma omp parallel for schedule(static,chunk) shared(i,inputStrings,LookUpStrings,isInserted)
    for ( i = 0; i < NumLookUpSts; i++)
    {
         for (int j = 0; j < NumInput; j++){
                    //if true it is inserted
                    if(strcmp(inputStrings[j],LookUpStrings[i]) == 0 &&  
                    get_bit(isInserted,j)==1){
                           
                            flipbit(input,i);
                            break;
                    }
         }
         
    }
    return input;

}
/*
False positive rate which baeed on the truth array from the naive comapre and querying list
which we know the true negative and false positive, which only cases is needed is when TruthArr[-
is needed, after that if MultiLookUp  array is used to know whether is not false positive or not
if MultiLookU[i]==1 it is a false positive. else it is not

@param int * MultiLookUp an count array that obtained from querying strings
@param int *TruthArr     an count array from Naive compare
@param int CnumElem      the number of element it inserted

@return float False positvie rate
parallization method which using schedule dynamic which to avoid race condtion 
reduction clause is needed which distribute a copy to eahc threads
*/

float FalsePositiveRate(int * MultiLookUp,int *TruthArr,int CnumElem ){
        int i=0,falsep=0,trueneg=0;
       
        
        #pragma omp parallel for schedule(dynamic) shared(i) reduction(+ : falsep,trueneg)
        for(i = 0; i < CnumElem; i++){
                 if(get_bit(TruthArr,i)==0){
                    
                      (get_bit(MultiLookUp,i)==1)?falsep++:trueneg++;
                }
       }


       printf("Number of False Positive::           %d\n", falsep);
       printf("Number of True Negative ::           %d\n", trueneg);
       printf("Rate of False Positive::             %1f\n", (float)falsep/(falsep+trueneg));
       
       return falsep/(falsep+trueneg);
}
/*
Writing file which based on the result
given the Nature of true array,false negative is impossible
so when turthArr[i], it will concat with tp string
else it will concat with tn and fp depends on MultiLookup
@param char* pointerFileName name of the file
@param int * MultiLookUp an array that obtained from querying strings
@param int *TruthArr     an array of querying strings
@param char*             Arr Querying strings
@param int CnumElem      the number of element it inserted
@void
*/

void WriteFile(char *pFN,int * MultiLookUp,int *TruthArr,char**Arr,int CnumElem ){
        int i=0,falsep=0,trueneg=0;
        FILE *pFile = fopen(pFN, "w");
        char * tp = " is a True Positive, It had been inserted";
        char * tn = " is a true negative,it has never been inserted before";
        char * fp = " is a False Positive, It has not been inserted";
        
        char *output;
        for(i = 0; i < CnumElem; i++){
                int len=strlen(Arr[i]);
                char * str=(char*)malloc((len+70)*sizeof(char));
                 if(get_bit(TruthArr,i)==1){
                     strcpy(str,Arr[i]);
                     char *output= (strcat(str,tp));
                     fprintf(pFile, "%s\n", str);
                     free(str);
                }
                else{
                     strcpy(str,Arr[i]);
                     char *output= (get_bit(MultiLookUp,i)==1)?(strcat(str,fp)):(strcat(str,tn));
                      (get_bit(MultiLookUp,i)==1)?falsep++:trueneg++;
                     fprintf(pFile, "%s\n", str);
                      free(str);
                }
       }
       //to confirm that i get the same value for above fucntion
       printf("Number of False Positive::           %d\n", falsep);
       printf("Number of True Negative ::           %d\n", trueneg);
       printf("Rate of False Positive::             %1f\n", (float)falsep/(falsep+trueneg));
       fprintf(pFile, "%d", falsep);
       fclose(pFile);

}