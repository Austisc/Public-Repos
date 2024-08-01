/**
 * Inside this file you will use the classes and functions from rx.js
 * to add visuals to the svg element in index.html, animate them, and make them interactive.
 *
 * Study and complete the tasks in observable exercises first to get ideas.
 *
 * Course Notes showing Asteroids in FRP: https://tgdwyer.github.io/asteroids/
 *
 * You will be marked on your functional programming style
 * as well as the functionality that you implement.
 *
 * Document your code!
 */



import "./style.css";

import { fromEvent, interval, merge ,Observable} from "rxjs";
import { map, filter, scan ,takeUntil, debounceTime} from "rxjs/operators";

/** Constants */

const Viewport = {
  CANVAS_WIDTH: 200,
  CANVAS_HEIGHT: 400,
  PREVIEW_WIDTH: 160,
  PREVIEW_HEIGHT: 80,
} as const;

const Constants = {
  TICK_RATE_MS: 100,
  GRID_WIDTH: 10,
  GRID_HEIGHT: 20,
} as const;

const BlockInfo = {
  WIDTH: Viewport.CANVAS_WIDTH/Constants.GRID_WIDTH,
  HEIGHT: Viewport.CANVAS_HEIGHT/Constants.GRID_HEIGHT,
};

/** User input */
/**used for key identifier
 * S: Move userblock towards the ground respect to 1
 * A: Move userblock towards the left respect to 1
 * D: Move userblock towards the right respect to 1
 * 
 * L:AntiClockwise Rotation
 * K:Clockwise Rotation
 * U:Restart game to the initial State
 * 
 * 
*/
type Key = "KeyS" | "KeyA" | "KeyD" | "KeyU"|"KeyL"|"KeyK"  ;
/**
 * Object in tetris
 * type of Block
 * position: position of the block 
 * sideLgth:klength of the block
 * respVector:the respective vector to the "centre" of its geometric shape
 * colour: color of the block
 */

type Block =Readonly<{pos:Vector;
  id:number;
  sideLgth:number;
  respVector:Vector;
  respective:number;
  colour:string;
  }>;

type Blocks =Readonly<{
    concatBlock:ReadonlyArray<Block>;
}>

/**
 * 
 * Shape that made out from a number of blocks and content the respective geometric of each Blocks
 * which for my game is cube,I,T,L,Z
 */
type Shape =Readonly<{
  Geometric:number[][];
  colour:string;
}>
const IShape:Shape ={
  Geometric:[[0,0],[1,0],[2,0],[3,0]],
  colour:"red"
} 
const  CubeShape:Shape= {
  Geometric:[[0,0],[0,1],[1,0],[1,1]],
  colour:"green"
}

const TShape:Shape ={
  Geometric:[[0,0],[1,0],[2,0],[1,1]],
  colour:"orange"
}
const LShape:Shape ={
  Geometric:[[0,0],[1,0],[2,0],[2,1]],
  colour:"purple"
}

const ZShape:Shape ={
  Geometric:[[0,0],[1,0],[1,1],[2,1]],
  colour:"pink"
}



/**
 * 
 * interface for the action
 */
interface Action {
  apply(s: State):State;
}

/** Utility functions */
/**simplfied immutable vector class that for the asteroid game which 
 * only add and rotate is added from https://tgdwyer.github.io/asteroids/
 * 
 * 
 */
class Vector{
     constructor(public readonly x: number = 0, public readonly y: number = 0){this.x=x,this.y=y}
     addV=(b:Vector)=> new Vector(this.x+b.x,this.y+b.y);
     rotate = (deg:number) => 
        (rad => (
            (cos, sin, { x, y }) => new Vector(Math.round(x * cos  - y * sin),Math.round( x * sin + y * cos))
        )(Math.cos(rad), Math.sin(rad), this)
        )((Math.PI * deg)/ 180)

     }
/**
 * 
 * brute force to for the super rotation
 */
const BruteF:number[]=[0,1,-1,2,-2,3,-3]
const BruteForceReAdjust:Vector[]= BruteF.reduce((acc,x1)=>acc.concat(BruteF.map(x2=>new Vector(BlockInfo.HEIGHT*x1,BlockInfo.WIDTH*x2))),[]);



const IDStart=4;

/** State processing
 * 
 * userBlock:Blocks : the 4 blocks that can be move around by user
 * BlocksInside:Blocks []: array of blocks that store the information of the 
 * preview:Blocks :next blocks for the user
 * occupied: a Matrix map respective to the view to deal with collison and adjusting of the userBlock
 * deleteid: after a row is full, it will store the deleted one, which it is mainly for the view of the deleted block
 * restartdeleteid: after the game is restart, it will store the deleted one, which it is mainly for the view of the deleted block
 * score: the current score 
 * high score: the maximum score that accumulate during the program runs
 * totalBlock: to compute the id of  lastest block that created
 * restart: to know whether the game is restarted 
 * gameEnd: is it game over?
 */

