#include <mpi.h> 
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

MPI_Datatype MPI_SystemTime,MPI_AlertLog,MPI_NeighborInfo,MPI_CRES;
struct SystemTime times; 
#define num
#define nDims 2
#define row_dims 10
#define col_dims 10
#define nPorts 5
#define nNbr 4
#define Node_Rep 1
#define MAX 5
#define EVCar row_dims*col_dims
#define TERMINATION 4



struct Car{// struct car for  Car generator 
      int carID;
      int BattCharge;
};


struct ChargingPort{//the charging port
      struct Car car;    
      int occupied;
};

struct NodeNeighborInfo{//Neighbor hood info to store inside base station

        int Nodeid;       //Node id of the 
        int NodeNeighbor[4];// the respective node id of its neighbor
        int coords[2]; //coord of the respective node id
};

struct  SystemTime {//the time of the system
   int year,month,mday,hour,min,sec;
};


struct NodeLog {  //shared array
  int NodeLogId;        //id of the log in the node
  int NodeSources;      // the log that came from         //Destination of the logs
   // number of ports
  int Availbility;
  int NMessage;            //Message
  
  int NAvailPorts[nNbr];  //Availibility for each respective Node
  struct SystemTime time; //time of the log
};
//struct
struct Queue{//queue to store the nodelog
      int front,rear,size,max_size;//front,back and the max_size of the queue
      struct NodeLog LQ[MAX];   //stack of the queue

};
/*
 Initializaiton of the queue

*/
struct Queue QueueInit(){
      struct Queue q;
      q.max_size =MAX;
      q.front = q.rear = q.size = 0;
      return q;
};
/*
  FIFO implementation using circular queue 
  to enqueue into the Queue for the log node
    @param Ndelog the lastest log
    @param q poitner to the queueu
    https://www.geeksforgeeks.org/introduction-to-circular-queue/
*/

void enQueue(struct NodeLog  Log,struct Queue* q) { 


    
    if(q->size < q->max_size){//if not max size
       if(q->size>0){q->rear++;}//if not rear
        q->LQ[q->rear] = Log;//set the back to the lastest node
        q->size++;//size increase
    } else {//if already full
        q->LQ[q->front] = Log;//the front become 
        q->front = (q->front + 1) % q->max_size;//front is increment to the rear of the original item
        q->rear = (q->front + q->size-1) % q->max_size;//rear make it to the lastest insert item
        
    }
}


struct Node { // dont nned to send
  int NodeId;      //the id of the log // the log that came from
     //Destination of the logs
 int nbr[nNbr];   // number of ports
 struct Car Ports[nPorts];
 struct Queue LQ;
  //int occ;
 
};

struct AlertLog {//Alert Log;

  int  AlertLogID; // AlertLogID to the respective Node
  int  NAvailable;// Node Sources
  int  nMessagesExchanges; //number of exchange
  int  NAvailPorts[nNbr]; //number of NAvail Ports
  struct SystemTime sys_time;//the time of Alert Log
};

struct BaseStation{//Base Station
   struct NodeNeighborInfo NNodeDB[row_dims*col_dims];//storing the neighbor info in each respective index
   struct AlertLog AlertLogs[row_dims*col_dims];//to store the current logs after it receicing message and to write to the output file
   MPI_Comm comm; //to store the comm
};

void Create_MPI_NeighborInfo();//prototype funciton to create MPI datatype Neighbor Info
void CTCreateSysTime();  // create MPI Data type time;
void MPIDT_AlertLog();//    create the alert Log

/*
to initialise time
@param struct system time
basically it get the year, month and date of month of the local time
and initialise on 6 o'clock
*/
void time_initialization(struct SystemTime* system_tm) {
    time_t raw_time;
    time(&raw_time);
    struct tm local_times = *localtime(&raw_time);
    system_tm->year = local_times.tm_year +1900;
    system_tm->month = local_times.tm_mon + 1;
    system_tm->mday = local_times.tm_mday+ 1;
    system_tm->hour = 6;
    system_tm->min =  0;
    system_tm->sec =  0;
}
/*
to increment the min and hour increment
@param System time
*/
void TimeIncrement(struct SystemTime* system_tm) {
    
    system_tm->min = (system_tm->min+15)%60;
    if(system_tm->min == 0){
          system_tm->hour = system_tm->hour%24+1;
    }

}


/*
Create Systiem
*/


/*
prototype fucntion of the base station 
@param world comm
@param BaseStation 
@param rd number of rows of the grid
@param cd number of column of the grid
*/

