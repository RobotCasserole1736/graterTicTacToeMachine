import libjevois as jevois
import cv2
import random
import numpy as np
import math
from statistics import mode

# @videomapping YUYV 352 288 30.0 YUYV 352 288 30.0 JeVois PythonSandbox
class GraterTicTacToe:
    # ###################################################################################################
    ## Constructor
    
    def __init__(self):
        self.storedlist= []
        self.GraterActivation=False
        self.ImPlaying=False
        self.optxo='X'
        self.HM=True
        self.label='Hello There'
        self.values={0:'Top-Left',1:'Top-Middle',2:'Top-Right',3:'Left-Middle',4:'Center',5:'Right-Middle',6:'Bottom-Left',7:'Bottom-Middle',8:'Bottom Right'}
        # Instantiate a JeVois Timer to measure our processing framerate:
        
    # ###################################################################################################
    ## Process function with USB output
    def process(self, inframe, outframe):
    
        # Gets the frame from the jevois camera in BGR format
        self.inimg = inframe.getCvRGB()
        #self.inimg=cv2.imread("/jevois/data/TTT TEST 1.jpg")
        
        # define range of black color in RGB
        lower_black = np.array([0,0,0])
        upper_black = np.array([125,125,125])
        
        #creates the mask using the ranges defined in the previous code
        mask = cv2.inRange(self.inimg, lower_black, upper_black)
        
        #makes a greyscale image of the mask and sends it out for viewing purposes
        self.maskoutput = cv2.cvtColor(mask, cv2.COLOR_GRAY2RGB)
        contours = self.findcontours(mask)
        

        #Uses contours to find position on board
        if contours != []:
            self.oandxlist=[' ',' ',' ',' ',' ',' ',' ',' ',' ']
            self.ContourBlackWidth(contours)
            self.ContourCheck(contours)
            #Once the command is sent
            if self.GraterActivation:
                
                #record the list from this loop
                self.storedlist.extend(self.oandxlist)
                
                #once we've gone through 20 loops (9*20)
                if len(self.storedlist)>=180:
                    
                    #logs the one that appeared the most at that location
                    finaloutput=[]
                    for i in range(9):
                        templist=[]
                        for j,boardvalues in enumerate(self.storedlist):
                            if j%9==i:
                                templist.append(boardvalues)
                        finaloutput.append(mode(templist))
                    #finaloutput='^'+mode1+mode2+mode3+mode4+mode5+mode6+mode7+mode8+mode9+'\n'
                    if(self.ImPlaying):
                        move=self.Choose(finaloutput,self.optxo)
                        self.ImPlaying=False
                        self.label='I lack arms, so could you place a '+self.optxo+' on the '+self.values[move]+' for me?'
                    jevois.sendSerial(str(finaloutput))
                    self.storedlist=[]
                    self.GraterActivation=False
        output= np.concatenate((self.inimg,self.maskoutput),axis=1)
        msgbox = np.zeros((22, 704, 3), dtype = np.uint8)
        
        cv2.putText(msgbox, self.label, (3, 15),
                   cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1, cv2.LINE_AA)
        output= np.concatenate((output,msgbox),axis=0)
        outframe.sendCvRGB(output)
        
        
    def parseSerial(self, str):
        jevois.LINFO("parseserial received command [{}]".format(str))
        if str =="GRATERTTT":
            self.GraterActivation=True
        if str =="PlayX":
            self.GraterActivation=True
            self.ImPlaying=True
            self.optxo='X'
        if str =="PlayO":
            self.GraterActivation=True
            self.ImPlaying=True
            self.optxo='O'
        if str=='EasyMode':
            self.HM=False
        if str=='HardMode':
            self.HM=True
        else:
            return "ERR: Unsupported command"
        
    @staticmethod
    def findcontours(input):
        contours, __ =cv2.findContours(input, mode=cv2.RETR_LIST, method=cv2.CHAIN_APPROX_SIMPLE)
        return contours
        
    def ContourBlackWidth(self, input):
        maxwidth = 0
        mincnt=[]
        for contour in input:
            x,y,w,h = cv2.boundingRect(contour)
            centX=x+(w/2)
            centY=y+(h/2)
            area = cv2.contourArea(contour)
            hull = cv2.convexHull(contour)
            hull_area = cv2.contourArea(hull)
            solidity = float(area)/(hull_area+0.0000000000000000001)
            extent=area/(w*h)
            if (extent>=0.85 and solidity>=0.9):
                mincnt=contour
            if (w >= maxwidth  and centY<180 and centY>100):
                cnt=contour
                maxwidth = w
                xcoord = x
                height = h
                ycoord = y
        self.maskoutput=cv2.drawContours(self.maskoutput,[cnt],0,(255,200,100),thickness=3)
        if mincnt==[]:
            wid1=xcoord+(maxwidth/3)
            wid2=xcoord+((2*maxwidth)/3)
            hei1=ycoord+(height/3)
            hei2=ycoord+((2*height)/3)
            self.gridcoords=[wid1,wid2,hei1,hei2]
        else:
            x,y,w,h = cv2.boundingRect(mincnt)
            self.gridcoords=[x,x+w,y,y+h]
        self.maskoutput = cv2.rectangle(self.maskoutput, (int(self.gridcoords[0]),int(self.gridcoords[2])), (int(self.gridcoords[1]),int(self.gridcoords[3])), (0,0,200), thickness=3)
        
        
    def Choose(self,input,XorO):
        score=[0,0,0,0,0,0,0,0,0]
        for i, item in enumerate(input):
            if(item==' '):
                score[i]+=1
                if(i!=4):
                    #Horizontal
                    if i%3==0:
                        if input[i+1]==input[i+2] and input[i+1]!=' ':
                            score[i]+=9
                        elif (input[i+1]==XorO and input[i+2]==' ') or (input[i+1]==' ' and input[i+2]==XorO):
                            score[i]+=1
                    elif i%3==1:
                        if input[i-1]==input[i+1] and input[i-1]!=' ':
                            score[i]+=9
                        elif (input[i-1]==XorO and input[i+1]==' ') or (input[i-1]==' ' and input[i+1]==XorO):
                            score[i]+=1
                    else:
                        if input[i-1]==input[i-2] and input[i-1]!=' ':
                            score[i]+=9
                        elif (input[i-1]==XorO and input[i-2]==' ') or (input[i-1]==' ' and input[i-2]==XorO):
                            score[i]+=1
                        
                    #Vertical
                    if(math.floor(i/3)==0):
                        if (input[i+3]==input[i+6] and input[i+3]!=' '):
                            score[i]+=9
                        elif (input[i+3]==XorO and input[i+6]==' ') or (input[i+3]==' ' and input[i+6]==XorO):
                            score[i]+=1
                    elif(math.floor(i/3)==1):
                        if (input[i-3]==input[i+3] and input[i-3]!=' '):
                            score[i]+=9
                        elif (input[i-3]==XorO and input[i+3]==' ') or (input[i-3]==' ' and input[i+3]==XorO):
                            score[i]+=1
                    else:
                        if (input[i-3]==input[i-6] and input[i-3]!=' '):
                            score[i]+=9
                        elif (input[i-3]==XorO and input[i-6]==' ') or (input[i-3]==' ' and input[i-6]==XorO):
                            score[i]+=1
                    #Diagonal
                    if(i%2==0):
                        if(input[4]==input[8-i] and input[4]!=' '):
                            score[i]+=9
                        elif(input[4]==' ' and input[8-i]==XorO)or(input[4]==XorO and input[8-i]==' '):
                            score[i]+=1
                
                    
                else:
                    score[i]+=8
        myPlay=0
        if(self.HM or random.randint(0,1)==1):
            myPlay = score.index(max(score)) 
        else:
            jevois.sendSerial('OOPS')
            score[score.index(max(score))]=1
            myPlay = score.index(max(score))
                
        return myPlay
                    
            
        
            
    def ContourCheck(self,input):
        for contour in input:
            x,y,w,h = cv2.boundingRect(contour)
            
            area = cv2.contourArea(contour)
            hull = cv2.convexHull(contour)
            hull_area = cv2.contourArea(hull)
            solidity = float(area)/(hull_area+0.0000000000000000001)
            extent=area/(w*h)
            centx= x+(w/2)
            centy= y+(h/2)
            color=(0,0,255)
            check=' '
            wcheck=self.gridcoords[1]-self.gridcoords[0]
            if w<wcheck and w>=wcheck/4 and centx>50 and centx<300 and centy>50 and centy<250:
                if extent<=0.9 and solidity<=0.9:
                    color=(255,0,0)
                    check='X'
                    self.maskoutput=cv2.drawContours(self.maskoutput,[contour],0,color,thickness=3)
                if extent<=0.9 and solidity>=0.9:
                    color=(0,255,0)
                    check='O'
                    self.maskoutput=cv2.drawContours(self.maskoutput,[contour],0,color,thickness=3)
                if check!=' ':
                    if centx<= self.gridcoords[0]:
                        if centy<=self.gridcoords[2]:
                            self.oandxlist[0]=check
                        elif centy>=self.gridcoords[3]:
                            self.oandxlist[6]=check
                        else:
                            self.oandxlist[3]=check
                    elif centx>= self.gridcoords[1]:
                        if centy<=self.gridcoords[2]:
                            self.oandxlist[2]=check
                        elif centy>=self.gridcoords[3]:
                            self.oandxlist[8]=check
                        else:
                            self.oandxlist[5]=check
                    else:
                        if centy<=self.gridcoords[2]:
                            self.oandxlist[1]=check
                        elif centy>=self.gridcoords[3]:
                            self.oandxlist[7]=check
                        else:
                            self.oandxlist[4]=check