type State = Readonly<{
  userBlock:Blocks;
  BlocksInside:ReadonlyArray<Block>;
  DeletedBlocks:ReadonlyArray<Block>;
  userStatic:Boolean;
  preview:Blocks;
  occupied:ReadonlyArray<ReadonlyArray<number>>;
  deleteid:ReadonlyArray<number>
  restartdeleteid:ReadonlyArray<number>
  score:number;
  highscore:number;
  totalBlock:number;
  restart:boolean;
  gameEnd: boolean;
}>;
/**
 * 
 * constant state of the block
 * default setting of the B
 * 
 * 
 */
const B=<Block>{id:0,pos:new Vector(0,0),sideLgth:BlockInfo.HEIGHT,colour:"",respective:0
,respVector:new Vector(0,0)};

/**
 * so this function basically create a block with its own attributes, which to differentiate in the view
 * 
 * @param x: x-th position
 * @param y: y-th position
 * @param color: color of the block
 * @ud    id: id of the block
 */
  
function CreateBlocks(x:number,y:number,color:String ,id:number):Block{
    return <Block> {
        ...B,pos: new Vector(BlockInfo.WIDTH*3+x*BlockInfo.WIDTH,(1+y)*BlockInfo.HEIGHT),colour:color,id:id,respVector:new Vector((x)*BlockInfo.WIDTH,(y)*BlockInfo.HEIGHT)
    }
    }

/**
 * so this function basically create blocks which is using a previous function,which since we got the shape information we could use
 * its respective geometric value to determine the respective displacement of the block when moving, and also using the advantage of the 
 * the number of blocks inside the object to compute its id with the total blocks that created in the state
 * 
 * @param x: x-th position
 * @param y: y-th position
 * @param color: color of the block
 * @param id: id of the block
 * @returns Blocks
 */
const CreateShape=(shapeInfo:Shape,num:number):Blocks=>{
  return <Blocks> {
       concatBlock:shapeInfo.Geometric.map((coord,ind)=>CreateBlocks(coord[0],coord[1],shapeInfo.colour,num+ind%4+8))
  }
}
const shapes:Shape[]=[TShape,LShape,IShape,ZShape,CubeShape];

const generateShape=(num:number,seed:number=100):Blocks=>{
      return CreateShape(shapes[(num+seed*2**3 )%shapes.length],num);
}


/**
 * 
 * initial state of the game 
 */

const initialState: State = {
  userBlock:generateShape(0,31),BlocksInside:[], DeletedBlocks:[],occupied:Array(Constants.GRID_HEIGHT).fill(Array(Constants.GRID_WIDTH).fill(0)),
  score:0,totalBlock:IDStart,highscore:0,restart:false,
  gameEnd: false,userStatic:false,preview:generateShape(4,31),deleteid:[],restartdeleteid:[]
} as const;

/**
 * 
 * to move a block
 * @param Block to change its pos
 * @return Block that has been moved by the vector
 * 
 */

  const moveBlock=(block:Block)=> (vec:Vector):Block=> (<Block>{
   ...block,pos: block.pos.addV(vec)
  })
/**
 * 
 * to move a group of block accoriding to the vector
 * @param Blocks to change its pos
 * @return Blocks that have been moved by the vector
 * 
 */
  const moveBlocks=(blocks:Blocks)=>(vec:Vector):Blocks=>(<Blocks>{
   ...blocks,concatBlock:blocks.concatBlock.map(x=>moveBlock(x)(vec))
  })
  