void baseStation(MPI_Comm world_comm, struct BaseStation* baseStation,int rd,int cd);
/*
prototype fucntion of the EVSation
@param world_comm to communicate with base master
@param new_comm to communicate between slave
@param Node EV Station 
@param rd number of rows of the grid
@param cd number of column of the grid
*/
void EVStation(MPI_Comm world_comm, MPI_Comm comm,struct Node* EVS,int rd ,int cd);

int main(int argc, char** argv) {

    int row=3;//number of rows of the grid
    int col=3;//number of column of the grid if it is default
    if (argc==3){
        row= atoi(argv[1]);//capture the additional first argument
        col = atoi(argv[2]);//capture the additional second argument 
   }
    int provided;
    MPI_Init_thread(&argc, &argv,MPI_THREAD_MULTIPLE,&provided );//to ensure that it is safe to has thread in a program
    int global_rank, global_size;//global rank and global size
    
    CTCreateSysTime();//create mpi datatype time
    Create_MPI_NeighborInfo();//create mpi neighbor hood info
    MPIDT_AlertLog();//create alert log
    MPI_Comm new_comm,comm2D;//new comm
    int reorder=0;//reorder
    int nbr[4];//nbr[4] to store the niegbor
   
   
    int rank;
    int coords[2];
    struct Node EVCS;//initliase the node EVCS
    int dims[nDims]={row,col};//initliase the row and col
    int wrap_around[nDims]={0,0};//wrap around ignored in this assign
    struct BaseStation baseSt;//intialse the base statioon
    struct  NodeNeighborInfo NInfo;// ifnormationn of Neighbor hood
    
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);//rank of all the process
    MPI_Comm_size(MPI_COMM_WORLD, &global_size);//size of all the process
    MPI_Comm_split( MPI_COMM_WORLD,global_rank == global_size - 1, 0, &new_comm);//split slave and master with colour global_rank == global_size - 1, which it had global size-1
    if (global_rank ==global_size - 1) {//Nothing to do here
  
    } 
    else {
        MPI_Cart_create(new_comm, nDims, dims, wrap_around, reorder, &comm2D);// create grid cartersian of row x col and store inside comm2D
        MPI_Comm_rank(new_comm, &rank);// its new rank in that respective comm
        MPI_Comm_rank(new_comm, &EVCS.NodeId);// its new rank and store it for the EVCS Node ID 
        MPI_Cart_coords(comm2D, rank, 2, coords);// get the respective coord of the process
        MPI_Cart_shift( comm2D, 0, 1, &EVCS.nbr[0], &EVCS.nbr[1] );//store the row neighbor of that particular rank
	    MPI_Cart_shift( comm2D, 1, 1, &EVCS.nbr[2], &EVCS.nbr[3] );//store the column neighbor of that particular rank
        printf("Global Rank : %d Process %d,NodeId : %d  Coordinates (%d, %d), Neighbors (up: %d, down: %d, left: %d, right: %d)\n  \n", //print the information of the neighbor hood
            global_rank ,rank,EVCS.NodeId, coords[0], coords[1], EVCS.nbr[0], EVCS.nbr[1], EVCS.nbr[2], EVCS.nbr[3]);
        //initialise the Neighbor Node
        NInfo.Nodeid=EVCS.NodeId;
        NInfo.coords[0]=coords[0];
        NInfo.coords[1]=coords[1];

        for (int i = 0; i < 4; i++) {
                NInfo.NodeNeighbor[i] = EVCS.nbr[i];
            }}
