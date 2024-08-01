


def optimal(graph,start,finish):
    """
    
    Heavily Modified Depth First Search is used since I am assuming there is no cyclic as it is downhill and 
    if there is a cyclic, the competitor can be greedy to take the 'infinite score' cycle, since if there is no cycle, which sketching the route from 6 to 2
    i could visualize a tree which contain duplicate item, which when going to a vertex, if there is no root or if the sub tree is optimally chosen so it would take the local optimal point 
    and local optimal max score, without going through the subtree again if it is locally optimal chosen. 
    
    
    :Input: 
        graph:a list of graph of length of |D| which each element of tuples contain Edge between u(the start point) and v (the finish point)  and in-between score
        start: integer that indicates the source
        finish: integer that indicates its destination

    
    :Output, return or postcondition: list of list which each route, represented by the list is globally maximal and each routes starts with the start point and ended with finish point
    else: None
    Input:[(0, 6, -500), (1, 4, 100), (1, 2, 300),
    (6, 3, -100), (6, 1, 200), (3, 4, 400), (3, 1, 400),
    (5, 6, 700), (5, 1, 1000), (4, 2, 100)]

    Output: [[5, 1, 2]]
    :Time complexity: O(|D|) 
    :Aux space complexity: O(|D|)+O(|P|))
    """

    # assume there is no cycle since if there is a cyclic, the competitor can be greedy to take the 'infinite score' cycle
    u_list=[]
    v_list=[]
    route=0
    # to find the number of vertex,similar to Q1
    for i in range(len(graph)):  #O(|D|)
        u_list.append(graph[i][0])
        v_list.append(graph[i][1])
    downhill_segment=max([max(u_list),max(v_list)])
    Start_END=[None]*(downhill_segment+1)
    END_Start=[None]*(downhill_segment+1)
    vertex_list=[None]*(downhill_segment+1)
    
    # initialise a adjancecy list of vertec which stroing the number of list and the edge.
    for i in range(downhill_segment+1):
        Start_END[i], END_Start[i]=[],[]
    
    for i in range(len(graph)): #O(|D|)
        Start_END[graph[i][0]].append(graph[i][1])
        END_Start[graph[i][0]].append(Edge(graph[i][0],graph[i][1],graph[i][2]))
        # it adding the incoming route for the destination
        if graph[i][1]==finish:
            route+=1
    # each length of list in the adjancecy list represent a route that it would bring its to another possible vertex
    for i in range(len(vertex_list)): #O(|D|)
            e=len(Start_END[i])
            # if there is no cycle, it will reach destination once, so the possible route for destination is 0
            if i==finish:
                e=0
            vertex_list[i]=Vertex2(i,e)
    # the finish route needs to know the incoming route so we know should we choose the finish point again
    vertex_list[finish].incoming_route=route
    discovered=[]
    visit=[]
    # discovered to put start inisde as the source.
    discovered.append(start)
    
    vertex_list[start].current_score=0
    vertex_list[finish].destination=True
    # score to indicate the sum of the route, it will get sequentially
    score=[]
    # score -inf as it is used to compare maximum value
    score_list=[-inf]
    
    d=0
    first_try=True
    
    while len(discovered)>0: #O(|D|+O(|P|)) since it is O(|D|+O(|P|)), since the inner loop only deal with O(|P|) that has no duplicate.    
        # to track fro the sources, it still has how many route
        if len(visit)==1:
            vertex_list[start].route-=1
        u=discovered.pop()
        visit.append(u)  #O(1)
        
        # as score of current score is 0 in first try and easier to mainpulate as current is needed to add
        if(first_try):
                if len(score)>0: 
                    vertex_list[u].current_score+=(vertex_list[u].prev_score+score[-1])
                    score.append(vertex_list[u].prev_score+score[-1])
                    
                   
                else:
                    score.append(0)           
        else:
             # overwrite the current score of vertex  instead of adding into it
             if len(score)>0:        
                vertex_list[u].current_score=(vertex_list[u].prev_score+score[-1])

                
                score.append(vertex_list[u].prev_score+score[-1])
                
                

             else:
                    score.append(0)
        # once it reach destination or reach a locally optimal road and it has destination
        if u==finish or (vertex_list[u].local_optimal==True and vertex_list[u].hasroot==True):
            vertex_list[finish].incoming_route-=1
            # score_list to keep track which returning route is chosen based on the score, which is back_track_score,if same, it will append the same score route else, overwrite it 
            if score_list[0]<vertex_list[u].current_score:
                score_list=[vertex_list[u].current_score]
                return_visit=copy.deepcopy(visit)
                while return_visit[-1]!=finish: #O(|P|)
                       return_visit.append(vertex_list[visit[-1]].optimal_child)
                score_list.append(return_visit)
            elif score_list[0]==vertex_list[u].current_score:
                score_list.append(visit)
            
            back_track_score=score[-1]
            #to change the child of source
            if back_track_score>vertex_list[start].local_optimal_score:   
                vertex_list[start].local_optimal_score=back_track_score
                vertex_list[start].optimal_child=visit[1]
            j=len(visit)-1
            # loop to decrease the possible of number route,if  it is destination, it is untouched.
            # if route==0: the vertex is local_optimal
            while j>0: #O(|P|)
                    if vertex_list[visit[j]].route>0:
                        
                        vertex_list[visit[j]].route-=1
                    if vertex_list[visit[j]].route==0:
                        if vertex_list[visit[j]].destination!=True:
                            vertex_list[visit[j]].local_optimal=True
                    
                    
                    j-=1
            i=len(visit)-1
            p=None
            # if it is locally optimal or it has destination
            while len(visit)>1 and((vertex_list[visit[i]].local_optimal==True)or vertex_list[visit[i]].destination==True): #(O|P|)
                if p==None:
                    p=visit[-1]
                if p==finish and  vertex_list[visit[i]].destination!=True:
                    # to know which road got the destination
                    
                    vertex_list[visit[i]].hasroot=True
               
                
                # operation to change the optimal parent and optimal child to reconstruct later/
                if vertex_list[visit[i]].local_optimal_score<back_track_score:
                            vertex_list[visit[i]].local_optimal_score=back_track_score
                            # it is destination, only the parent need to be change, and the local optimal score need to be changes
                            if vertex_list[visit[i]].destination==True:
                                
                                vertex_list[visit[i]].optimal_parent=visit[i-1]
                                vertex_list[vertex_list[visit[i]].optimal_parent].optimal_child=visit[i]
                                vertex_list[vertex_list[visit[i]].optimal_parent].local_optimal_score=back_track_score
                                vertex_list[visit[i-1]].local_optimal_score=back_track_score
                            else:  
                            #
                               
                                if vertex_list[visit[i-1]].local_optimal_score<vertex_list[visit[i]].local_optimal_score:
                                    vertex_list[visit[i-1]].optimal_child=visit[i]
                                    vertex_list[visit[i]].optimal_parent=visit[i-1]
                                    #print(vertex_list[visit[i]].optimal_parent,vertex_list[visit[i-1]].optimal_child)

                score.pop()
                o=visit.pop()
                first_try=False
                i-=1
                
            # we need to loop to change the previous parent and child of unchanged vertex as previous loop does not 
            # change it.   
            
            if back_track_score>=vertex_list[visit[i]].local_optimal_score:
                    # if length one, the only element is the cource, which just change its child from the previous while its child change its parent to the source
                    if len(visit)==1:
                        vertex_list[visit[i]].optimal_child=o
                        vertex_list[visit[i]].local_optimal_score=back_track_score
                        vertex_list[o].optimal_parent=visit[i]
                        vertex_list[o].local_optimal_score=back_track_score
                    
                    else:
                         # for length >1, append back the o, so the length will be at least 3 and the last one will change both parent and child respectively 
                        visit.append(o)
                        if back_track_score>vertex_list[start].local_optimal_score:
                            vertex_list[start].local_optimal_score=back_track_score
                            vertex_list[start].optimal_child=visit[2]
                            
                        
                            for g in range(1,len(visit)-1): #O(|P|)
                                vertex_list[visit[g-1]].local_optimal_parent=visit[g]
                                vertex_list[visit[g]].local_optimal_score=back_track_score
                                vertex_list[visit[g]].local_optimal_child=visit[g+1]
                                vertex_list[visit[g]].local_optimal_score=visit[g]
                            vertex_list[visit[len(visit)-1]].local_optimal_score=back_track_score
                            
                        visit.pop()
                    
                        
               
                
        # using same concept of depth first search, which edge only goes for one time so the big O is O(|D|) and added into discovered
        

        if (len(Start_END[u]))>0:
            if vertex_list[u].route>0:
                for i in range(len(Start_END[u])):
                    # it choose the the incoming vertex, and put its id number into discovered 



                    if vertex_list[END_Start[u][i].v].destination==True and vertex_list[END_Start[u][i].v].incoming_route>0: #O(|D|)
                        # need to appendd the finish if it is the v(end point) is True and the end point of incoming route >0, so we can know do we need to find the destination again.
                        discovered.append(Start_END[u][i])
                        # just the prev score is the weight of the edge
                        vertex_list[END_Start[u][i].v].prev_score=END_Start[u][i].w
                      
                       
                    elif vertex_list[END_Start[u][i].v].local_optimal==False:
                        # since it is not local optimal, we does not know what we can find so needed to append
                        if vertex_list[END_Start[u][i].v].route>0:
                            discovered.append(Start_END[u][i]) 
                            # just the prev score is the weight of the edge
                            vertex_list[END_Start[u][i].v].prev_score=END_Start[u][i].w
                            

                        
                    
                    elif  vertex_list[END_Start[u][i].v].hasroot==True and not(vertex_list[END_Start[u][i].v].destination==True) and vertex_list[END_Start[u][i].v].local_optimal: 
                                # it is locally optimal and hasroot, so we know it has local maximum  number or not, so it could not be destinatio to avoid appending the finish point again
                            
                                discovered.append(Start_END[u][i])
                                # for getting the score we can get from as it need to know the weight we currently getting and the known route incoming score, whcih it is the original score we have
                                # chosen and the orignal score is used to minus the local optimal score, so we know the score, we can get, without going to the subtree again
                                vertex_list[END_Start[u][i].v].prev_score=(END_Start[u][i].w+vertex_list[END_Start[u][i].v].local_optimal_score-vertex_list[END_Start[u][i].v].current_score)
        
            
            
    return_route=[]
    # to append the score_list
    for i in range(1,len(score_list)):# O(|D|)
        return_route.append(score_list[i])
    
    return return_route if len(return_route)>=1 else None                                                                                                                                                          



