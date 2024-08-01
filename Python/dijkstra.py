"""
FIT 2004 Assignment 2
Python Version 2022.12.1 ,3.10-64 *
Author: Austin Sing Jet Wong
ID: 32933975
"""
import heapq
from math import inf

import copy
"""
Question1
"""
class RoadGraph:

   
    
    def __init__(self, roads, cafes):
        """
        Constructor for a graph which it is using adjancecy list that it check for the maximum number of vertex, as we assuming that all number exist between minimum and maximu
        and store inside adjancecy list so can meet the time and aux space complexity
        roads :a list of graph of length of number of edge which each element of tuple contain Edge between u(the start point) and v (the finish point) and the  in-between positive times
        cafe: a list of vertex which each turples containing the 
        :Output, return or postcondition: None
        :Time complexity:O(V+E) V=Number of Edge, E=len(graph)
        :Aux space complexity: O(|V|+|E|)   V=Number of Edge, E=len(graph)
        """
        # to get the maximum number of vertex
        u_list=[]
        v_list=[] 
        for i in range(len(roads)):  #
            u_list.append(roads[i][0])
            v_list.append(roads[i][1])
        self.Vertices_num=max(max(u_list),max(v_list))
        self.vertices=[None]*(self.Vertices_num+1)
          
        # intilialieses the adjancecy list
        for i in range(self.Vertices_num+1):
            self.vertices[i]=[[]]
        # store the Edge inside self.vertices[u], which depend on the start point, to know which vertex, it is comingfrom.
        for i in range(len(roads)):
            self.vertices[roads[i][0]][0].append((Edge(roads[i][0],roads[i][1],roads[i][2])))
        # store the waitign time in each vertex list  
        for cafe in cafes:
            self.vertices[cafe[0]].append(cafe[1])
        self.cafe_list=[]# storing vertex in each positioon, which vertex id is key.
        self.graph=[]
        # to store the additional info inside adjancecy list to store the waiting time at the end of vertices
        # cafe list store vertex id ascendingly to get the info of vertex easily as we can use a number to find the according vertex
        for i in range(len(self.vertices)): #O(V+E)
            waiting=0
            if len(self.vertices[i])>1:
                waiting=self.vertices[i][1]
            self.cafe_list.append(Vertex(i,waiting))
            for j in range(len(self.vertices[i][0])): #O(E)
                self.graph.append(copy.deepcopy(self.vertices[i][0][j]))#O(1)
        
        
    def routing(self, start, end):
        
        """
        My Dijkstra's Algorithm just able to find the minimum distance to each destination form the source which is the start, which using the empty minimum heap as priority queue
        as heap does not need to be update, but it will accept possible duplicate item, which could be running in O(ElogV^2), which is O(ElogV)
        input:
            start: positive number between 0 and number of vertex
            end:   positive number between 0 and number of vertex
        output:
            list of distance from the start to each vertex. stored respectively
        input: 
            start:1
            end:1
        output: [17, 0, 2, 5, 6, 2, 7, 4, 6]
        :Time complexity:O(ElogV)
        :Aux space complexity: O(V+E)
        References: FIT 2004 Lecture Slide Week 5 Greedy Graph Algorithm,Dijkstra’s Algorithm
        """
        # as finding minimum, use inf to compare is easier as it will be larger.
        # each distance stored inside its respective vertex
        dist=[inf]*(self.Vertices_num+1)
        prioque=[] # empty list for Min heap
        heapq.heappush(prioque,Vertex(start,0))  #stroing the source and 0 inside the heap
        dist[start]=0
        # the source inside dist[start] always will be 0
        while len(prioque)>0: #O(ElogV +V)
            fr=heapq.heappop(prioque).ver_num  #fr is the number that pop from minHeap() 
            self.cafe_list[fr].visited=True # to know which is visit
           
            for i in range(len(self.vertices[fr][0])):   #O(V)
                if len(self.vertices[fr][0])>0:                        
                    
                    if self.cafe_list[self.vertices[fr][0][i].v].visited==False: # if vertices is not visited
                            if self.cafe_list[self.vertices[fr][0][i].v].discovered==False:# if vertices not ye appended into discovered
                                
                                    dist[self.vertices[fr][0][i].v]=(dist[fr]+self.vertices[fr][0][i].w) # the current distance it got + the distance of incoming distance
                                    self.cafe_list[self.vertices[fr][0][i].v].pred=self.vertices[fr][0][i].u # changes its predessor of incoming vertex to the previous vertex
                                    self.cafe_list[self.vertices[fr][0][i].v].discovered=True     # it has been discovered once
                                    heapq.heappush(prioque,Vertex(self.vertices[fr][0][i].v,self.vertices[fr][0][i].w)) # push inside the heap and it will update itself according to the weight, automically
                                    
                            else:# if vertices appended into discovered 
                                    if dist[self.vertices[fr][0][i].v] > dist[fr]+self.vertices[fr][0][i].w: # if distance of incoming distance is larger,which we want in minimum
                                        dist[self.vertices[fr][0][i].v]=(dist[fr]+self.vertices[fr][0][i].w) # the distance will be change as well as its predecessor.
                                        self.cafe_list[self.vertices[fr][0][i].v].pred=self.vertices[fr][0][i].u
    
                                # updating the heap is not required as it is empty heap
            
        
        return dist                             


    def __str__(self):
        """
        to string method for RoadGraph

        Time complexity:O(E)
        Space and Aux complexity: O(E)
        """
        to_strg="["
        for i in range(len(self.graph)-1):
            to_strg+=(str(self.graph[i].__str__())+",")
        to_strg+=(str(self.graph[i+1].__str__()))
        return to_strg+ "]"

class Edge:
    def __init__(self,u,v,w):
        """
        class for Q1 and Q2, which it is easier to store the info
        it represent the edge of grpah

        Time complexity:O(1)
        Space and Aux complexity: O(1)
        """
        self.u=u
        self.v=v
        self.w=w
    def __str__(self):
        """
        to String Method for Edge
        """
        return str((self.u,self.v,self.w))

class Vertex:
    def __init__(self,ver,weight=0,in_heap=False):
        """
        Class for Q1
        it reprensent the class for vertex in q1
        Time complexity:O(1)
        Space and Aux complexity: O(1)
        """
        self.ver_num=ver
        self.weight=weight
        self.discovered=False
        self.pred=None
        self.visited=False
        self.in_heap=in_heap

       
       
    def __str__(self):
        """
       Magic Method of comparing the weight
        so it can compare with others' weight if it is inside a heap, which controlled by boolena
        """
        if self.in_heap==True:
            return self.weight < other.weight


    #\to String Method for Vertex
    def __lt__(self, other):
        """
        the less_than operate Magic Method is overidden, 
        Magic Method of comparing the weight
        so it can compare with others' weight if it is inside a heap, which controlled by boolena
        """
        if self.in_heap==True:
            return self.weight < other.weights
        #the less_than operate Magic Method is overidden, 
     