/**
 * 
 * 3 function belwo is to check the position of the x to let them limit the movement which
 * only negative y is excluded since that the block will not go up itself 
 * @param Block to check its pos 
 * @return boolean depends on the position of the block
 * 
 */
  

  const hitBottomEdge =(b:Block):boolean => ((b.pos.y+BlockInfo.HEIGHT<Viewport.CANVAS_HEIGHT));
 
  const hitlHorizontalEdge =(b:Block):boolean =>((b.pos.x >0 )) ; 
  const hitrHorizontalEdge =(b:Block):boolean =>((b.pos.x)<Viewport.CANVAS_WIDTH-BlockInfo.WIDTH)

  /**
 * 
 * 3 function below is to check each block in the obj,Blocks that satisfy the condition, if not, all false
 * @param Blocks to check its pos 
 * @return boolean depends on every position of the blocks
 * 
 */
  //UserBlocks;
  const NhitEdges           =(bs:Blocks):boolean =>   bs.concatBlock.every(hitBottomEdge);
  const hitlHorizontalEdges =(bs:Blocks):boolean =>   bs.concatBlock.every(hitlHorizontalEdge)
  const hitrHorizontalEdges =(bs:Blocks):boolean =>   bs.concatBlock.every(hitrHorizontalEdge)


  /**
 * the function to return a matrix with matrix[x][y]=d
 * @param Matrix, an array of array of ids that to help easy tracking
 * @param x,y,d
 * @return Matrix[x][y] where is d
 * 
 */
  const MatFlipBit=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(x:number,y:number,d:number)=>
     Mat.map((row,rowIndex) =>
  rowIndex === x? row.map((col,colIndex) => (colIndex===y?d:col)): row)

   /**
 * using the reduce function to find the sum of each row and then sum it up and then if it is equals to number of row, concat it to a list
 * @param Matrix, an array of array of ids that to help easy tracking
 * @return array of index number to represent which index is needed to eleiminate
 * 
 */
  const indexElimination=(Mat:ReadonlyArray<ReadonlyArray<number>>):ReadonlyArray<number>=>(
   Mat.map(row=>row.reduce((sum,x)=>x>0?sum+1:sum,0)).reduce((res,val,index)=>val===Constants.GRID_WIDTH?res.concat([index]):res,[]));
    /**
 * using the reduce function to concat list of id to be removed
 * @param array of id to be deleted,Matrix, 
 * @param array of array of ids that to help easy tracking
 * @return return the list of id for the block to be deleted 
 * 
 */
  const deleteElimination=(Lst:ReadonlyArray<number>)=>(Mat:ReadonlyArray<ReadonlyArray<number>>):ReadonlyArray<number>=>
  Lst.reduce((res,val)=>res.concat(Mat[val]),[])
    /**
 * to check the any number is equal to the block of the id
 * @param array of id to be deleted
 * @param block to be deleted or not
 * @return true or false
 * 
 */
  const BlockElimination=(Lst:ReadonlyArray<number>)=>(b:Block):boolean=> !Lst.some(val=>(val===b.pos.y/Constants.GRID_HEIGHT));

/**
 * to filter out the number that are smaller than bl.pos.y/Constants.GRID_HEIGHT, so we can get by how far
 * it needed to move for the game which is the length of the filtered Lst
 * @param array of id to be delete
 * @param static block to be moved
 * @return length of the filtered list
 * 
 */

  const StaticScale=(Lst:ReadonlyArray<number>)=>(bl:Block):number=>Lst.filter(val=>(bl.pos.y/Constants.GRID_HEIGHT<val)).length
  /**
 * to move the block acocrding to the number given by the StaticScale
 * 
 * @param array of id to be delete
 * @param an array of static block to be moved
 * @return block that moved downward according to its relative position
 * 
 */

  //Static
  const RelativeStaticMove= (Lst:ReadonlyArray<number>)=>(bls:ReadonlyArray<Block>):ReadonlyArray<Block>=>{

     return bls.map(bl=>moveBlock(bl)(new Vector(0,StaticScale(Lst)(bl)*BlockInfo.HEIGHT)));

    }
    /**
 * to move the block acocrding to the number given by the StaticScale
 * the whole operation of moving the static blocks which we need to know which block is needed to be eliminated and those have be moved 
 * and then perform the whole list of block to be moved downards
 * @param array of id to be delete
 * @param an array of static block to be moved
 * @return blocks that has moved downwards according to its relative position
 * 
 */

  const moveStaticBlocks=(Lst:ReadonlyArray<number>)=>(B:ReadonlyArray<Block>):ReadonlyArray<Block>=>RelativeStaticMove(Lst)((B.filter(bls=> BlockElimination(Lst)(bls))));
  /**
 * the function to save the blocks that if it is not on the index elimination and at the same time, inclduing the user Block that has become static after moving and 
 * hit the ground by concating the whole list into the saved blocks
 *  it will eliminate the block and then move the block that are nessecary to move according to its position for the game condition inside this function
 * @param static blocks
 * @param user Blocks
 * @param current occupied
 * @return array of blocks that depends on the position of the blocks
 * 
 */
  const saveBlock = (blocks:ReadonlyArray<Block> )=>(userBlock:Blocks )=>(occupied:ReadonlyArray<ReadonlyArray<number>>):ReadonlyArray<Block>=>
   moveStaticBlocks(indexElimination(occupied))(!(verticalCollsion(userBlock)(occupied))?blocks.concat(userBlock.concatBlock):blocks);


 
  /**
 * the function to return a matrix with matrix[x][y]=id
 * @param Matrix, an array of array of ids that to help easy tracking
 * @param block 
 * @return Matrix[x][y] where is id of the block
 * 
 */

  const Occupied=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Block)=>MatFlipBit(Mat)(b.pos.y/BlockInfo.WIDTH,b.pos.x/BlockInfo.HEIGHT,b.id);


 /**
 * the function to return a matrix with matrix[x][y]=b.id
 * @param Matrix, an array of array of ids that to help easy tracking
 * @param block 
 * @return Matrix[x][y] where is id of the block
 * 
 */

  
  const setOccupied=(b:ReadonlyArray<Block>)=>b.reduce((acc,b)=>Occupied(acc)(b), Array(Constants.GRID_HEIGHT).fill(Array(Constants.GRID_WIDTH).fill(0)));
  