class Vertex2: 
    """
    class for Q2, which it is easier to store the info as there are a lot of information is added, a seperate class is needed so it is easier to manipulate Q2
    Time complexity:O(1)
    Space and Aux complexity: O(1)
    """
    def __init__(self,ver,route):
        self.ver_num=ver
        self.current_score=0
        self.optimal_parent=-1
        self.optimal_child=-1
        self.route=route
        self.local_optimal_score=-inf
        self.destination=False
        self.prev_score=0
        self.pred=None 
        self.hasroot=False
        self.local_optimal=False
        self.incoming_route=0
        self.change_prev=True
    def __str__(self):
        return str((self.ver_num,self.route,self.current_score,self.local_optimal_score,self.optimal_parent,self.optimal_child,self.local_optimal,self.destination,self.hasroot,self.incoming_route,self.prev_score))
if __name__ == "__main__": 
    """
    # The roads represented as a list of tuples
    roads = [(0, 1, 4), (1, 2, 2), (2, 3, 3), (3, 4, 1), (1, 5, 2),
    (5, 6, 5), (6, 3, 2), (6, 4, 3), (1, 7, 4), (7, 8, 2),
    (8, 7, 2), (7, 3, 2), (8, 0, 11), (4, 3, 1), (4, 8, 10)]
    downhill=[(0, 6, -500), (1, 4, 100), (1, 2, 300),
    (6, 3, -100), (6, 1, 200), (3, 4, 400), (3, 1, 400),
    (5, 6, 700), (5, 1, 1000), (4, 2, 100)]
    
    cafes = [(5, 10), (6, 1), (7, 5), (0, 3), (8, 4)]
    mygraph = RoadGraph(roads, cafes)
    print(mygraph.routing(1,1))
    
    print(optimal(downhill,6,2))  #output: [[6, 3, 1, 2]]
    print(optimal(downhill,3,2)) #output: [[3, 1, 2]]
    print(optimal(downhill,4,2)) #output: [[4, 2]]
    print(optimal(downhill,5,2))  #output: [[5, 1, 2]]
    """



    