//gather the  neighbor information and store it inside the respective lectuve address inside array
    
    MPI_Gather(&NInfo, 1, MPI_NeighborInfo, &baseSt.NNodeDB, 1, MPI_NeighborInfo, global_size - 1, MPI_COMM_WORLD);
    if(global_rank<global_size-1){//to store the array
      printf("Node ID: %d\n", NInfo.Nodeid);// to print its received
      printf("Node Neighbors: %d, %d, %d, %d\n", NInfo.NodeNeighbor[0], NInfo.NodeNeighbor[1], NInfo.NodeNeighbor[2], NInfo.NodeNeighbor[3]);
      printf("Coordinates: (%d, %d)\n", NInfo.coords[0], NInfo.coords[1]);}
    
   
    MPI_Barrier(MPI_COMM_WORLD);//to sychronise
    if(global_rank  < global_size-1){
       FILE *file;
       file = fopen("AL.txt", "w");//overwrite a new file
        EVStation(MPI_COMM_WORLD,comm2D,&EVCS,row,col);//
        printf("EV Station Node %d has been terminated\n ",EVCS.NodeId);//
        MPI_Comm_free(&comm2D);
        fclose(file);//close
    }
    if(global_rank ==global_size - 1){//size of the global rnak
    // double start_sim=MPI_Wtime();
     baseStation(MPI_COMM_WORLD,&baseSt,row,col);//base station
      //double end_sim=MPI_Wtime();
      //double diff=end_sim-start_sim;
      printf("Base Station has been terminated\n ");
   // printf(" \n Total Time of its excecution   : %.4f\n ",diff);//
     }
     
    MPI_Barrier(MPI_COMM_WORLD);//sychronise the world
    MPI_Type_free(&MPI_NeighborInfo);//free the mpi
    MPI_Type_free(&MPI_SystemTime);// free system time 
    MPI_Type_free(&MPI_AlertLog);// freee alert log mpi time
    MPI_Comm_free( &new_comm ); //free the comm
    MPI_Finalize();
    

    //docker run -v C:\FIT3143:/home/student/project -it monashfit/fit3143 bash
    return 0;
}

void CTCreateSysTime() {
    const int nvariable = 6;//the number of varaiable
    int blocklens[] = {1, 1, 1, 1, 1, 1};//create the time
    struct SystemTime  times;
    MPI_Datatype systime_types[] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};// each datatype in system time 
    MPI_Aint disp[nvariable];//displacement of the datatyoe
    //get the respective address of address
    MPI_Get_address(&times.year, &disp[0]);
    MPI_Get_address(&times.month,&disp[1]);
    MPI_Get_address(&times.mday, &disp[2]);
    MPI_Get_address(&times.hour, &disp[3]);
    MPI_Get_address(&times.min, &disp[4]);
    MPI_Get_address(&times.sec, &disp[5]);
  

    // to get the respective address  distance to the first element
    for (int i = 1; i < nvariable; i++) {
        disp[i] = disp[i] - disp[0];
    }
    disp[0] = 0;//set the first to be 0

    MPI_Type_create_struct(nvariable, blocklens, disp,systime_types, &MPI_SystemTime);// create the struct 
    MPI_Type_commit(&MPI_SystemTime);//commit
}

void Create_MPI_NeighborInfo() {
    //doing the samething to create struct
    const int nvariable = 3;
    int blocklens[] = {1, 4, 2};
    struct NodeNeighborInfo NNDB;
    MPI_Datatype systime_types[] = {MPI_INT, MPI_INT,MPI_INT};
    MPI_Aint disp[nvariable];
    MPI_Get_address(&NNDB.Nodeid, &disp[0]);
    MPI_Get_address(&NNDB.NodeNeighbor, &disp[1]);
    MPI_Get_address(&NNDB.coords, &disp[2]);
    //https://stackoverflow.com/questions/33618937/trouble-understanding-mpi-type-create-struct
    disp[2]=disp[2]-disp[0];
    disp[1]=disp[1]-disp[0];
    disp[0] = 0;
    MPI_Type_create_struct(nvariable, blocklens, disp,systime_types, &MPI_NeighborInfo);
    MPI_Type_commit(&MPI_NeighborInfo);
};
 
void MPIDT_AlertLog() {
    //doing the samething to create struct but with owns custom MPI data struct
    const int nvariable = 5;
    struct AlertLog ALogs;
    int blocklens[] = {1, 1, 1, 4, 1};
    MPI_Datatype systime_types[] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT,MPI_SystemTime};
    MPI_Aint disp[nvariable];
    MPI_Get_address(&ALogs.AlertLogID, &disp[0]);
    MPI_Get_address(&ALogs.NAvailable,&disp[1]);
    MPI_Get_address(&ALogs.nMessagesExchanges, &disp[2]);
    MPI_Get_address(&ALogs.NAvailPorts, &disp[3]);
    MPI_Get_address(&ALogs.sys_time, &disp[4]);
     // Calculate displacements relative to the base address of the struct
     //https://stackoverflow.com/questions/33618937/trouble-understanding-mpi-type-create-struct
    for (int i = 1; i < nvariable; i++) {
        disp[i] = disp[i] - disp[0];//
    }
    disp[0] = 0;
    MPI_Type_create_struct(nvariable, blocklens, disp,systime_types, &MPI_AlertLog);
    MPI_Type_commit(&MPI_AlertLog);
}