/**
 * the higher order fuciton for comparing according to every block to the Mat 
 * @param f, function that take in Matrix and block
 * @param Matrix
 * @param block 
 * @return boolean according to the f for each list
 * 
 */
  const BlocksCollision= (f:(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Block)=>boolean)=>(Mat:ReadonlyArray<ReadonlyArray<number>>)=>
  (b:Blocks):boolean=> b.concatBlock.every(d=>f(Mat)(d));


 /**
 * several function that determine can the block be manipulated according ot its position
 *
 * @param Matrix , matrix to help for the collision of two  block
 * @param block 
 * @return boolean according to the position of the block 
 * 
 * 1. it hit a block for the auto down movement
 * 2. the collision to maintaint the down Action  for a block
 * 3. the collision to handle the movement to the left for a block
 * 4. the collision to handle the movement to the right for a block
 */

  const VBCollision=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Block):boolean=>(Mat[b.pos.y/BlockInfo.HEIGHT+1][b.pos.x/BlockInfo.WIDTH]===0);
  const VBDCollision=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Block):boolean=> (b.pos.y+BlockInfo.HEIGHT<Viewport.CANVAS_HEIGHT) &&(Mat[b.pos.y/BlockInfo.HEIGHT+1][b.pos.x/BlockInfo.WIDTH]===0);
  
  const HBlCollision=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Block):boolean=>(b.pos.x>= 0) &&
  (Mat[b.pos.y/BlockInfo.HEIGHT][b.pos.x/BlockInfo.WIDTH-1]===0);
  const HBrCollision=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Block):boolean=>((b.pos.x)<Viewport.CANVAS_WIDTH-BlockInfo.WIDTH) &&
  (Mat[b.pos.y/BlockInfo.HEIGHT][b.pos.x/BlockInfo.WIDTH+1]===0);


   /**
 * the combination of clause which according to the function from above
 *
 * @param Matrix , matrix to help for the collision of two  block
 * @param block 
 * @return boolean according to the position of the block 
 * 
 * 1. it hit a block for the auto down movement
 * 2. the collision to maintaint the down Action  for a block
 * 3. the collision to handle the movement to the left for a block
 * 4. the collision to handle the movement to the right for a block
 * 5. combination of 1. and hit the bottom of edges
 */
   
   //vertical collision for the blocks between blocks
   const verticalBlocksCollision=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Blocks):boolean  =>
   BlocksCollision(VBCollision)(Mat)(b);
   //vertical collision for the down action
   const vdBlocksCollision=(Mat:ReadonlyArray<ReadonlyArray<number>>)=>(b:Blocks):boolean        =>
  BlocksCollision(VBDCollision)(Mat)(b);
  //horizontal left collision  for the right left collision
  const horizontalLCollsion=(userBlock:Blocks )=>(occupied:ReadonlyArray<ReadonlyArray<number>>)=>
  BlocksCollision(HBlCollision)(occupied)(userBlock) &&hitlHorizontalEdges(userBlock);
  //horizontal right collision for the right move collision
  const horizontalRCollsion=(userBlock:Blocks )=>(occupied:ReadonlyArray<ReadonlyArray<number>>)=>
  BlocksCollision(HBrCollision)(occupied)(userBlock) &&hitrHorizontalEdges(userBlock); 
//vertical collision for the tick action of  blocks between blocks and the bottom edges
  const verticalCollsion=(userBlock:Blocks )=>(occupied:ReadonlyArray<ReadonlyArray<number>>)   =>
  (NhitEdges(userBlock)&&verticalBlocksCollision(occupied)(userBlock));
 
  /**
 * to store the deleted blocks' id for deleting view child
 * @param list of deleted block
 * @return  convert it into id
 * 
 */
  const StoreDeleteid=(blst:ReadonlyArray<Block>)=>blst.map(d=>d.id)


/**
 * to determine the game is game over or not
 * @param Matrix, an array of array of ids that to help easy tracking
 * @return true if one of element in Mat[0] is more than 0 
 * 
 */

  
    const isgameOver=(Mat:ReadonlyArray<ReadonlyArray<number>>)=> Mat[0].reduce((res,val)=> res+val,0)>0 
