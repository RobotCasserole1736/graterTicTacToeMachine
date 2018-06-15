import libjevois as jevois
import cv2
import numpy as np
import math
from statistics import mode

# @videomapping YUYV 352 288 30.0 YUYV 352 288 30.0 JeVois PythonSandbox
class GraterTicTacToe:
    # ###################################################################################################
    ## Constructor
    
    def __init__(self):
        self.timer = jevois.Timer("sandbox", 100, jevois.LOG_INFO)
        self.storedlist= []
        self.GraterActivation=0
        # Instantiate a JeVois Timer to measure our processing framerate:
        
    # ###################################################################################################
    ## Process function with USB output
    def process(self, inframe, outframe):
    
        # Gets the frame from the jevois camera in BGR format
        inimg = inframe.getCvRGB()
        
        # Start measuring image processing time (NOTE: does not account for input conversion time):
        self.timer.start()
        
        # define range of yellow color in LAB
        lower_red = np.array([200,90,90])
        upper_red = np.array([255,200,200])
        
        lower_green= np.array([50,120,95])
        upper_green = np.array([150,255,200])
        
        lower_black = np.array([0,0,0])
        upper_black = np.array([95,95,95])
        
        #creates the mask using the ranges defined in the previous code
        redmask = cv2.inRange(inimg, lower_red, upper_red)
        greenmask = cv2.inRange(inimg, lower_green, upper_green)
        blackmask = cv2.inRange(inimg, lower_black, upper_black)
        
        #makesa greyscale image of the mask and sends it out for viewing purposes
        redmaskoutput = cv2.cvtColor(redmask, cv2.COLOR_GRAY2BGR)
        greenmaskoutput = cv2.cvtColor(greenmask, cv2.COLOR_GRAY2BGR)
        blackmaskoutput = cv2.cvtColor(blackmask, cv2.COLOR_GRAY2BGR)
        
        
        #sets kernel
        kernel = np.ones((5,5), np.uint8)
        
        #Red
        redmaskblurred = cv2.medianBlur(redmask,3)
        redmaskdilated = cv2.dilate(redmaskblurred, kernel, iterations =1)
        redmaskeroded = cv2.erode(redmaskdilated, kernel, iterations =1)
        redcontours = self.findcontours(redmaskeroded)

        #Green
        greenmaskblurred = cv2.medianBlur(greenmask,3)
        greenmaskeroded = cv2.erode(greenmaskblurred, kernel, iterations =1)
        greenmaskdilated = cv2.dilate(greenmaskeroded, kernel, iterations =1)
        greencontours = self.findcontours(greenmaskdilated)
        
        #Black
        blackmaskblurred = cv2.medianBlur(blackmask,3)
        blackmaskdilated = cv2.dilate(blackmaskblurred, kernel, iterations =10)
        blackmaskeroded = cv2.erode(blackmaskdilated, kernel, iterations =8)
        blackcontours = self.findcontours(blackmaskeroded)
        

        #Uses contours to find position on board
        if blackcontours != []:
            gridcoords = (self.ContourBlackWidth(blackcontours))
        
            xlist,dottedinimg=(self.ContourXcheck(redcontours,gridcoords,inimg))
            oandxlist,multidottedinimg=(self.ContourOcheck(greencontours,gridcoords,xlist,dottedinimg))
            
            #sends output over serial
            completedoutput = cv2.rectangle(multidottedinimg, (int(gridcoords[0]),int(gridcoords[2])), (int(gridcoords[1]),int(gridcoords[3])), (255,255,255), thickness=3)
            
            topscreen= np.concatenate((completedoutput,blackmaskoutput),axis=1)
            #NOTE I CHANGED THIS TO COMPLETED OUTPUT
            bottomscreen= np.concatenate((redmaskoutput,greenmaskoutput),axis=1)
            output=np.concatenate((topscreen,bottomscreen),axis=0)
            
            outframe.sendCvRGB(output)
            if self.GraterActivation==1:
                self.storedlist=self.storedlist+oandxlist
                if len(self.storedlist)>=180:
                    z=0
                    pos1=[]
                    pos2=[]
                    pos3=[]
                    pos4=[]
                    pos5=[]
                    pos6=[]
                    pos7=[]
                    pos8=[]
                    pos9=[]
                    for boardvalues in self.storedlist:
                        if z==0:
                            pos1=pos1+[boardvalues]
                        if z==1:
                            pos2=pos2+[boardvalues]
                        if z==2:
                            pos3=pos3+[boardvalues]
                        if z==3:
                            pos4=pos4+[boardvalues]
                        if z==4:
                            pos5=pos5+[boardvalues]
                        if z==5:
                            pos6=pos6+[boardvalues]
                        if z==6:
                            pos7=pos7+[boardvalues]
                        if z==7:
                            pos8=pos8+[boardvalues]
                        if z==8:
                            pos9=pos9+[boardvalues]
                        if z<8:
                            z=z+1
                        else:
                            z=0
                    mode1=mode(pos1)
                    mode2=mode(pos2)
                    mode3=mode(pos3)
                    mode4=mode(pos4)
                    mode5=mode(pos5)
                    mode6=mode(pos6)
                    mode7=mode(pos7)
                    mode8=mode(pos8)
                    mode9=mode(pos9)
                    finaloutput='^'+mode1+mode2+mode3+mode4+mode5+mode6+mode7+mode8+mode9+'\n'
                    jevois.sendSerial(str(finaloutput))
                    self.storedlist=[]
                    self.GraterActivation=0
        else:
            topscreen= np.concatenate((inimg,blackmaskoutput),axis=1)
            bottomscreen= np.concatenate((redmaskoutput,greenmaskoutput),axis=1)
            output=np.concatenate((topscreen,bottomscreen),axis=0)
            outframe.sendCvRGB(output)
        
    def parseSerial(self, str):
        jevois.LINFO("parseserial received command [{}]".format(str))
        if str =="GRATERTTT":
            self.GraterActivation=1
        else:
            return "ERR: Unsupported command"
        
    @staticmethod
    def findcontours(input):
        """Sets the values of pixels in a binary image to their distance to the nearest black pixel.
        Args:
            input: A numpy.ndarray.
            external_only: A boolean. If true only external contours are found.
        Return:
            A list of numpy.ndarray where each one represents a contour.
        """
        im2, contours, hierarchy =cv2.findContours(input, mode=cv2.RETR_LIST, method=cv2.CHAIN_APPROX_SIMPLE)
        return contours
        
        
    def ContourBlackWidth(input, secondinput):
        maxwidth = 0
        for contour in secondinput:
            x,y,w,h = cv2.boundingRect(contour)
            if (w >= maxwidth):
                maxwidth = w
                xcoord = x
                height = h
                ycoord = y
        if (maxwidth == 0):
            return [error]
        else:
            wid1=xcoord+(maxwidth/3)
            wid2=xcoord+((2*maxwidth)/3)
            hei1=ycoord+(height/3)
            hei2=ycoord+((2*height)/3)
            return [wid1,wid2,hei1,hei2]
            
    def ContourXcheck(input,secondinput,gridcoords,image):
        xlist= [' ',' ',' ',' ',' ',' ',' ',' ',' ']
        for contour in secondinput:
            x,y,w,h = cv2.boundingRect(contour)
            centx= x+(w/2)
            centy= y+(h/2)
            image=cv2.circle(image,(int(centx),int(centy)),2,(255,0,0),thickness=2)
            if centx<= gridcoords[0]:
                if centy<=gridcoords[2]:
                    xlist[0]='X'
                elif centy>=gridcoords[3]:
                    xlist[6]='X'
                else:
                    xlist[3]='X'
            elif centx>= gridcoords[1]:
                if centy<=gridcoords[2]:
                    xlist[2]='X'
                elif centy>=gridcoords[3]:
                    xlist[8]='X'
                else:
                    xlist[5]='X'
            else:
                if centy<=gridcoords[2]:
                    xlist[1]='X'
                elif centy>=gridcoords[3]:
                    xlist[7]='X'
                else:
                    xlist[4]='X'
        return xlist,image
    
    def ContourOcheck(input,secondinput,gridcoords,xlist,image):
        for contour in secondinput:
            x,y,w,h = cv2.boundingRect(contour)
            centx= x+(w/2)
            centy= y+(h/2)
            image=cv2.circle(image,(int(centx),int(centy)),2,(0,255,0),thickness=2)
            if centx <= gridcoords[0]:
                if centy <= gridcoords[2]:
                    xlist[0]='O'
                elif centy>=gridcoords[3]:
                    xlist[6]='O'
                else:
                    xlist[3]='O'
            elif centx>= gridcoords[1]:
                if centy<=gridcoords[2]:
                    xlist[2]='O'
                elif centy>=gridcoords[3]:
                    xlist[8]='O'
                else:
                    xlist[5]='O'
            else:
                if centy<=gridcoords[2]:
                    xlist[1]='O'
                elif centy>=gridcoords[3]:
                    xlist[7]='O'
                else:
                    xlist[4]='O'
        return xlist,image