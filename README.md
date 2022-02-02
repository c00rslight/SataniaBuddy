# sneed

dependencies: X11, cairo (plus development libraries)  
compile: make  
run: make run  

Config:  
currently config file location is ./satania.cfg  
Move=BOOL  
MoveIntervalStart=INT  // does not apply when Move set to false, in seconds  
MoveIntervalStop=INT   // does not apply when Move set to false, in seconds  
SitY=INT               // y pixel of the ass of satania, only useful if you're changing the assets  
MoveRandom=BOOL        // toggles the random move  
MoveWindow=BOOL        // toggles the window move