/**
 * to handle the state of the game that handled by the tick(interval observable)
 * so it basically will handle automatically the game such as saving,eliminating  and move staic rows
 * make the block move according to the game rule
 * make sure the game will still static due to game over until an restart action is called
 * to show the next shape at the preview
 * to show the current score of the current run
 * to chnage the highest score if nesseary
 * 
 * @param current state
 * @return next state
 * 
 */
   
  
  const handleVerticalCollision= (s:State)=>(direction:Vector)=>{
    
    const setMat=setOccupied(s.BlocksInside)
    const isitgameOver=isgameOver(setMat)
    const current_score=100*Math.pow((indexElimination(setMat).length),2)+s.score
    const highscore=s.highscore>s.score?s.highscore:current_score
    return  verticalCollsion(s.userBlock)(setMat)? 
       <State>{...s, userBlock: moveBlocks(s.userBlock)(direction),
      BlocksInside:saveBlock(s.BlocksInside)(s.userBlock)(setMat),
      restartdeleteid:[],
      totalBlock:s.totalBlock ,
      deleteid:deleteElimination(indexElimination(setMat))(setMat),
      occupied:setMat, score:current_score,restart:false,highscore:highscore
  }:!isitgameOver?
   <State>{...s,userBlock:moveBlocks(s.preview)(new Vector(0,-BlockInfo.HEIGHT)),BlocksInside: 
    saveBlock(s.BlocksInside)(s.userBlock)(setMat),totalBlock:s.totalBlock+4,preview:generateShape(s.totalBlock+4,s.totalBlock+31),
    occupied:setMat 
    }:<State> {...s,gameEnd:isitgameOver}} ;


/**
 * next state that when the action left or right or down is called and move according to the direction which determine by the two clause
 * 
 * @param current state
 * @return next state
 * 
 */
   
    
  const handlelHorizontalCollision= (s:State)=>(direction:Vector)=>{
  const setMat=setOccupied(s.BlocksInside)
   return  <State>{...s,userBlock:  horizontalLCollsion(s.userBlock)(setMat)&& verticalCollsion(s.userBlock)(s.occupied)?moveBlocks(s.userBlock)(direction):s.userBlock}
  }

   
 
  const handlerHorizontalCollision= (s:State)=>(direction:Vector)=>{
  const setMat=setOccupied(s.BlocksInside) 
  return  <State>{...s,userBlock:  horizontalRCollsion(s.userBlock)(setMat) && verticalCollsion(s.userBlock)(s.occupied)?moveBlocks(s.userBlock)(direction):s.userBlock
    }}
  
    const handleddownCollision= (s:State)=>(direction:Vector)=>{
    const setMat=setOccupied(s.BlocksInside) 
    
    return  <State>{...s,userBlock: vdBlocksCollision(setMat)(s.userBlock)?moveBlocks(s.userBlock)(direction):s.userBlock
    }}

    
/**
 * it check that whether can it be rotated or not 
 * 
 * @param Matrix 
 * @param block to be rotated
 * @param the first block  which is  at the relative position of (0,0)
 * @param BFVec for super rotation attempt through brute force 
 * @param deg angle to be rotated 
 * @return can it be rotated
 */

    const checkRotateCollision=(occupied:ReadonlyArray<ReadonlyArray<number>>)=>(block:Block)=>(centreP:Vector)=>(BFVec:Vector)=>(deg:number)=>{
      const rotateCoordinate=centreP.addV(block.respVector.rotate(deg)).addV(BFVec)
      
      return rotateCoordinate.x-3*BlockInfo.WIDTH>=0 && rotateCoordinate.x+3*BlockInfo.WIDTH<Viewport.CANVAS_WIDTH && rotateCoordinate.y-3*BlockInfo.HEIGHT >=0 && rotateCoordinate.y +3*BlockInfo.HEIGHT<Viewport.CANVAS_HEIGHT
      && occupied[rotateCoordinate.y/BlockInfo.HEIGHT][rotateCoordinate.x/BlockInfo.WIDTH]===0
    }

    /**
 * it check that can it be attempted to rotate by each user Blocks
 * 
 * @param Matrix 
 * @param blocks to be rotated
 * @param the first block  which is  at the relative position of (0,0)
 * @param BFVec for super rotation attempt through brute force 
 * @param deg angle to be rotated 
 * 
 */


    const checkEveryRRotateCollision=(occupied:ReadonlyArray<ReadonlyArray<number>>)=>(bls:Blocks)=>(BFVec:Vector)=>(deg:number):boolean=>(
            bls.concatBlock.every(b=>checkRotateCollision(occupied)(b)(bls.concatBlock[0].pos)(BFVec)(deg))
    )
  /**
 * to determine which vector that changes the position of the block can allow to be super rotate while not breaking the rule
 * @param a list of Brute Force Vector for super rotation
 * @param Matrix  for that current state
 * @param the user block to be rotated
 * @param a list of brute force vector that allowed for super position
 * 
 */

    const BFRotateCollision=(BFVec:Vector[])=>(occupied:ReadonlyArray<ReadonlyArray<number>>)=>(bls:Blocks)=>(deg:number):Vector[]=>(
         BFVec.filter(vec=>checkEveryRRotateCollision(occupied)(bls)(vec)(deg))
    )