/*
@param id NodeLogId the currrent id of the node log in taht ev station
@param NodeSources  the log where its log the item
@param Availbility availibility of theports in that Node

*/

struct NodeLog NL_Init(int * NodeLogId,int *NodeSources,int Availbility){
    //initliase the nodelog
    struct NodeLog NL;

    NL.NodeLogId = * NodeLogId;
    NL.NodeSources =* NodeSources;
    NL.Availbility = Availbility;
    *NodeLogId+=1;// increment the value of that address NodeLogId by 1
   
    return NL;
}
/*
 Car generator
 to generate the car for the EV Station
 @param force_random to make it all zero
 @param 

*/
struct Car CarGenerator( int force_random){
        struct Car car ;
        if(force_random < 20|| rand()%100<50 ){
            car.carID=rand()%150000;
            car.BattCharge = rand()%5*10;}
        
        else{
              car.carID=-1;
        }

        return car;
}
/*
initlialise the alertLogInit
@param id of allert log
@param Messages Exchanges
@param nAvail 
@param Avail
*/

struct AlertLog AlertLogInit(int *ALID,int nMessagesExchanges,int nAvail,int*Avail){
               struct AlertLog AL;
               AL.AlertLogID=*ALID;
               AL.nMessagesExchanges=nMessagesExchanges;
               AL.NAvailable=nAvail;
               for (int i = 0; i < 4; i++) {
                    AL.NAvailPorts[i] = Avail[i];
                }
                *ALID+=1;
                return AL;}


/*
Writing output file at base station
@param BL base station
@param AL alert log
@param current iteration in the base station
@param time com  time between node and base station
@int  row dim
@int col dim
*/


void WriteLog(struct BaseStation * BL,struct AlertLog * AL , int iteration,int NodeId, int *node_recveived,double time,int avail,struct SystemTime*stime,int cd,int rd){
      FILE *file;
      file = fopen("AL.txt", "a");
      
      int log_time = rand()%60+1;//to simulate the time for log
      int Alogtime = rand()%log_time;//must be before log tiem
      https://stackoverflow.com/questions/19429138/append-to-the-end-of-a-file-in-c
      fprintf(file, "\n--------------------------------------------------------------------\n");
      fprintf(file, "Iteration :     %d\n",iteration);
      fprintf(file, "Logged time :      :   ");
      fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d\n",stime->year, stime->month, stime->mday,stime->hour, stime->min, log_time );

      fprintf(file, "Alert reported time      :");
      fprintf(file,"%04d-%02d-%02d %02d:%02d:%02d\n",stime->year, stime->month, stime->mday,stime->hour, stime->min,Alogtime);
      fprintf(file, "Number of Adjacent Node :  \n");
      fprintf(file, "Reporting Node\tCoord\tPort Value\tAvailabe Port\n");
      fprintf(file,"%d\t\t(%d,%d)\t\t%d\t%d\n", NodeId, BL->NNodeDB[NodeId].coords[0],BL->NNodeDB[NodeId].coords[1],nPorts, AL->NAvailable);
      fprintf(file,"Adjacent Nodes\tCoord\tPort Value\tAvailable Port\t\n");
    
    
      {
      // neighbor information ot the respective node
      for (int i = 0; i < 4; i++) {
        if(BL->NNodeDB[NodeId].NodeNeighbor[i]>=0){
            int neigh=BL->NNodeDB[NodeId].NodeNeighbor[i];
            fprintf(file, "\n%d\t\t(%d,%d)\t\t%d\t%d\n", neigh,BL->NNodeDB[neigh].coords[0], BL->NNodeDB[neigh].coords[1],nPorts, AL->NAvailPorts[i]);
        }}
        };
        fprintf(file,"Nearby Nodes		Coord\n");
        int arr[cd*rd];
        memset(arr,0,sizeof(arr));
        // to find the information of the neighbro's nrighbor in terms of  
        for (int i = 0; i < nNbr; i++) {
        if(BL->NNodeDB[NodeId].NodeNeighbor[i]>=0){
        int neigh=BL->NNodeDB[NodeId].NodeNeighbor[i];
        for (int j = 0; j  < nNbr; j++) {
          if(BL->NNodeDB[neigh].NodeNeighbor[j]>=0){
             int neighn=BL->NNodeDB[neigh].NodeNeighbor[j];
             if(arr[neighn]==0 && neighn!=NodeId){
                      arr[neighn]=1;
                      fprintf(file,"%d\t\t\t(%d,%d)\n", neighn, BL->NNodeDB[neighn].coords[0],BL->NNodeDB[neighn].coords[1]);}
           
           }
          }}}
         

          int RNum=0;
          char rec[50]="Available station nearby within 3th period :";
          //number of the receiving 
          for (int i = 0; i  < 12; i++) {
              if(node_recveived[i]>=0){
                  RNum++;
                  int neigh = node_recveived[i] ;  
                  char intTostr[35];  
                  sprintf(intTostr, " %d ",neigh);
                  strcat(rec,intTostr);

              }}
        fprintf(file,"Number of Nearby Recommended Node :  %d\n" ,RNum);
        fprintf(file,"%s\n" ,rec);
        fprintf(file,"Communication Time betweeen Base Station and Reporting Node :  %.8f  \n",time);
        fprintf(file,"Number of Communication betweeen Base Station and Reporting Node :  %d  \n" ,avail);
        fclose(file);

        }

