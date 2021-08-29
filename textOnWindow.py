# -*- coding: utf-8 -*-
"""
Script that helps me figure out the state of variables in a mouse_controlled_program.
Was built for a simple c++ program that writes text to the window.
"""
import random

class tester():
    ###conditional variables
    bDrawing, bWriting, bGaugingSize = 0,0,0
    ptx, pty = 0,0          #point of landing
    x, y = 0, 0              #current position of the mouse
    
    ###actions
    edit = 0
    transfer = 0
    
    def fx_up(self):
        if(self.ptx!=self.x and self.pty!=self.y):
            self.bDrawing = 0
            self.bWriting = 1
            self.bGaugingSize = 0
        else:
            self.bDrawing = 0
            self.bGaugingSize = 0
    
    def fx_down(self):
        if self.bWriting==1:
            self.bWriting = 0
        self.bDrawing = 1
        
    
    def fx_move(self, xx=random.randint(0,100), yy=random.randint(0,100)):
        self.x,self.y = xx, yy
        if self.bDrawing:
            self.bGaugingSize = 1
            
    def virgin_state(self):
        self.bDrawing, self.bWriting, self.bGaugingSize, self.ptx, self.pty, self.x, self.y, self.edit = 0,0,0,0,0,0,0
        
    def down_up(self):
        self.fx_down()
        self.fx_up()
    
    def down_move_up(self, xx=random.randint(0,100), yy=random.randint(0,100)):
        self.fx_down()
        self.fx_move(xx, yy)
        self.fx_up()
        
    def print(self):
        print("bDrawing, bWriting, bGaugingSize: ", self.bDrawing, self.bWriting, self.bGaugingSize)
    
    
def down_up(n):
    p = tester()
    for x in range(n):
        p.down_up()
    print("bDrawing, bWriting, bGaugingSize: ", p.bDrawing, p.bWriting, p.bGaugingSize)
    

"""

Aftermath

I think the circumstances that led me here was a desperate desire to understand how variables were affecting my program. I needed to know the state of conditional variables at any one time, so i asked myself some questions:
    1. what can the program do under the following:
        button down
        button up
        button move
    2. how are variables affected in each of the above states
    3. what are the posible points of confusion.
    4. how can the actions under each situation in 1 be distinguished one from another.

It was nerve wracking trying to come up with a mechanism for this, so i resorted to diagrammatic representation.

I drew out these actions:
    button down:
        starting to draw (either nothing, ie singularity or a non-degenerate rectangle)
    button move:
        doing nothing
        gauging the size of the rectangle
        #these two are distinguishable if there is a variable set by mousedown
    button up:
        finish drawing
        drawing nothing
        #these are distinguishable by the equality of the current point and the initial point of impact
        
for each of these situations, i looked at their conditions and their effect on the variables that were conditions for other actions (or say a case). I wanted to see clearly why i wasn't understanding how things were working. after working out step by step what things were I learnt one very important lesson:
    
    BEFORE GIVING UP ON STH, DETERMINE QUITE INTENSIVELY WHY IT IS POSSIBLE OR IMPOSSIBLE. 
    OR 
    DON'T GIVE UP JUST LIKE THAT

I think i would have very much like a situation where I played around with the functions and kind of automatically determined which were the set of conditions that i would need to accomplish particular states. for example, i might have picked out what some sections (loops or if statements) were doing and what conditions they had and then figured out what states were there. it might also have been clever, if i were to do this, to see that in some siutations an action must always be proceeded or succeeded by another - what goes up must come down, and what is downclicked must be upclicked.
"""