/**
 * using vector rotate and moveBlock to do the rotation of the blocks
 * @param blocks to be rotate
 * @param vector
 * @param deg number
 * @reutrn block taht has been rotate 
 * 
 */
    
    const  rotateBlock=(blocks:Blocks)=>(BF:Vector)=>(deg:number):Blocks=>(
      <Blocks>{
        ...blocks,concatBlock:blocks.concatBlock.map(x=>{
          
          return {...x,respVector: x.respVector.rotate(deg),pos:blocks.concatBlock[0].pos.addV(BF).addV(x.respVector.rotate(deg))}}
        )
        })


 /**
 * rotate all the block which find the vector of the avail bf to 
 * @param blocks to be rotate
 * @param vector
 * @param deg number
 * @reutrn block taht has been rotate 
 * 
 */
   
    const BFFinalRotateCollision=(BFVec:Vector[])=>(occupied:ReadonlyArray<ReadonlyArray<number>>)=>(bls:Blocks)=>(deg:number):Blocks=>{
     const AvailBF=BFRotateCollision(BFVec)(occupied)(bls)(deg);
     return AvailBF.length>0?  rotateBlock(bls)(BFVec[0])(deg) : bls;
    }

 /**
 * to handle rotation collision
 * @param current state
 * @return next state
 * 
 */
    const handleRotationCollision= (s:State)=>(deg:number)=>{
    const setMat= setOccupied(s.BlocksInside)

     return  <State>{...s,userBlock: verticalCollsion(s.userBlock)(setMat)
      ? BFFinalRotateCollision(BruteForceReAdjust)(setMat)(s.userBlock)(deg):s.userBlock,occupied:setMat
      }}
    
/**
 * restart "manager" which by setting up the initial state again with carrying-on data
 * 
 * @param current state
 * @return next state
 * 
 */
  
    const restart = (s:State)=>{
     
     return <State>{...initialState,highscore:s.highscore>s.score?s.highscore:s.score,restart:true,
      restartdeleteid:StoreDeleteid(s.BlocksInside.concat(s.userBlock.concatBlock))};
    } 

       
/**
 * Action class that govern and be allowed by the obserable  of certain event to change the state
 * 
 * @param Tick scale of moving down
 * @return None it is constructor
 * 
 */
   

class Tick implements Action{
  constructor(public readonly scale: number=1) { }
  apply=(s:State):State=>(handleVerticalCollision(s)(new Vector(0,BlockInfo.HEIGHT*this.scale)))
  
}
class LeftMove implements Action{
  constructor(public readonly horDir: number) { }
  apply=(s:State):State=>(handlelHorizontalCollision(s)(new Vector(this.horDir,0)))
}
class RightMove implements Action{
  constructor(public readonly horDir: number) { }
  apply=(s:State):State=>(handlerHorizontalCollision(s)(new Vector(this.horDir,0)))
}
class MoveDown implements Action{
  constructor(public readonly downhill: number) { }
  apply=(s:State):State=>(handleddownCollision(s)(new Vector(0,this.downhill)))
}

class Rotation implements Action{
  constructor(public readonly deg: number) { }
  apply=(s:State):State=>(handleRotationCollision(s)(this.deg));
}
class Restart implements Action{
  constructor() { }
  apply=(s:State):State=>(restart(s))
}
/**
 * Updates the state by proceeding with one time step.s
 *
 * @param s Current state
 * @returns Updated state
 */
const tick = (s: State,action:Action) =>action.apply(s);

/** Rendering (side effects) */

/**
 * Displays a SVG element on the canvas. Brings to foreground.
 * @param elem SVG element to display
 */
const show = (elem: SVGGraphicsElement) => {
  elem.setAttribute("visibility", "visible");
  elem.parentNode!.appendChild(elem);
};

/**
 * Hides a SVG element on the canvas.
 * @param elem SVG element to hide
 */



const hide = (elem: SVGGraphicsElement) =>
  elem.setAttribute("visibility", "hidden");