/*
simulate the EVStatin
@param world comm commbetween master and slave
@param commbetween slave
@param poitner to  node of the  EV station
@param rd row of the grid
@param cd column of grid
*/

void EVStation(MPI_Comm world_comm, MPI_Comm comm, struct Node *EVS, int rd, int cd) {

    int termination = 0, nNL = 0, curr_occ = 0, l = 0, count = 0, sent = 1, Availability = 0;
    struct SystemTime stime;
    time_initialization(&stime);
    int sent2 = 1; //second setn
    int sent3 = 1;//third sent
    MPI_Status probe_status, status1, probe_status2;
    int flag, index;
    EVS->LQ = QueueInit();//intiliase the queue
    int ALID = 0;
    int avail[nNbr];// number of availiaable

    int NEx = 0;
    for (int a = 0; a < nNbr; a++) {
        if (EVS->nbr[a] >= 0) {
            NEx++;
        }
    }

    int i = 0;
    srand(EVS->NodeId*12385844);//intialise the seed
    while (!termination) {
        int force = rand() % 100;//force to occupy all the port
        int n_curr_avail = 0;
         struct NodeLog NL = NL_Init(&nNL, &EVS->NodeId,0);//Node Log
        #pragma omp parallel num_threads(nPorts) //simulate the ports
        {
            #pragma omp for
            for (int p = 0; p < nPorts; p++) {
                struct Car car = CarGenerator(force);
                if (EVS->Ports[p].carID > 0) {//if carID>0 
                    if (EVS->Ports[p].BattCharge >=90) { // if batt >90
                        EVS->Ports[p].carID = -1; //the car left
                    } else {
                        EVS->Ports[p].BattCharge += 20; //increment batt every one it loop
                    }
                }
                if (car.carID > 0) {//if not occupy
                    if (EVS->Ports[p].carID == -1) {//carID
                        EVS->Ports[p] = car;//car
                    } else {
                        #pragma omp atomic
                        NL.Availbility++;// n_curr_avail number of not available
                    }
                }
            }
        }

            
         
                int message[nNbr][2];//storing  receiving message
                int mrp = 0;
                sent = 1;
                sent2 = 1;
                sent3 = 1;
                count = 0;
                n_curr_avail=NL.Availbility;
                Availability = NL.Availbility;
                double start, end;
                //struct NodeLog NL = NL_Init(&nNL, &EVS->NodeId,0);//Node Log
                enQueue(NL, &EVS->LQ);//enqueue the lastest node
                double start_time, start_time2, end_time, end_time2;

                int sentM = 1;

                start_time = MPI_Wtime();//tiem measurement between
                while (1) {
                    int tflag = 0;
                    MPI_Status tstatus;
                    //in this probe it intent to listen to master to receive termination
                    MPI_Iprobe(rd * cd, TERMINATION, world_comm, &tflag, &probe_status);
                    if (tflag == 1) {
                        int dummy;

                        tflag = 0;
                        MPI_Recv(&dummy, 1, MPI_INT, rd * cd, TERMINATION, world_comm, &tstatus); //if recv
                       
                        termination = 1; //it will terminate

                        return;
                        
                    }
                    MPI_Status status;
                    int flag;
                    MPI_Iprobe(MPI_ANY_SOURCE, Node_Rep, comm, &flag, &probe_status);// if from slave comm
                    if (flag == 1) {
                        MPI_Recv(&message[mrp][1], 1, MPI_INT, probe_status.MPI_SOURCE, Node_Rep, comm, &status);//store the msg

                        message[mrp][0] = probe_status.MPI_SOURCE; //it will store incoming message store where the message from
                        mrp++;//increment the message response by 1
                    }
                    //if ports , if lastest log of availbility is smaller than 0;
                    if (nPorts - EVS->LQ.LQ[EVS->LQ.rear].Availbility <= 1) {
                        int avail[nNbr] = {-1, -1, -1, -1};//initliase -1 sicne avail can be 0
                        int avail_node = 0;//number of avail Node
                        if (sent) { //sent oen time onyl
                            sent = 0;
                            for (int n = 0; n < nNbr; n++) {
                                if (EVS->nbr[n] >= 0) {
                                    MPI_Send(&n, 1, MPI_INT, EVS->nbr[n], Node_Rep, comm);//send its index so sending back can immediately know which index to store at
                                }
                            }
                        }
                    }
                    if (count >= 5000 * nNbr) {
                        break;//loop counter to make sure a stable receive 
                    }
                    count++;
                }
                end_time = MPI_Wtime();//end time
                double td = end_time - start_time;// the communication time of listening and sending to the receiving neighbors
                MPI_Barrier(comm);
                int rem=0;
                int avail[nNbr] = {-1, -1, -1, -1};
                if (!termination) {//if not terminate
  
                    start_time = MPI_Wtime();
                    while (1) {
                        MPI_Status status;
                        int flag2;
                        MPI_Iprobe(MPI_ANY_SOURCE, 0, comm, &flag2, &probe_status2); //check whether there is message form neighbor
                        if (flag2 == 1) {
                            flag2 = 0;
                            int recv_msg[2];
                            rem=1;
                            MPI_Recv(recv_msg, 2, MPI_INT, probe_status2.MPI_SOURCE, 0, comm, &status);//
                            int recv_flag2 = 0;

                            avail[recv_msg[0]] = recv_msg[1];//the  availible of the neighbor node
                        }
                        if (sent2) {
                            sent2 = 0;
                            for (int n = 0; n < mrp; n++){
                                int sm[2] = {message[n][1], nPorts - Availability};//send the index of the neighbor node from the sources node and the numbe rof ports its availbility
                                MPI_Send(sm, 2, MPI_INT, message[n][0], 0, comm);//send bakc to the sources node;
                            }
                        }
                        if (count >= 10000 * nNbr) {
                            break;
                        }
                        count++;
                    }
                    end_time = MPI_Wtime();
                    double td2 = end_time - start_time;//time difference to send back and response back the time
                    MPI_Barrier(comm);
                    int Not_avl = 0;
                    int RNum = 0;
                    char rec[150] = "Recommended Neighboring Node      Number of Availbility  ";// to get the recommneded neiigboring ndoe 
                    for (int c = 0; c < nNbr; c++) { 
                        if (avail[c] <= 1 && avail[c] >= 0 && EVS->nbr[c] >= 0) {
                                  Not_avl++;//if not avail
                        } else if (EVS->nbr[c] >= 1 && avail[c] >= 2) {
                            RNum++;
                            int neigh = EVS->nbr[c];
                            char intTostr[35];
                            sprintf(intTostr, "\n\t%d\t\t\t\t%d\n", neigh,avail[c] );
                            strcat(rec, intTostr);
                        }
                    }

                    EVS->LQ.LQ[EVS->LQ.rear].Availbility=n_curr_avail;
                    MPI_Barrier(world_comm);
                    int sentAlert = 0;
                    int array2[12];
                    //log the node Log
                    if(rem){
                        printf("------------------------------------------------------------------------------------------\n");
                        printf("Iteration:                                %d\n",i+1);
                        printf("Reporting Time:                                ");
                        printf("%04d-%02d-%02d %02d:%02d:%02d\n", stime.year, stime.month, stime.mday, stime.hour, stime.min, stime.sec);

                        printf("Node ID      :                            %d \n", EVS->NodeId);
                        printf("Availibility :                            %d \n", nPorts - n_curr_avail);
                        printf("Number of Neighbors:                      %d\n", NEx);
                        printf("Communication Time to sending resquest:   %.4f\n", td);
                        printf("Communication Time to respond message:    %.4f\n", td2);
                        printf("Number of Neighboring Node Availibility:  %d \n", RNum);
                        printf("%s \n", rec);

                        
                    //if Number of enightbors - not avialable <=2
                    if (NEx - Not_avl <= 1) {
                       

                        struct AlertLog AL = AlertLogInit(&ALID, mrp, nPorts - n_curr_avail, avail);
                        if (sentM) {
                            sentM = 0;
                            MPI_Send(&AL, 1, MPI_AlertLog, rd * cd, 0, world_comm);
                            MPI_Recv(array2, 12, MPI_INT, rd * cd, 0, world_comm, MPI_STATUS_IGNORE);
                        
                            char st2[100] = "Nearby Availbility Nodes :";
                            int NND = 0;
                            for (int y = 0; y < rd * cd; y++) {
                                if (array2[y] >= 0) {
                                    char intTostr[35];
                                    sprintf(intTostr, " %d ", array2[y]);
                                    NND++;
                                    strcat(st2, intTostr);
                                }
                            }
                            printf("Number of Nearby Availbility Nodes :   %d\n", NND);
                            printf("%s  \n", st2);
                        }
                        }
                        printf("------------------------------------------------------------------------------------------\n");
                        fflush(stdout);
                    }

                }
                MPI_Barrier(world_comm);//barrier
                i++;
                TimeIncrement(&stime);//to increment the time
                sleep(0.1);

            }
}