/**
 * Creates an SVG element with the given properties.
 *
 * See https://developer.mozilla.org/en-US/docs/Web/SVG/Element for valid
 * element names and properties.
 *
 * @param namespace Namespace of the SVG element
 * @param name SVGElement name
 * @param props Properties to set on the SVG element
 * @returns SVG element
 */
const createSvgElement = (
  namespace: string | null,
  name: string,
  props: Record<string, string> = {}
) => {
  const elem = document.createElementNS(namespace, name) as SVGElement;
  Object.entries(props).forEach(([k, v]) => elem.setAttribute(k, v));
  return elem;
};








/**
 * This is the function called on page load. Your main game loop
 * should be called here.
 */

export function main() {
  // Canvas elements
  
  const svg = document.querySelector("#svgCanvas") as SVGGraphicsElement      & HTMLElement;
  const preview = document.querySelector("#svgPreview") as SVGGraphicsElement & HTMLElement;
  const gameover = document.querySelector("#gameOver") as SVGGraphicsElement  & HTMLElement;
  const container = document.querySelector("#main") as HTMLElement;

  svg.setAttribute("height", `${Viewport.CANVAS_HEIGHT}`);
  svg.setAttribute("width", `${Viewport.CANVAS_WIDTH}`);
  preview.setAttribute("height", `${Viewport.PREVIEW_HEIGHT}`);
  preview.setAttribute("width", `${Viewport.PREVIEW_WIDTH}`);

  // Text fields
  //const levelText = document.querySelector("#levelText") as HTMLElement;
  const scoreText = document.querySelector("#scoreText") as HTMLElement;
  const highScoreText = document.querySelector("#highScoreText") as HTMLElement;

  /** User input */

  const key$ = fromEvent<KeyboardEvent>(document, "keypress");

  const fromKey = (keyCode: Key) =>
    key$.pipe(filter(({ code }) => code === keyCode));
 /** User input 
  * 
  * 
  * keybroad event that A,D,S,K,L,U which will change the emitted state according to its logic
  * 
  */
  const left$ = fromKey("KeyA").pipe(map(_=>new LeftMove(-1*BlockInfo.WIDTH)));
  const right$ = fromKey("KeyD").pipe(map(_=>new RightMove(BlockInfo.WIDTH)));
  const down$ = fromKey("KeyS").pipe(map(_=>new MoveDown(BlockInfo.HEIGHT)));
  const clockwiseRotate$ = fromKey("KeyK").pipe(map(_=>new Rotation(90)));
  const anticlockwiseRotate$ = fromKey("KeyL").pipe(map(_=>new Rotation(-90)));
  const restart$ = fromKey("KeyU").pipe(map(_=>new Restart()))

  /** Observables */
  /** Determines the rate of time steps, it will change the game state as time going*/
  const tick$ = interval(Constants.TICK_RATE_MS).pipe(map(_=> new Tick()));


  
/**
 * Creates each SVG element of each elemets from array of block with the given properties
 *
 *
 * @param namespace Namespace of the SVG element
 * @param name SVGElement name
 * @param array of block 
 * @param an extra indicate for the string id of svg element
 */
  const createSVGBlocks= (svg:SVGGraphicsElement,namespace:string|null,blocks:ReadonlyArray<Block>,name:string)=> blocks.forEach((b,ind)=>{
    const svgBlock=createSvgElement(namespace,`rect`,
  { 
    id:`${name}${ind}` ,
    height: `${BlockInfo.HEIGHT}`,
    width: `${BlockInfo.WIDTH}`,
    x: `${b.pos.x}`,
    y: `${b.pos.y}`,
    style: `fill: ${b.colour} `,
  })
  svg.append(svgBlock);
  
 
  })

  /**
 * edit userBlock as it keep moving and find it through its id and which we knows its id since 
 * we keep using the same id for block user
 * 
 *
 *
 * @param array of block 
 * 
 */

  const editUserSVGElement= (blocks:ReadonlyArray<Block>)=> blocks.forEach((b,ind)=>{
    const svgBlock=document.getElementById(`BlockUser${ind}`);
   
    if(svgBlock){
    
    svgBlock.setAttribute('x',`${b.pos.x}` );
    svgBlock.setAttribute('y',`${b.pos.y}`);
    svgBlock.setAttribute('style',`fill: ${b.colour} `);
    }
  })
    /**
 * edit preview for the preview as the shape keep changing in static view
 *
 *
 * @param array of block 
 * 
 */
  const editPreviewSVGElement= (blocks:ReadonlyArray<Block>)=> blocks.forEach((b,ind)=>{
    const svgBlock=document.getElementById(`Preview${ind}`);
   
    if(svgBlock){
    
    svgBlock.setAttribute('x',`${b.pos.x}` );
    svgBlock.setAttribute('y',`${b.pos.y}`);
    svgBlock.setAttribute('style',`fill: ${b.colour} `);
    }
  })
    /**
 * delete each svg number according to the id inside Lst
 *
 *
 * @param List of number
 * 
 */

  const deleteSVGElement=(Lst:ReadonlyArray<number>)=>Lst.forEach(id=>{
    const svgBlock=document.getElementById(`${id}`);
    if(svgBlock){
        svg.removeChild(svgBlock);}
  })
  /**
 * delete each userBlock to prevent sideeffect
 *
 *
 * @param 
 * 
 */
  const deleteUserSVGElement=()=>Array.from([0,1,2,3]).forEach(id=>{
    const svgBlock=document.getElementById(`BlockUser${id}`);
    if(svgBlock){
        svg.removeChild(svgBlock);}
  })

  /** 
 * edit the block that we want due to row elimination
 *
 *
 * @param  List of block to be delete
 * 
 */
 
  const editStaticSVGElement=(bls:ReadonlyArray<Block>)=>
  bls.forEach(b=>{
    const svgBlock=document.getElementById(`${b.id}`);
    if(svgBlock){
      svgBlock.setAttribute('x',`${b.pos.x}` );
      svgBlock.setAttribute('y',`${ b.pos.y}`);
      }
    })

     /** 
 * delete the block that we deleted due to row elimination
 *
 *
 * @param  List of block to be delete
 * 
 */
    const deleteStaticSVGElement=(ids:ReadonlyArray<number>)=>
    ids.map(id=>{  
      const svgBlock=document.getElementById(`${id}`);
      if(svgBlock){
            svg.removeChild(svgBlock);
        }
      })
  /** 
 * Creates  SVG for the lastest static svg blocks when user blocks unable to move
 *
 *
 * @param namespace Namespace of the SVG element
 * @param name SVGElement name
 * @param array of block 
 * @param an extra indicate for the string id of svg element
 */
 

  
  const createStaticSVGBlocks= (svg:SVGGraphicsElement,namespace:string|null,blocks:ReadonlyArray<Block>)=> blocks.forEach((b)=>{
    const svgCheck=document.getElementById(`${b.id}`);
    if(!svgCheck){
    const svgBlock=createSvgElement(namespace,`rect`,
  {
    id:`${b.id}` ,
    height: `${BlockInfo.HEIGHT}`,
    width: `${BlockInfo.WIDTH}`,
    x: `${b.pos.x}`,
    y: `${b.pos.y}`,
    style: `fill: ${b.colour} `,
  })
 
svg.append(svgBlock)}})
  
  


 
scoreText.textContent=`${0}`
highScoreText.textContent=`${0}`
  createSVGBlocks(svg,svg.namespaceURI,initialState.userBlock.concatBlock,"BlockUser")
  createSVGBlocks(preview,preview.namespaceURI,initialState.userBlock.concatBlock,"Preview")
  const render = (s: State) => {
   
   scoreText.textContent=`${s.score}`
   highScoreText.textContent=`${s.highscore}`

   /**
    * 
    * 
    * 
    * 
    */
   if(!verticalCollsion(s.userBlock)(s.occupied)){
       createStaticSVGBlocks(svg,svg.namespaceURI,s.userBlock.concatBlock); //create the static block that lastest userBlock just droppped
   }
   if(s.deleteid.length>0){
       scoreText.textContent=`${s.score}`
       scoreText.textContent=`${s.highscore}`
      deleteSVGElement(s.deleteid);//delete all element
      editStaticSVGElement(s.BlocksInside);//delete the user for removing side effect
   }
    
    editUserSVGElement(s.userBlock.concatBlock);//edit the user block
    editPreviewSVGElement(s.preview.concatBlock);//edit the static blocks
    if(s.restart){
      deleteStaticSVGElement(s.restartdeleteid); //delete all element
      deleteUserSVGElement(); //delete the user for removing side effect
      createSVGBlocks(svg,svg.namespaceURI,initialState.userBlock.concatBlock,"BlockUser") //create the userblock againn
    }
  };
 

  const action$: Observable<Action> = merge(tick$,left$,right$,down$,restart$,clockwiseRotate$,anticlockwiseRotate$);  

  const source$ = action$
    .pipe(scan(tick, initialState))
    .subscribe((s: State) => {
      render(s);
      if (s.gameEnd) {
        show(gameover);
      } else {
        hide(gameover);
      }
    });
}

// The following simply runs your main function on window load.  Make sure to leave it in place.
if (typeof window !== "undefined") {
  window.onload = () => {
    main();
  };
}