/*
Communicatior of the base staiton
@param world_comm
@param poitner ot the base staiton
@param current to store the current to store did it receive a message
@param to set that it is finish receicing
@param poitner to the arraay of Neighbor hood info for sending purpose 
@param termination not used
@param time  the time comunication time from each node
@param tmessage it received from the iteraiton
@param row dimension of the gird
@param col dimension of the grid
*/


void  Communicator(MPI_Comm world_comm,struct BaseStation* baseStation,int * current,int * receiving,  int (*NNInfo)[12],int *termination,double *time, int * tmessage,int rd,int cd ) {
	  int count=0;
      MPI_Status status,p_status;
      int global=rd* cd;
      
      double start=MPI_Wtime();
      #pragma omp critical
      {
       while(count<rd* cd*10000 && global>0){
         int flag;
         
         MPI_Iprobe(MPI_ANY_SOURCE, 0, world_comm, &flag, &p_status);// to listen to the message from the EVS node through world comm
         if(flag){
              flag=0;
              MPI_Recv(&baseStation->AlertLogs[p_status.MPI_SOURCE] , 1 , MPI_AlertLog, p_status.MPI_SOURCE , 0 , world_comm, MPI_STATUS_IGNORE);// store the alert node at the base station
              current[p_status.MPI_SOURCE]=1;//store whether it got receivinb message or ngto
              tmessage[p_status.MPI_SOURCE]+=1;// to store the total message
              double end=MPI_Wtime();
              time[p_status.MPI_SOURCE]=end-start;//time difference of node and base communicaition 
              global--;
              
         }

         count++;
       }}

    #pragma omp critical
    {
    *receiving=0;//if finish receiving
    }
    #pragma omp barrier
    
    #pragma omp barrier
    for (int o = 0; o < rd* cd; o++){
        
           MPI_Send(NNInfo[o],12, MPI_INT, o , 0 , world_comm); //send the information back to the respective ndoe
        
    }
    #pragma omp barrier
    }
  
/*

simulate the baseStation
@param world comm
@param pointer to the baseStation
@param row dims of grid
@param col dimension of col




*/
void baseStation(MPI_Comm world_comm,struct BaseStation* baseStation,int rd,int cd){
    int termination=0;
    int iteration=(23-6)*4;//exact counter ot 11:00pm
    int recv_period[rd* cd]; //the last period it receive for each no
    int temp=iteration;
    int recv_current[rd* cd];//did it receibe ti during the first message
    int is_receiving=1;
    double times[rd* cd];//tjhe time difference between the time
    int totalmessage[rd* cd];//total message from each respective noe
    struct SystemTime stime;//stime
    time_initialization(&stime);//start the time
    memset(totalmessage,0,sizeof(totalmessage));//set all the rpestpective to 0
    int nearest_node[rd* cd][12];//the maximum neartest node for  a node is 12
    double start_sim=MPI_Wtime();
     for (int i = 0; i < rd* cd; i++) {
                       recv_period[i]=2;// to know did it receive during this peirod, cannto mroe than 2 and less than 0
    }
    baseStation->comm=world_comm;

    #pragma omp parallel num_threads(2) shared(termination,iteration,baseStation,recv_period,recv_current,is_receiving,nearest_node)//omp to seperate the tie
    {
       int threadID = omp_get_thread_num();
       if(threadID==0)
        {   
            int threadID = omp_get_thread_num();
            while (iteration > 0) {

                
               
               
                
                MPI_Barrier(world_comm);
                
                while(is_receiving){ //if finish receiving
             
                }
                  for (int i = 0; i < rd* cd; i++) {
                           for (int j = 0; j < 12; j++) {
                                    nearest_node[i][j] = -1; //set all the address to -1 to restart the coubnter
                  }
                  } 
               
               
                #pragma omp barrier
                if(!is_receiving){//start analaysis which nearest node is avaialble
                #pragma omp critical//to be safe that communicator thrread cannot overwrite anything although it has been synchornise
                {
                   is_receiving =1;
                   for (int i = 0; i < rd* cd; i++) {//check whether whcih message has been received or not during that periof
                     
                       if(recv_current[i]==1 && recv_period[i]>0)//check if it got receive message then if got decrement
                            {recv_period[i]--;}
                       else if (recv_current[i]==0 && recv_period[i]<2)//if not, ++ and if the recv_period is less than 3
                       {recv_period[i]++;}
                       }
                        
                }}
                
                 for (int i = 0; i < rd* cd; i++){// to find the nearet node avialbiltiy
                     int curr=0;
                     int is_nbr[rd* cd];
                     memset(is_nbr,0,sizeof(is_nbr));//set all is_nbr to 0
                     int found_nbr[rd* cd];
                     memset(found_nbr,0,sizeof(is_nbr));//to mark is this neighbor neighbor found
                     for(int r=0;r<nNbr;r++){
                        if(baseStation-> NNodeDB[i].NodeNeighbor[r]>=0){
                                    is_nbr[baseStation-> NNodeDB[i].NodeNeighbor[r]]=1;//if it is a neigbor to node i
                        } 
                     }
                     for (int j = 0; j < nNbr; j++) {// for each neighbor
                       if(baseStation->NNodeDB[i].NodeNeighbor[j]>=0){
                              int neigh=baseStation->NNodeDB[i].NodeNeighbor[j];
                        for(int k =0 ;k<nNbr;k++){//for eahc neighbir neighbor
                           int neighn = baseStation->NNodeDB[neigh].NodeNeighbor[k];
                            if (neighn >= 0 && !is_nbr[neighn] && neighn!=i && !found_nbr[neighn]) {//if neighbor's neighbor is not found
                             found_nbr[neighn]=1;//if found 
                             if( recv_period[neighn]==2 ){//if not receive
                                       nearest_node[i][curr++]=neighn;//store the information there to be send later
                             }
                              
                        }
                        }
                     }}}
                #pragma omp barrier
                for(int i = 0; i < rd*cd; i++){
                if(recv_current[i])
                { 
                WriteLog(baseStation,&baseStation->AlertLogs[i],temp-iteration,i,nearest_node[i],times[i],totalmessage[i],&stime,cd ,rd);}}//write to the output file
                #pragma omp barrier
               
                TimeIncrement(&stime);//time increment
                iteration--;
                #pragma omp critical
                {
                  if((iteration==1)){
                        termination=1;
                }
                }
               MPI_Barrier(world_comm);
               sleep(0.1);
               
            }
             
           
        }
        else if(threadID==1)
        {

            while (iteration>0) {
                memset(recv_current,0,sizeof(recv_current));// to restart current receive
                Communicator(world_comm,baseStation,recv_current,&is_receiving,nearest_node,&termination,times,totalmessage,rd,cd);//communicator for eahc loop after receiving
                sleep(0.1);
            }
            if(termination==1){
                    int dummy=0;
                    for(int i = 0; i < rd*cd; i++){//using tag to specify for terminate it
                           MPI_Send(&i,1,MPI_INT,i,TERMINATION,world_comm);
                           
                           printf("Node Rank %d has been terminated for maintainece \n", i );
                    }
                    printf("All the Stations will be closing\n" );
                    printf("Base Station will be terminated for maintainace\n" );
                    }
            int total=0;
            for(int i=0;i<rd*cd;i++){
                total+=totalmessage[i];
            }
            printf("Number of Processor: %d\n",rd*cd+1);
            printf("grid: (%d,%d)\n",rd,cd);
            printf("total number of iteration:   %d\n" ,temp);
            printf("total number of transacition :   %d\n" ,total);
            double end_sim=MPI_Wtime();
            double diff=end_sim-start_sim;
            printf("Total Time of its excecution   : %.4f\n",diff);
      }}}